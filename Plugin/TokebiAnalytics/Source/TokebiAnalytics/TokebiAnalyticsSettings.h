#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TokebiAnalyticsSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class TOKEBIANALYTICS_API UTokebiAnalyticsSettings : public UObject
{
    GENERATED_BODY()
    
public:
    UTokebiAnalyticsSettings(const FObjectInitializer& ObjectInitializer);
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="API Key"))
    FString TokebiApiKey;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Game ID"))
    FString TokebiGameId;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="API Endpoint"))
    FString TokebiEndpoint;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Environment"))
    FString TokebiEnvironment;
};