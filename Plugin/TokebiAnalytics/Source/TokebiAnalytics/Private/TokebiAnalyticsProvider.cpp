#include "TokebiAnalyticsProvider.h"
#include "Analytics.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogTokebiProvider);

#define TOKEBI_FLUSH_INTERVAL 30.0f // Flush every 30 seconds

FTokebiAnalyticsProvider::FTokebiAnalyticsProvider(const FString& InApiKey, const FString& InEndpoint, 
                                                 const FString& InEnvironment, const FString& InGameId)
    : ApiKey(InApiKey)
    , Endpoint(InEndpoint)
    , Environment(InEnvironment)
    , GameId(InGameId)
    , Platform(TEXT("unreal"))
    , bSessionStarted(false)
    , HttpModule(&FHttpModule::Get())
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Tokebi Analytics Provider initialized"));
    UE_LOG(LogTokebiProvider, Log, TEXT("Endpoint: %s, Environment: %s, GameId: %s"), *Endpoint, *Environment, *GameId);
    
    // Generate initial user ID if not set
    UserID = GeneratePlayerID();
    
    // Start flush timer
    StartFlushTimer();
}

FTokebiAnalyticsProvider::~FTokebiAnalyticsProvider()
{
    StopFlushTimer();
    FlushEvents();
    UE_LOG(LogTokebiProvider, Log, TEXT("Tokebi Analytics Provider destroyed"));
}

bool FTokebiAnalyticsProvider::StartSession(const TArray<FAnalyticsEventAttribute>& Attributes)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Starting session"));
    
    if (bSessionStarted)
    {
        UE_LOG(LogTokebiProvider, Warning, TEXT("Session already started"));
        return false;
    }
    
    SessionID = GenerateSessionID();
    bSessionStarted = true;
    
    // Create session start event payload
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("sessionId"), SessionID);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    // Add custom attributes
    if (Attributes.Num() > 0)
    {
        TSharedPtr<FJsonObject> AttributesJson = AttributesToJson(Attributes);
        for (auto& Pair : AttributesJson->Values)
        {
            Payload->SetField(Pair.Key, Pair.Value);
        }
    }
    
    QueueEvent(TEXT("session_start"), Payload);
    
    UE_LOG(LogTokebiProvider, Log, TEXT("Session started with ID: %s"), *SessionID);
    return true;
}

void FTokebiAnalyticsProvider::EndSession()
{
    if (!bSessionStarted)
    {
        UE_LOG(LogTokebiProvider, Warning, TEXT("No active session to end"));
        return;
    }
    
    UE_LOG(LogTokebiProvider, Log, TEXT("Ending session: %s"), *SessionID);
    
    // Create session end event payload
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("sessionId"), SessionID);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    QueueEvent(TEXT("session_end"), Payload);
    
    // Flush immediately for session end
    FlushEvents();
    
    bSessionStarted = false;
    SessionID.Empty();
}

void FTokebiAnalyticsProvider::FlushEvents()
{
    SendQueuedEvents();
}

void FTokebiAnalyticsProvider::SetUserID(const FString& InUserID)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Setting user ID: %s"), *InUserID);
    UserID = InUserID;
}

FString FTokebiAnalyticsProvider::GetUserID() const
{
    return UserID;
}

FString FTokebiAnalyticsProvider::GetSessionID() const
{
    return SessionID;
}

bool FTokebiAnalyticsProvider::SetSessionID(const FString& InSessionID)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Setting session ID: %s"), *InSessionID);
    SessionID = InSessionID;
    return true;
}

void FTokebiAnalyticsProvider::RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes)
{
    UE_LOG(LogTokebiProvider, Verbose, TEXT("Recording event: %s"), *EventName);
    
    // Create event payload
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    // Add session ID if available
    if (!SessionID.IsEmpty())
    {
        Payload->SetStringField(TEXT("sessionId"), SessionID);
    }
    
    // Add custom attributes
    if (Attributes.Num() > 0)
    {
        TSharedPtr<FJsonObject> AttributesJson = AttributesToJson(Attributes);
        for (auto& Pair : AttributesJson->Values)
        {
            Payload->SetField(Pair.Key, Pair.Value);
        }
    }
    
    QueueEvent(EventName, Payload);
}

void FTokebiAnalyticsProvider::RecordItemPurchase(const FString& ItemId, const FString& Currency, int PerItemCost, int ItemQuantity)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Recording item purchase: %s"), *ItemId);
    
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("itemId"), ItemId);
    Payload->SetStringField(TEXT("currency"), Currency);
    Payload->SetNumberField(TEXT("perItemCost"), PerItemCost);
    Payload->SetNumberField(TEXT("itemQuantity"), ItemQuantity);
    Payload->SetNumberField(TEXT("totalCost"), PerItemCost * ItemQuantity);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    if (!SessionID.IsEmpty())
    {
        Payload->SetStringField(TEXT("sessionId"), SessionID);
    }
    
    QueueEvent(TEXT("item_purchase"), Payload);
}

void FTokebiAnalyticsProvider::RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, 
                                                     const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Recording currency purchase: %d %s for %f %s"), 
           GameCurrencyAmount, *GameCurrencyType, RealMoneyCost, *RealCurrencyType);
    
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("gameCurrencyType"), GameCurrencyType);
    Payload->SetNumberField(TEXT("gameCurrencyAmount"), GameCurrencyAmount);
    Payload->SetStringField(TEXT("realCurrencyType"), RealCurrencyType);
    Payload->SetNumberField(TEXT("realMoneyCost"), RealMoneyCost);
    Payload->SetStringField(TEXT("paymentProvider"), PaymentProvider);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    if (!SessionID.IsEmpty())
    {
        Payload->SetStringField(TEXT("sessionId"), SessionID);
    }
    
    QueueEvent(TEXT("currency_purchase"), Payload);
}

void FTokebiAnalyticsProvider::RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Recording currency given: %d %s"), GameCurrencyAmount, *GameCurrencyType);
    
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("gameCurrencyType"), GameCurrencyType);
    Payload->SetNumberField(TEXT("gameCurrencyAmount"), GameCurrencyAmount);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    if (!SessionID.IsEmpty())
    {
        Payload->SetStringField(TEXT("sessionId"), SessionID);
    }
    
    QueueEvent(TEXT("currency_given"), Payload);
}

void FTokebiAnalyticsProvider::RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& Attributes)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Recording error: %s"), *Error);
    
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("error"), Error);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    if (!SessionID.IsEmpty())
    {
        Payload->SetStringField(TEXT("sessionId"), SessionID);
    }
    
    // Add custom attributes
    if (Attributes.Num() > 0)
    {
        TSharedPtr<FJsonObject> AttributesJson = AttributesToJson(Attributes);
        for (auto& Pair : AttributesJson->Values)
        {
            Payload->SetField(Pair.Key, Pair.Value);
        }
    }
    
    QueueEvent(TEXT("error"), Payload);
}

void FTokebiAnalyticsProvider::RecordProgress(const FString& ProgressType, const FString& ProgressHierarchy, 
                                            const TArray<FAnalyticsEventAttribute>& Attributes)
{
    UE_LOG(LogTokebiProvider, Log, TEXT("Recording progress: %s - %s"), *ProgressType, *ProgressHierarchy);
    
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("progressType"), ProgressType);
    Payload->SetStringField(TEXT("progressHierarchy"), ProgressHierarchy);
    Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
    
    if (!SessionID.IsEmpty())
    {
        Payload->SetStringField(TEXT("sessionId"), SessionID);
    }
    
    // Add custom attributes
    if (Attributes.Num() > 0)
    {
        TSharedPtr<FJsonObject> AttributesJson = AttributesToJson(Attributes);
        for (auto& Pair : AttributesJson->Values)
        {
            Payload->SetField(Pair.Key, Pair.Value);
        }
    }
    
    QueueEvent(TEXT("progress"), Payload);
}

FString FTokebiAnalyticsProvider::GeneratePlayerID()
{
    // Try to load existing player ID first
    FString SavedPlayerID;
    FString PlayerIDPath = FPaths::ProjectSavedDir() / TEXT("Analytics") / TEXT("TokebiPlayerID.txt");
    
    if (FFileHelper::LoadFileToString(SavedPlayerID, *PlayerIDPath) && !SavedPlayerID.IsEmpty())
    {
        UE_LOG(LogTokebiProvider, Log, TEXT("Loaded existing player ID: %s"), *SavedPlayerID);
        return SavedPlayerID.TrimStartAndEnd();
    }
    
    // Generate new player ID
    FString NewPlayerID = FString::Printf(TEXT("player_%lld_%s"), 
                                         FDateTime::UtcNow().ToUnixTimestamp(),
                                         *FGuid::NewGuid().ToString(EGuidFormats::Digits).Right(8));
    
    // Save the player ID
    FString DirectoryPath = FPaths::GetPath(PlayerIDPath);
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*DirectoryPath))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath);
    }
    
    FFileHelper::SaveStringToFile(NewPlayerID, *PlayerIDPath);
    
    UE_LOG(LogTokebiProvider, Log, TEXT("Generated new player ID: %s"), *NewPlayerID);
    return NewPlayerID;
}

FString FTokebiAnalyticsProvider::GenerateSessionID()
{
    return FString::Printf(TEXT("session_%lld_%s"), 
                          FDateTime::UtcNow().ToUnixTimestamp(),
                          *FGuid::NewGuid().ToString(EGuidFormats::Digits).Right(8));
}

TSharedPtr<FJsonObject> FTokebiAnalyticsProvider::AttributesToJson(const TArray<FAnalyticsEventAttribute>& Attributes)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    
    for (const FAnalyticsEventAttribute& Attribute : Attributes)
    {
        if (Attribute.GetValue().IsEmpty())
        {
            continue;
        }
        
        // Try to parse as number first
        if (Attribute.GetValue().IsNumeric())
        {
            if (Attribute.GetValue().Contains(TEXT(".")))
            {
                JsonObject->SetNumberField(Attribute.GetName(), FCString::Atof(*Attribute.GetValue()));
            }
            else
            {
                JsonObject->SetNumberField(Attribute.GetName(), FCString::Atoi(*Attribute.GetValue()));
            }
        }
        else
        {
            JsonObject->SetStringField(Attribute.GetName(), Attribute.GetValue());
        }
    }
    
    return JsonObject;
}

TSharedPtr<FJsonObject> FTokebiAnalyticsProvider::CreateEventPayload(const FString& EventType, TSharedPtr<FJsonObject> Payload)
{
    TSharedPtr<FJsonObject> EventPayload = MakeShareable(new FJsonObject);
    
    EventPayload->SetStringField(TEXT("eventType"), EventType);
    EventPayload->SetObjectField(TEXT("payload"), Payload);
    EventPayload->SetStringField(TEXT("gameId"), GameId);
    EventPayload->SetStringField(TEXT("playerId"), UserID);
    EventPayload->SetStringField(TEXT("platform"), Platform);
    EventPayload->SetStringField(TEXT("environment"), Environment);
    
    return EventPayload;
}

void FTokebiAnalyticsProvider::QueueEvent(const FString& EventType, TSharedPtr<FJsonObject> Payload)
{
    TSharedPtr<FJsonObject> EventPayload = CreateEventPayload(EventType, Payload);
    
    {
        FScopeLock Lock(&EventQueueCriticalSection);
        EventQueue.Add(EventPayload);
    }
    
    UE_LOG(LogTokebiProvider, Verbose, TEXT("Queued event: %s (Queue size: %d)"), *EventType, EventQueue.Num());
}

void FTokebiAnalyticsProvider::SendQueuedEvents()
{
    TArray<TSharedPtr<FJsonObject>> EventsToSend;
    
    {
        FScopeLock Lock(&EventQueueCriticalSection);
        if (EventQueue.Num() == 0)
        {
            return;
        }
        
        EventsToSend = EventQueue;
        EventQueue.Empty();
    }
    
    UE_LOG(LogTokebiProvider, Log, TEXT("Sending %d queued events to Tokebi"), EventsToSend.Num());
    
    for (TSharedPtr<FJsonObject> Event : EventsToSend)
    {
        // Create HTTP request
        TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetURL(Endpoint + TEXT("/track"));
        HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        HttpRequest->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + ApiKey);
        
        // Serialize JSON
        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(Event.ToSharedRef(), Writer);
        
        HttpRequest->SetContentAsString(JsonString);
        
        // Set completion callback
        HttpRequest->OnProcessRequestComplete().BindUObject(this, &FTokebiAnalyticsProvider::OnHttpRequestComplete);
        
        // Send request
        HttpRequest->ProcessRequest();
    }
}

void FTokebiAnalyticsProvider::OnHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        int32 ResponseCode = Response->GetResponseCode();
        if (ResponseCode == 200)
        {
            UE_LOG(LogTokebiProvider, Verbose, TEXT("Event sent successfully"));
        }
        else
        {
            UE_LOG(LogTokebiProvider, Warning, TEXT("Event send failed with response code: %d"), ResponseCode);
            UE_LOG(LogTokebiProvider, Warning, TEXT("Response: %s"), *Response->GetContentAsString());
        }
    }
    else
    {
        UE_LOG(LogTokebiProvider, Error, TEXT("Failed to send event - network error"));
    }
}

void FTokebiAnalyticsProvider::StartFlushTimer()
{
    if (GEngine && GEngine->GetWorld())
    {
        GEngine->GetWorld()->GetTimerManager().SetTimer(
            FlushTimerHandle,
            FTimerDelegate::CreateUObject(this, &FTokebiAnalyticsProvider::FlushEvents),
            TOKEBI_FLUSH_INTERVAL,
            true
        );
        
        UE_LOG(LogTokebiProvider, Log, TEXT("Flush timer started (%.1f seconds interval)"), TOKEBI_FLUSH_INTERVAL);
    }
}

void FTokebiAnalyticsProvider::StopFlushTimer()
{
    if (GEngine && GEngine->GetWorld() && FlushTimerHandle.IsValid())
    {
        GEngine->GetWorld()->GetTimerManager().ClearTimer(FlushTimerHandle);
        UE_LOG(LogTokebiProvider, Log, TEXT("Flush timer stopped"));
    }
}
