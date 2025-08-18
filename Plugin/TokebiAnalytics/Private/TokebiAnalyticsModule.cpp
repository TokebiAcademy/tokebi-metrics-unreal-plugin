#include "TokebiAnalyticsModule.h"
#include "TokebiAnalyticsProvider.h"
#include "Analytics.h"
#include "Core.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogTokebiAnalytics, Log, All);

#define LOCTEXT_NAMESPACE "FTokebiAnalyticsModule"

void FTokebiAnalyticsModule::StartupModule()
{
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Analytics module starting up"));
    
    // Register this module as an analytics provider factory
    FAnalytics::Get().RegisterProviderFactory(
        TEXT("Tokebi"),
        FAnalyticsProviderFactory::CreateLambda([this](const FAnalyticsProviderConfigurationDelegate& GetConfigValue) -> TSharedPtr<IAnalyticsProvider>
        {
            return CreateAnalyticsProvider(GetConfigValue);
        })
    );
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Analytics provider factory registered"));
}

void FTokebiAnalyticsModule::ShutdownModule()
{
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Analytics module shutting down"));
    
    // Unregister the provider factory
    if (FAnalytics::IsAvailable())
    {
        FAnalytics::Get().UnregisterProviderFactory(TEXT("Tokebi"));
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Analytics provider factory unregistered"));
    }
}

TSharedPtr<IAnalyticsProvider> FTokebiAnalyticsModule::CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const
{
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Creating Tokebi Analytics provider instance"));
    
    // Read configuration values
    FString ApiKey = GetConfigValue(TEXT("TokebiApiKey"), true);
    FString Endpoint = GetConfigValue(TEXT("TokebiEndpoint"), false);
    FString Environment = GetConfigValue(TEXT("TokebiEnvironment"), false);
    FString GameId = GetConfigValue(TEXT("TokebiGameId"), false);
    
    // Set defaults if not provided
    if (Endpoint.IsEmpty())
    {
        Endpoint = TEXT("https://tokebi-api.vercel.app");
    }
    
    if (Environment.IsEmpty())
    {
        Environment = TEXT("production");
    }
    
    // Validate required config
    if (ApiKey.IsEmpty())
    {
        UE_LOG(LogTokebiAnalytics, Error, TEXT("TokebiApiKey is required but not configured"));
        return nullptr;
    }
    
    if (GameId.IsEmpty())
    {
        UE_LOG(LogTokebiAnalytics, Warning, TEXT("TokebiGameId not configured - using default"));
        GameId = TEXT("unreal_game");
    }
    
    UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Config - Endpoint: %s, Environment: %s, GameId: %s"), 
           *Endpoint, *Environment, *GameId);
    
    // Create and return the provider
    return MakeShared<FTokebiAnalyticsProvider>(ApiKey, Endpoint, Environment, GameId);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTokebiAnalyticsModule, TokebiAnalytics)
