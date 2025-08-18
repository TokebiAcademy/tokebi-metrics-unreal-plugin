#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IAnalyticsProvider.h"

class IAnalyticsProvider;

/**
 * Tokebi Analytics module implementation
 * Registers Tokebi as an analytics provider with Unreal Engine
 */
class FTokebiAnalyticsModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    /**
     * Creates a new Tokebi Analytics provider instance
     * @param Key Configuration key for this provider
     * @return Shared pointer to the analytics provider
     */
    virtual TSharedPtr<IAnalyticsProvider> CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const;

private:
    /** Analytics provider factory delegate */
    FAnalyticsProviderConfigurationDelegate ConfigDelegate;
    
    /** Handle for the provider factory */
    FDelegateHandle ProviderFactoryHandle;
};
