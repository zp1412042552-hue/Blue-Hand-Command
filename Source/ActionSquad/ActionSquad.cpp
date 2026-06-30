#include "ActionSquad.h"

#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogActionSquad);

namespace ActionSquad
{
	void ApplyLowScalabilitySettings()
	{
		static const TCHAR* IntCvars[] =
		{
			TEXT("sg.ViewDistanceQuality"),
			TEXT("sg.AntiAliasingQuality"),
			TEXT("sg.PostProcessQuality"),
			TEXT("sg.ShadowQuality"),
			TEXT("sg.GlobalIlluminationQuality"),
			TEXT("sg.ReflectionQuality"),
			TEXT("sg.TextureQuality"),
			TEXT("sg.EffectsQuality"),
			TEXT("sg.FoliageQuality"),
			TEXT("sg.ShadingQuality"),
			TEXT("sg.LandscapeQuality"),
			TEXT("r.MaterialQualityLevel")
		};

		for (const TCHAR* CvarName : IntCvars)
		{
			if (IConsoleVariable* Cvar = IConsoleManager::Get().FindConsoleVariable(CvarName))
			{
				Cvar->Set(0, ECVF_SetByCode);
			}
		}

		if (IConsoleVariable* ScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")))
		{
			ScreenPercentage->Set(50.0f, ECVF_SetByCode);
		}
	}
}

class FActionSquadModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		ActionSquad::ApplyLowScalabilitySettings();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FActionSquadModule, ActionSquad, "ActionSquad");
