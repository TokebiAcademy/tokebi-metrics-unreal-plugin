// =============================================================================
// TOKEBI ANALYTICS - BASIC USAGE EXAMPLES
// =============================================================================
// This file shows common patterns for integrating Tokebi Analytics
// in your Unreal Engine project using both C++ and Blueprint approaches.

#pragma once

#include "CoreMinimal.h"
#include "Analytics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

// =============================================================================
// EXAMPLE 1: GAME MODE INTEGRATION
// =============================================================================
// Typical pattern: Start session when game begins, end when game ends

class YOURGAME_API AYourGameMode : public AGameModeBase
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        
        // Start analytics session when game begins
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            Attributes.Add(FAnalyticsEventAttribute(TEXT("game_version"), TEXT("1.0.0")));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("level_name"), GetWorld()->GetMapName()));
            
            FAnalytics::Get().GetDefaultProvider()->StartSession(Attributes);
            UE_LOG(LogTemp, Log, TEXT("Tokebi Analytics session started"));
        }
    }
    
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
    {
        // End analytics session when game ends
        if (FAnalytics::IsAvailable())
        {
            FAnalytics::Get().GetDefaultProvider()->EndSession();
            UE_LOG(LogTemp, Log, TEXT("Tokebi Analytics session ended"));
        }
        
        Super::EndPlay(EndPlayReason);
    }
};

// =============================================================================
// EXAMPLE 2: PLAYER CONTROLLER INTEGRATION  
// =============================================================================
// Track player actions and achievements

class YOURGAME_API AYourPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    // Call this when player completes a level
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void TrackLevelComplete(int32 LevelNumber, float CompletionTime, int32 Score)
    {
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            Attributes.Add(FAnalyticsEventAttribute(TEXT("level"), FString::FromInt(LevelNumber)));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("completion_time"), FString::SanitizeFloat(CompletionTime)));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("score"), FString::FromInt(Score)));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("attempts"), FString::FromInt(GetLevelAttempts(LevelNumber))));
            
            FAnalytics::Get().GetDefaultProvider()->RecordProgress(
                TEXT("level_complete"),
                FString::Printf(TEXT("level_%d"), LevelNumber),
                Attributes
            );
            
            UE_LOG(LogTemp, Log, TEXT("Tracked level %d completion: %.2fs, score %d"), 
                   LevelNumber, CompletionTime, Score);
        }
    }
    
    // Call this when player makes a purchase
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void TrackItemPurchase(const FString& ItemId, int32 Cost, const FString& Currency)
    {
        if (FAnalytics::IsAvailable())
        {
            FAnalytics::Get().GetDefaultProvider()->RecordItemPurchase(
                ItemId,
                Currency,
                Cost,
                1  // Quantity
            );
            
            UE_LOG(LogTemp, Log, TEXT("Tracked purchase: %s for %d %s"), 
                   *ItemId, Cost, *Currency);
        }
    }
    
    // Call this for custom game events
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void TrackCustomEvent(const FString& EventName, const TMap<FString, FString>& EventData)
    {
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            
            // Convert TMap to Analytics Attributes
            for (const auto& Pair : EventData)
            {
                Attributes.Add(FAnalyticsEventAttribute(Pair.Key, Pair.Value));
            }
            
            // Add timestamp
            Attributes.Add(FAnalyticsEventAttribute(TEXT("timestamp"), 
                          FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp())));
            
            FAnalytics::Get().GetDefaultProvider()->RecordEvent(EventName, Attributes);
            
            UE_LOG(LogTemp, Log, TEXT("Tracked custom event: %s"), *EventName);
        }
    }

private:
    // Helper function to track level attempts
    int32 GetLevelAttempts(int32 LevelNumber)
    {
        // Implement your logic to track attempts
        // This is just an example
        return 1;
    }
};

// =============================================================================
// EXAMPLE 3: UI WIDGET INTEGRATION
// =============================================================================
// Track UI interactions and menu navigation

class YOURGAME_API UYourMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // Call this when any button is clicked
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void TrackButtonClick(const FString& ButtonName, const FString& MenuName)
    {
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            Attributes.Add(FAnalyticsEventAttribute(TEXT("button_name"), ButtonName));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("menu_name"), MenuName));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("click_time"), 
                          FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp())));
            
            FAnalytics::Get().GetDefaultProvider()->RecordEvent(TEXT("button_clicked"), Attributes);
        }
    }
    
    // Call this when navigating between menus
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void TrackMenuNavigation(const FString& FromMenu, const FString& ToMenu)
    {
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            Attributes.Add(FAnalyticsEventAttribute(TEXT("from_menu"), FromMenu));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("to_menu"), ToMenu));
            
            FAnalytics::Get().GetDefaultProvider()->RecordEvent(TEXT("menu_navigation"), Attributes);
        }
    }
};

// =============================================================================
// EXAMPLE 4: ERROR AND CRASH REPORTING
// =============================================================================
// Track errors and unexpected events

class YOURGAME_API UYourAnalyticsHelper : public UObject
{
    GENERATED_BODY()

public:
    // Call this when an error occurs
    UFUNCTION(BlueprintCallable, Category = "Analytics", CallInEditor = true)
    static void TrackError(const FString& ErrorMessage, const FString& ErrorContext = TEXT(""))
    {
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            if (!ErrorContext.IsEmpty())
            {
                Attributes.Add(FAnalyticsEventAttribute(TEXT("context"), ErrorContext));
            }
            Attributes.Add(FAnalyticsEventAttribute(TEXT("timestamp"), 
                          FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp())));
            
            FAnalytics::Get().GetDefaultProvider()->RecordError(ErrorMessage, Attributes);
            
            UE_LOG(LogTemp, Warning, TEXT("Tracked error: %s (Context: %s)"), 
                   *ErrorMessage, *ErrorContext);
        }
    }
    
    // Call this for performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    static void TrackPerformanceMetric(const FString& MetricName, float Value, const FString& Unit = TEXT(""))
    {
        if (FAnalytics::IsAvailable())
        {
            TArray<FAnalyticsEventAttribute> Attributes;
            Attributes.Add(FAnalyticsEventAttribute(TEXT("metric_name"), MetricName));
            Attributes.Add(FAnalyticsEventAttribute(TEXT("value"), FString::SanitizeFloat(Value)));
            if (!Unit.IsEmpty())
            {
                Attributes.Add(FAnalyticsEventAttribute(TEXT("unit"), Unit));
            }
            
            FAnalytics::Get().GetDefaultProvider()->RecordEvent(TEXT("performance_metric"), Attributes);
        }
    }
};

// =============================================================================
// BLUEPRINT USAGE PATTERNS
// =============================================================================

/*
BLUEPRINT SETUP EXAMPLES:

1. LEVEL BLUEPRINT - Session Management:
   [Event BeginPlay] → [Start Session]
                       ├ Attributes: Add "level_name" with current level
                       └ Attributes: Add "game_mode" with current mode

2. PLAYER BLUEPRINT - Action Tracking:
   [InputAction Jump] → [Record Event]
                        ├ Event Name: "player_jump"
                        └ Attributes: Add "location" with player position

3. UI BLUEPRINT - Button Tracking:
   [Button OnClicked] → [Record Event]
                        ├ Event Name: "button_clicked"  
                        └ Attributes: Add "button_name" with button identifier

4. GAME MODE BLUEPRINT - Progress Tracking:
   [Custom Event: Level Complete] → [Record Progress]
                                    ├ Progress Type: "level_complete"
                                    ├ Progress Name: "level_1"
                                    └ Attributes: Add completion data

COMMON ATTRIBUTE PATTERNS:
- Player position: GetActorLocation converted to string
- Game time: Get Game Time In Seconds converted to string  
- Player stats: Health, score, level, etc.
- Session info: Build version, platform, etc.
- Timestamps: Current time for timing events
*/

// =============================================================================
// BEST PRACTICES
// =============================================================================

/*
1. SESSION MANAGEMENT:
   - Start session in GameMode::BeginPlay
   - End session in GameMode::EndPlay
   - Include game version and platform info in session start

2. EVENT NAMING:
   - Use consistent naming: "level_start", "level_complete", "button_clicked"
   - Avoid spaces, use underscores
   - Be descriptive but concise

3. ATTRIBUTE GUIDELINES:
   - Include context: level name, menu name, item ID
   - Add timestamps for time-sensitive events
   - Use consistent data types (strings for text, proper number formatting)
   - Don't include sensitive user data

4. PERFORMANCE:
   - Events are automatically batched and sent asynchronously
   - Don't call FlushEvents() frequently - let the timer handle it
   - Avoid sending excessive events (hundreds per second)

5. ERROR HANDLING:
   - Always check FAnalytics::IsAvailable() before calling
   - Log analytics calls for debugging
   - Handle network failures gracefully (plugin does this automatically)

6. TESTING:
   - Use development API keys for testing
   - Enable verbose logging to see event flow
   - Test with airplane mode to verify offline queueing
*/
