#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "TokebiAnalyticsFunctions.generated.h"

UCLASS()
class TOKEBIANALYTICS_API UTokebiAnalyticsFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiRegisterGame();
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiStartSession();
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiEndSession();
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiTrack(FString EventName, const TMap<FString, FString>& EventData);
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiTrackLevelStart(FString LevelName);
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiTrackLevelComplete(FString LevelName, float CompletionTime, int32 Score);
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiTrackPurchase(FString ItemId, FString Currency, int32 Cost);
    
    UFUNCTION(BlueprintCallable, meta = (Keywords = "Tokebi analytics"), Category = "Tokebi Analytics")
    static void TokebiFlushEvents();

private:
    // Core system
    static void InitializeTokebiSystem();
    static void QueueEvent(const FString& EventType, const TMap<FString, FString>& EventData);
    static void FlushQueuedEvents();
    
    // 🔧 TICKER FUNCTIONS - ADDED
    static void StartFlushTicker();
    static void StopFlushTicker();
    
    // Game registration
    static void RegisterGameWithTokebi();
    static void OnGameRegistrationComplete(bool bSuccess);
    
    // HTTP handling
    static void SendHTTPRequest(const FString& Endpoint, const FString& JsonPayload, TFunction<void(bool, int32, FString)> Callback);
    
    // Offline persistence
    static void SaveEventsToFile(const TArray<TSharedPtr<class FJsonObject>>& Events);
    static void LoadEventsFromFile();
    static FString GetOfflineEventsPath();
    
    // Utility
    static FString GetPlayerID();
    static FString GenerateSessionID();
    
    // State management
    static bool bSystemInitialized;
    static bool bGameRegistered;
    static FString CurrentSessionID;
};