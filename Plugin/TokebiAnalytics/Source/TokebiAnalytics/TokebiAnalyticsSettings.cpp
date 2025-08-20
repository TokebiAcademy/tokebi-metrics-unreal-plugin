#include "TokebiAnalyticsSettings.h"

UTokebiAnalyticsSettings::UTokebiAnalyticsSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TokebiApiKey(TEXT(""))
    , TokebiGameId(TEXT(""))
    , TokebiEndpoint(TEXT("https://tokebi-api.vercel.app"))  // 🔧 REMOVED /track
    , TokebiEnvironment(TEXT("development"))
{
}