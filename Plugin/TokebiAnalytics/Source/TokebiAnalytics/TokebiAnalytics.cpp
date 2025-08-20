#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "TokebiAnalyticsSettings.h"
#include "TokebiAnalyticsFunctions.h"
#include "ISettingsModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogTokebiAnalytics, Log, All);

#define LOCTEXT_NAMESPACE "TokebiAnalytics"

class FTokebiAnalyticsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Analytics module started"));
        
        // Register settings
        if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
        {
            SettingsModule->RegisterSettings("Project", "Plugins", "TokebiAnalytics",
                                             LOCTEXT("RuntimeSettingsName", "Tokebi Analytics"),
                                             LOCTEXT("RuntimeSettingsDescription", "Configure the Tokebi Analytics plugin"),
                                             GetMutableDefault<UTokebiAnalyticsSettings>()
                                             );
        }
        
        // Auto-register game on startup
        if (GEngine)
        {
            UE_LOG(LogTokebiAnalytics, Log, TEXT("Auto-registering game with Tokebi"));
            UTokebiAnalyticsFunctions::TokebiRegisterGame();
        }
    }

    virtual void ShutdownModule() override
    {
        // Flush any remaining events before shutdown
        UTokebiAnalyticsFunctions::TokebiFlushEvents();
        
        // Unregister settings
        if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
        {
            SettingsModule->UnregisterSettings("Project", "Plugins", "TokebiAnalytics");
        }
        
        UE_LOG(LogTokebiAnalytics, Log, TEXT("Tokebi Analytics module shutdown"));
    }
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTokebiAnalyticsModule, TokebiAnalytics)