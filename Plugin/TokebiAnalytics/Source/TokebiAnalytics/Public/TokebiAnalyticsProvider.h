#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "Http.h"
#include "Dom/JsonObject.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTokebiProvider, Log, All);

/**
 * Tokebi Analytics Provider implementation
 * Implements the IAnalyticsProvider interface to send events to Tokebi's REST API
 */
class TOKEBIANALYTICS_API FTokebiAnalyticsProvider : public IAnalyticsProvider
{
public:
    /**
     * Constructor
     * @param InApiKey API key for Tokebi authentication
     * @param InEndpoint Tokebi API endpoint URL
     * @param InEnvironment Environment (development/production)
     * @param InGameId Game identifier
     */
    FTokebiAnalyticsProvider(const FString& InApiKey, const FString& InEndpoint, 
                           const FString& InEnvironment, const FString& InGameId);
    
    /** Virtual destructor */
    virtual ~FTokebiAnalyticsProvider();

    // IAnalyticsProvider interface implementation
    virtual bool StartSession(const TArray<FAnalyticsEventAttribute>& Attributes) override;
    virtual void EndSession() override;
    virtual void FlushEvents() override;
    virtual void SetUserID(const FString& InUserID) override;
    virtual FString GetUserID() const override;
    virtual FString GetSessionID() const override;
    virtual bool SetSessionID(const FString& InSessionID) override;
    virtual void RecordEvent(const FString& EventName, const TArray<FAnalyticsEventAttribute>& Attributes) override;
    virtual void RecordItemPurchase(const FString& ItemId, const FString& Currency, int PerItemCost, int ItemQuantity) override;
    virtual void RecordCurrencyPurchase(const FString& GameCurrencyType, int GameCurrencyAmount, const FString& RealCurrencyType, float RealMoneyCost, const FString& PaymentProvider) override;
    virtual void RecordCurrencyGiven(const FString& GameCurrencyType, int GameCurrencyAmount) override;
    virtual void RecordError(const FString& Error, const TArray<FAnalyticsEventAttribute>& Attributes) override;
    virtual void RecordProgress(const FString& ProgressType, const FString& ProgressHierarchy, const TArray<FAnalyticsEventAttribute>& Attributes) override;

private:
    /** Configuration */
    FString ApiKey;
    FString Endpoint;
    FString Environment;
    FString GameId;
    FString Platform;
    
    /** Session data */
    FString UserID;
    FString SessionID;
    bool bSessionStarted;
    
    /** Event queue for batching */
    TArray<TSharedPtr<FJsonObject>> EventQueue;
    FCriticalSection EventQueueCriticalSection;
    
    /** HTTP module reference */
    FHttpModule* HttpModule;
    
    /** Timer for periodic flush */
    FTimerHandle FlushTimerHandle;
    
    /**
     * Generates a unique player ID if none exists
     * @return Generated player ID
     */
    FString GeneratePlayerID();
    
    /**
     * Generates a unique session ID
     * @return Generated session ID
     */
    FString GenerateSessionID();
    
    /**
     * Converts analytics attributes to JSON object
     * @param Attributes Input attributes array
     * @return JSON object with converted attributes
     */
    TSharedPtr<FJsonObject> AttributesToJson(const TArray<FAnalyticsEventAttribute>& Attributes);
    
    /**
     * Creates a JSON payload for an event
     * @param EventType Event type/name
     * @param Payload Event payload data
     * @return Complete JSON object ready for API submission
     */
    TSharedPtr<FJsonObject> CreateEventPayload(const FString& EventType, TSharedPtr<FJsonObject> Payload);
    
    /**
     * Queues an event for sending
     * @param EventType Event type
     * @param Payload Event payload
     */
    void QueueEvent(const FString& EventType, TSharedPtr<FJsonObject> Payload);
    
    /**
     * Sends queued events to Tokebi API
     */
    void SendQueuedEvents();
    
    /**
     * HTTP request completion callback
     * @param Request The HTTP request that completed
     * @param Response The HTTP response received
     * @param bWasSuccessful Whether the request was successful
     */
    void OnHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    
    /**
     * Starts the periodic flush timer
     */
    void StartFlushTimer();
    
    /**
     * Stops the periodic flush timer
     */
    void StopFlushTimer();
};
