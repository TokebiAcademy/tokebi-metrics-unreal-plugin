#include "TokebiAnalyticsFunctions.h"
#include "TokebiAnalyticsSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Containers/Ticker.h"

DEFINE_LOG_CATEGORY_STATIC(LogTokebiAnalytics, Log, All);

// Static variables for system state
bool UTokebiAnalyticsFunctions::bSystemInitialized = false;
bool UTokebiAnalyticsFunctions::bGameRegistered = false;
FString UTokebiAnalyticsFunctions::CurrentSessionID = TEXT("");

// Store the real game_id from registration
static FString RegisteredGameId = TEXT("");

// Event queue for batching
static TArray<TSharedPtr<FJsonObject>> EventQueue;
static FCriticalSection EventQueueLock;

// Ticker handle for auto-flush
static FTSTicker::FDelegateHandle FlushTickerHandle;

// Constants
static const float FLUSH_INTERVAL = 30.0f; // Flush every 30 seconds
static const int32 MAX_QUEUE_SIZE = 100;   // Max events before forced flush

void UTokebiAnalyticsFunctions::TokebiRegisterGame()
{
    InitializeTokebiSystem();
    
    if (!bGameRegistered)
    {
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Registering game with Tokebi..."));
        RegisterGameWithTokebi();
    }
    else
    {
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Game already registered with Tokebi"));
    }
}

void UTokebiAnalyticsFunctions::TokebiStartSession()
{
    InitializeTokebiSystem();
    
    CurrentSessionID = GenerateSessionID();
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi session started: %s"), *CurrentSessionID);
    
    TMap<FString, FString> EventData;
    EventData.Add(TEXT("session_id"), CurrentSessionID);
    EventData.Add(TEXT("timestamp"), FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp()));
    
    QueueEvent(TEXT("session_start"), EventData);
}

void UTokebiAnalyticsFunctions::TokebiEndSession()
{
    if (CurrentSessionID.IsEmpty())
    {
        UE_LOG(LogTokebiAnalytics, Warning, TEXT("No active session to end"));
        return;
    }
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi session ended: %s"), *CurrentSessionID);
    
    TMap<FString, FString> EventData;
    EventData.Add(TEXT("session_id"), CurrentSessionID);
    EventData.Add(TEXT("timestamp"), FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp()));
    
    QueueEvent(TEXT("session_end"), EventData);
    
    // Flush immediately for session end
    TokebiFlushEvents();
    
    CurrentSessionID.Empty();
}

void UTokebiAnalyticsFunctions::TokebiTrack(FString EventName, const TMap<FString, FString>& EventData)
{
    InitializeTokebiSystem();
    
    UE_LOG(LogTokebiAnalytics, Verbose, TEXT("Tracking event: %s"), *EventName);
    
    TMap<FString, FString> EnhancedData = EventData;
    EnhancedData.Add(TEXT("timestamp"), FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp()));
    
    if (!CurrentSessionID.IsEmpty())
    {
        EnhancedData.Add(TEXT("session_id"), CurrentSessionID);
    }
    
    QueueEvent(EventName, EnhancedData);
}

void UTokebiAnalyticsFunctions::TokebiTrackLevelStart(FString LevelName)
{
    TMap<FString, FString> EventData;
    EventData.Add(TEXT("level"), LevelName);
    
    TokebiTrack(TEXT("level_start"), EventData);
}

void UTokebiAnalyticsFunctions::TokebiTrackLevelComplete(FString LevelName, float CompletionTime, int32 Score)
{
    TMap<FString, FString> EventData;
    EventData.Add(TEXT("level"), LevelName);
    EventData.Add(TEXT("completion_time"), FString::SanitizeFloat(CompletionTime));
    EventData.Add(TEXT("score"), FString::FromInt(Score));
    
    TokebiTrack(TEXT("level_complete"), EventData);
}

void UTokebiAnalyticsFunctions::TokebiTrackPurchase(FString ItemId, FString Currency, int32 Cost)
{
    TMap<FString, FString> EventData;
    EventData.Add(TEXT("item_id"), ItemId);
    EventData.Add(TEXT("currency"), Currency);
    EventData.Add(TEXT("cost"), FString::FromInt(Cost));
    
    TokebiTrack(TEXT("item_purchase"), EventData);
}

void UTokebiAnalyticsFunctions::TokebiFlushEvents()
{
    UE_LOG(LogTokebiAnalytics, Verbose, TEXT("Manual flush requested"));
    FlushQueuedEvents();
}

void UTokebiAnalyticsFunctions::InitializeTokebiSystem()
{
    if (bSystemInitialized)
    {
        return;
    }
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Initializing Tokebi Analytics system"));
    
    // Load any offline events from previous session
    LoadEventsFromFile();
    
    // Start auto-flush using global ticker
    StartFlushTicker();
    
    // 🔧 IMPROVED: If we loaded saved events, flush them immediately
    // Don't wait for the 30-second timer
    {
        FScopeLock Lock(&EventQueueLock);
        if (EventQueue.Num() > 0)
        {
            UE_LOG(LogTokebiAnalytics, Log, TEXT("🔧 Flushing %d loaded events immediately"), EventQueue.Num());
            // Use a small delay to ensure ticker is fully set up
            FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateStatic([](float DeltaTime) -> bool {
                    UTokebiAnalyticsFunctions::FlushQueuedEvents();
                    return false; // One-time flush
                }), 
                1.0f // 1 second delay
            );
        }
    }
    
    bSystemInitialized = true;
}

void UTokebiAnalyticsFunctions::QueueEvent(const FString& EventType, const TMap<FString, FString>& EventData)
{
    const UTokebiAnalyticsSettings* Settings = GetDefault<UTokebiAnalyticsSettings>();
    
    if (!Settings || Settings->TokebiApiKey.IsEmpty() || Settings->TokebiGameId.IsEmpty())
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("Tokebi Analytics not configured! Please set API Key and Game ID in Project Settings"));
        return;
    }
    
    // Use the real game_id if available, fallback to settings
    FString GameIdToUse = RegisteredGameId.IsEmpty() ? Settings->TokebiGameId : RegisteredGameId;
    
    // Create event JSON
    TSharedPtr<FJsonObject> EventObject = MakeShareable(new FJsonObject);
    EventObject->SetStringField(TEXT("eventType"), EventType);
    EventObject->SetStringField(TEXT("gameId"), GameIdToUse);
    EventObject->SetStringField(TEXT("playerId"), GetPlayerID());
    EventObject->SetStringField(TEXT("platform"), TEXT("unreal"));
    EventObject->SetStringField(TEXT("environment"), Settings->TokebiEnvironment);
    
    // Add payload
    TSharedPtr<FJsonObject> PayloadObject = MakeShareable(new FJsonObject);
    for (const auto& Pair : EventData)
    {
        PayloadObject->SetStringField(Pair.Key, Pair.Value);
    }
    EventObject->SetObjectField(TEXT("payload"), PayloadObject);
    
    // Debug log - Show which game ID we're using
    UE_LOG(LogTokebiAnalytics, Log, TEXT("🔧 Event '%s' using gameId: %s"), *EventType, *GameIdToUse);
    
    // Add to queue (thread-safe)
    {
        FScopeLock Lock(&EventQueueLock);
        EventQueue.Add(EventObject);
        
        UE_LOG(LogTokebiAnalytics, Verbose, TEXT("Queued event: %s (Queue size: %d)"), *EventType, EventQueue.Num());
        
        // Force flush if queue is getting large
        if (EventQueue.Num() >= MAX_QUEUE_SIZE)
        {
            UE_LOG(LogTokebiAnalytics, Warning, TEXT("Event queue full, forcing flush"));
            FlushQueuedEvents();
        }
    }
}

void UTokebiAnalyticsFunctions::FlushQueuedEvents()
{
    TArray<TSharedPtr<FJsonObject>> EventsToSend;
    
    // Get events from queue (thread-safe)
    {
        FScopeLock Lock(&EventQueueLock);
        if (EventQueue.Num() == 0)
        {
            UE_LOG(LogTokebiAnalytics, Verbose, TEXT("No events to flush"));
            return;
        }
        
        EventsToSend = EventQueue;
        EventQueue.Empty();
    }
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Flushing %d events to Tokebi"), EventsToSend.Num());
    
    const UTokebiAnalyticsSettings* Settings = GetDefault<UTokebiAnalyticsSettings>();
    if (!Settings)
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("Failed to get Tokebi settings"));
        return;
    }
    
    // Create batch payload
    TSharedPtr<FJsonObject> BatchObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> EventsArray;
    
    for (const auto& Event : EventsToSend)
    {
        EventsArray.Add(MakeShareable(new FJsonValueObject(Event)));
    }
    
    BatchObject->SetArrayField(TEXT("events"), EventsArray);
    
    // Serialize to string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(BatchObject.ToSharedRef(), Writer);
    
    // Use correct track endpoint
    FString TrackEndpoint = Settings->TokebiEndpoint + TEXT("/api/track");
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Sending to endpoint: %s"), *TrackEndpoint);
    UE_LOG(LogTokebiAnalytics, Verbose, TEXT("Payload: %s"), *JsonString);
    
    SendHTTPRequest(TrackEndpoint, JsonString, [EventsToSend](bool bSuccess, int32 ResponseCode, FString ResponseBody)
    {
        if (bSuccess && ResponseCode == 200)
        {
            UE_LOG(LogTokebiAnalytics, Log, TEXT("✅ Successfully sent batch of %d events"), EventsToSend.Num());
        }
        else
        {
            UE_LOG(LogTokebiAnalytics, Warning, TEXT("❌ Failed to send events batch, response code: %d"), ResponseCode);
            UE_LOG(LogTokebiAnalytics, Warning, TEXT("Response body: %s"), *ResponseBody);
            
            // Save failed events to file for retry
            SaveEventsToFile(EventsToSend);
        }
    });
}

void UTokebiAnalyticsFunctions::RegisterGameWithTokebi()
{
    const UTokebiAnalyticsSettings* Settings = GetDefault<UTokebiAnalyticsSettings>();
    if (!Settings || Settings->TokebiApiKey.IsEmpty() || Settings->TokebiGameId.IsEmpty())
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("Cannot register game - Tokebi settings not configured"));
        return;
    }
    
    // Create game registration payload with CORRECT field names (matching RPG Maker script)
    TSharedPtr<FJsonObject> GameObject = MakeShareable(new FJsonObject);
    GameObject->SetStringField(TEXT("gameName"), Settings->TokebiGameId);
    GameObject->SetStringField(TEXT("platform"), TEXT("unreal"));
    GameObject->SetStringField(TEXT("unrealVersion"), ENGINE_VERSION_STRING);
    GameObject->SetNumberField(TEXT("playerCount"), 1);
    
    // Serialize to string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(GameObject.ToSharedRef(), Writer);
    
    // Construct games endpoint correctly
    FString GamesEndpoint = Settings->TokebiEndpoint + TEXT("/api/games");
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Registering game with endpoint: %s"), *GamesEndpoint);
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Registration payload: %s"), *JsonString);
    
    // Send registration request
    SendHTTPRequest(GamesEndpoint, JsonString, [](bool bSuccess, int32 ResponseCode, FString ResponseBody)
    {
        if (bSuccess && (ResponseCode == 200 || ResponseCode == 201))
        {
            UE_LOG(LogTokebiAnalytics, Log, TEXT("✅ Game registration successful: %s"), *ResponseBody);
            
            // Parse and store the real game_id
            TSharedPtr<FJsonObject> JsonResponse;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
            if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
            {
                FString RealGameId;
                if (JsonResponse->TryGetStringField(TEXT("game_id"), RealGameId))
                {
                    RegisteredGameId = RealGameId;
                    UE_LOG(LogTokebiAnalytics, Log, TEXT("🔧 Stored real game ID: %s"), *RegisteredGameId);
                }
                else
                {
                    UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ No game_id field in registration response"));
                }
            }
            else
            {
                UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ Failed to parse registration response JSON"));
            }
            
            OnGameRegistrationComplete(true);
        }
        else
        {
            UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ Game registration failed, response code: %d, body: %s"), ResponseCode, *ResponseBody);
            OnGameRegistrationComplete(false);
        }
    });
}

void UTokebiAnalyticsFunctions::OnGameRegistrationComplete(bool bSuccess)
{
    bGameRegistered = bSuccess;
    
    if (bSuccess)
    {
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Game is now registered with Tokebi - events will be processed"));
    }
    else
    {
        UE_LOG(LogTokebiAnalytics, Warning, TEXT("Game registration failed - events may not be processed correctly"));
    }
}

void UTokebiAnalyticsFunctions::SendHTTPRequest(const FString& Endpoint, const FString& JsonPayload, TFunction<void(bool, int32, FString)> Callback)
{
    const UTokebiAnalyticsSettings* Settings = GetDefault<UTokebiAnalyticsSettings>();
    if (!Settings)
    {
        Callback(false, 0, TEXT("Settings not available"));
        return;
    }
    
    // Create HTTP request
    TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetURL(Endpoint);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Authorization"), Settings->TokebiApiKey);
    HttpRequest->SetContentAsString(JsonPayload);
    
    // Debug logging
    UE_LOG(LogTokebiAnalytics, Verbose, TEXT("HTTP Request URL: %s"), *Endpoint);
    UE_LOG(LogTokebiAnalytics, Verbose, TEXT("HTTP Request API Key: %s"), *Settings->TokebiApiKey);
    UE_LOG(LogTokebiAnalytics, VeryVerbose, TEXT("HTTP Request Body: %s"), *JsonPayload);
    
    // Set completion callback
    HttpRequest->OnProcessRequestComplete().BindLambda([Callback, Endpoint](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful && Response.IsValid())
        {
            int32 ResponseCode = Response->GetResponseCode();
            FString ResponseBody = Response->GetContentAsString();
            
            UE_LOG(LogTokebiAnalytics, Verbose, TEXT("HTTP Response [%s] Code: %d"), *Endpoint, ResponseCode);
            if (ResponseCode != 200 && ResponseCode != 201)
            {
                UE_LOG(LogTokebiAnalytics, Warning, TEXT("HTTP Response Body: %s"), *ResponseBody);
            }
            
            Callback(true, ResponseCode, ResponseBody);
        }
        else
        {
            UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ HTTP Request failed for endpoint: %s"), *Endpoint);
            Callback(false, 0, TEXT("Network error"));
        }
    });
    
    // Send request (async)
    if (!HttpRequest->ProcessRequest())
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ Failed to process HTTP request"));
        Callback(false, 0, TEXT("Failed to process request"));
    }
}

void UTokebiAnalyticsFunctions::SaveEventsToFile(const TArray<TSharedPtr<FJsonObject>>& Events)
{
    if (Events.Num() == 0)
    {
        UE_LOG(LogTokebiAnalytics, Verbose, TEXT("No events to save"));
        return;
    }
    
    FString FilePath = GetOfflineEventsPath();
    
    // 🔧 IMPROVED: Load existing events more safely
    TArray<TSharedPtr<FJsonValue>> AllEvents;
    FString ExistingJson;
    
    if (FFileHelper::LoadFileToString(ExistingJson, *FilePath) && !ExistingJson.IsEmpty())
    {
        TSharedPtr<FJsonValue> JsonValue;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExistingJson);
        if (FJsonSerializer::Deserialize(Reader, JsonValue) && JsonValue->Type == EJson::Array)
        {
            AllEvents = JsonValue->AsArray();
            UE_LOG(LogTokebiAnalytics, Verbose, TEXT("Found %d existing saved events"), AllEvents.Num());
        }
        else
        {
            UE_LOG(LogTokebiAnalytics, Warning, TEXT("Existing saved events file corrupted, starting fresh"));
            AllEvents.Empty();
        }
    }
    
    // Add new failed events
    for (const auto& Event : Events)
    {
        AllEvents.Add(MakeShareable(new FJsonValueObject(Event)));
    }
    
    // 🔧 IMPROVED: Limit total saved events to prevent unlimited growth
    const int32 MAX_SAVED_EVENTS = 500; // Prevent file from growing too large
    if (AllEvents.Num() > MAX_SAVED_EVENTS)
    {
        // Keep only the most recent events
        TArray<TSharedPtr<FJsonValue>> TrimmedEvents;
        int32 StartIndex = AllEvents.Num() - MAX_SAVED_EVENTS;
        for (int32 i = StartIndex; i < AllEvents.Num(); i++)
        {
            TrimmedEvents.Add(AllEvents[i]);
        }
        AllEvents = TrimmedEvents;
        UE_LOG(LogTokebiAnalytics, Warning, TEXT("Trimmed saved events to %d (max %d)"), 
               AllEvents.Num(), MAX_SAVED_EVENTS);
    }
    
    // Serialize to string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    if (!FJsonSerializer::Serialize(AllEvents, Writer))
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ Failed to serialize events to JSON"));
        return;
    }
    
    // Ensure directory exists
    FString DirectoryPath = FPaths::GetPath(FilePath);
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*DirectoryPath))
    {
        if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath))
        {
            UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ Failed to create analytics directory: %s"), *DirectoryPath);
            return;
        }
    }
    
    // Save to file
    if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
    {
        UE_LOG(LogTokebiAnalytics, Log, TEXT("✅ Saved %d failed events to file (total: %d)"), 
               Events.Num(), AllEvents.Num());
    }
    else
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("❌ Failed to save events to file: %s"), *FilePath);
    }
}

void UTokebiAnalyticsFunctions::LoadEventsFromFile()
{
    FString FilePath = GetOfflineEventsPath();
    FString SavedJson;
    
    if (!FFileHelper::LoadFileToString(SavedJson, *FilePath))
    {
        UE_LOG(LogTokebiAnalytics, Verbose, TEXT("No saved events file found"));
        return; // No saved events
    }
    
    TArray<TSharedPtr<FJsonValue>> SavedEventsArray;
    TSharedPtr<FJsonValue> JsonValue;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SavedJson);
    
    if (FJsonSerializer::Deserialize(Reader, JsonValue) && JsonValue->Type == EJson::Array)
    {
        SavedEventsArray = JsonValue->AsArray();
        
        // 🔧 IMPROVED: Process events and fix game IDs if needed
        const UTokebiAnalyticsSettings* Settings = GetDefault<UTokebiAnalyticsSettings>();
        int32 EventsLoaded = 0;
        int32 EventsFixed = 0;
        
        {
            FScopeLock Lock(&EventQueueLock);
            for (const auto& EventValue : SavedEventsArray)
            {
                if (EventValue->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> EventObj = EventValue->AsObject();
                    
                    // 🔧 IMPROVED: Check and fix game ID if needed
                    FString CurrentGameId;
                    if (EventObj->TryGetStringField(TEXT("gameId"), CurrentGameId))
                    {
                        // If event has old game ID (from settings) but we have a registered ID, update it
                        if (!RegisteredGameId.IsEmpty() && 
                            CurrentGameId == Settings->TokebiGameId && 
                            CurrentGameId != RegisteredGameId)
                        {
                            EventObj->SetStringField(TEXT("gameId"), RegisteredGameId);
                            EventsFixed++;
                            UE_LOG(LogTokebiAnalytics, Log, TEXT("🔧 Fixed game ID in saved event: %s → %s"), 
                                   *CurrentGameId, *RegisteredGameId);
                        }
                    }
                    
                    EventQueue.Add(EventObj);
                    EventsLoaded++;
                }
            }
        }
        
        UE_LOG(LogTokebiAnalytics, Log, TEXT("✅ Loaded %d saved events for retry (%d game IDs fixed)"), 
               EventsLoaded, EventsFixed);
        
        // Clear the saved file since we've loaded the events
        if (FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*FilePath))
        {
            UE_LOG(LogTokebiAnalytics, Verbose, TEXT("Cleared saved events file"));
        }
    }
    else
    {
        UE_LOG(LogTokebiAnalytics, Warning, TEXT("❌ Failed to parse saved events JSON, deleting corrupted file"));
        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*FilePath);
    }
}

FString UTokebiAnalyticsFunctions::GetOfflineEventsPath()
{
    return FPaths::ProjectSavedDir() / TEXT("Analytics") / TEXT("TokebiOfflineEvents.json");
}

// TICKER IMPLEMENTATION - NO WORLD CONTEXT NEEDED
void UTokebiAnalyticsFunctions::StartFlushTicker()
{
    // Remove existing ticker if any
    if (FlushTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(FlushTickerHandle);
    }
    
    // Add new ticker that runs every 30 seconds
    FlushTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateStatic([](float DeltaTime) -> bool {
            UTokebiAnalyticsFunctions::FlushQueuedEvents();
            return true; // Keep ticking
        }), 
        FLUSH_INTERVAL
    );
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("✅ Flush ticker started (%.1f seconds interval) - NO WORLD CONTEXT NEEDED"), FLUSH_INTERVAL);
}

void UTokebiAnalyticsFunctions::StopFlushTicker()
{
    if (FlushTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(FlushTickerHandle);
        FlushTickerHandle.Reset();
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Flush ticker stopped"));
    }
}

FString UTokebiAnalyticsFunctions::GetPlayerID()
{
    // Simple player ID generation - could be made persistent
    static FString PlayerID;
    if (PlayerID.IsEmpty())
    {
        // Try to load existing player ID first
        FString SavedPlayerID;
        FString PlayerIDPath = FPaths::ProjectSavedDir() / TEXT("Analytics") / TEXT("TokebiPlayerID.txt");
        
        if (FFileHelper::LoadFileToString(SavedPlayerID, *PlayerIDPath) && !SavedPlayerID.IsEmpty())
        {
            PlayerID = SavedPlayerID.TrimStartAndEnd();
            UE_LOG(LogTokebiAnalytics, Log, TEXT("Loaded existing player ID: %s"), *PlayerID);
        }
        else
        {
            // Generate new player ID (matching RPG Maker format)
            PlayerID = FString::Printf(TEXT("player_%lld_%s"), 
                                     FDateTime::UtcNow().ToUnixTimestamp(),
                                     *FGuid::NewGuid().ToString(EGuidFormats::Digits).Right(8));
            
            // Save the player ID
            FString DirectoryPath = FPaths::GetPath(PlayerIDPath);
            if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*DirectoryPath))
            {
                FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath);
            }
            
            FFileHelper::SaveStringToFile(PlayerID, *PlayerIDPath);
            UE_LOG(LogTokebiAnalytics, Log, TEXT("Generated new player ID: %s"), *PlayerID);
        }
    }
    return PlayerID;
}

FString UTokebiAnalyticsFunctions::GenerateSessionID()
{
    return FString::Printf(TEXT("session_%lld_%s"), 
                          FDateTime::UtcNow().ToUnixTimestamp(),
                          *FGuid::NewGuid().ToString(EGuidFormats::Digits).Right(8));
}