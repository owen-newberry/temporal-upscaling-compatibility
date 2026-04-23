// Fill out your copyright notice in the Description page of Project Settings.

#include "UpscalerControl.h"
#include "HAL/IConsoleManager.h"

namespace DemosUpscaler
{
	static IConsoleVariable* FindCVar(const TCHAR* Name)
	{
		return IConsoleManager::Get().FindConsoleVariable(Name);
	}

	FString ToString(EMethod Method)
	{
		switch (Method)
		{
			case EMethod::None: return TEXT("None");
			case EMethod::FXAA: return TEXT("FXAA");
			case EMethod::TAA:  return TEXT("TAA");
			case EMethod::TSR:  return TEXT("TSR");
			case EMethod::FSR3: return TEXT("FSR3");
			case EMethod::DLSS: return TEXT("DLSS");
			case EMethod::XeSS: return TEXT("XeSS");
			default:            return TEXT("Unknown");
		}
	}

	EMethod ParseName(const FString& Name)
	{
		const FString N = Name.TrimStartAndEnd().ToUpper();
		if (N == TEXT("OFF") || N == TEXT("NONE"))     return EMethod::None;
		if (N == TEXT("FXAA"))                         return EMethod::FXAA;
		if (N == TEXT("TAA") || N == TEXT("TAAU"))     return EMethod::TAA;
		if (N == TEXT("TSR"))                          return EMethod::TSR;
		if (N == TEXT("FSR") || N == TEXT("FSR3")
		 || N == TEXT("FIDELITYFX"))                   return EMethod::FSR3;
		if (N == TEXT("DLSS") || N == TEXT("NGX"))     return EMethod::DLSS;
		if (N == TEXT("XESS") || N == TEXT("INTEL"))   return EMethod::XeSS;
		return EMethod::Unknown;
	}

	// Returns true if the named CVar exists AND its integer value is >= 1.
	static bool CVarIsEnabled(const TCHAR* Name)
	{
		IConsoleVariable* CVar = FindCVar(Name);
		return CVar && CVar->GetInt() >= 1;
	}

	EMethod GetActive()
	{
		// Check plugin-specific enables first — they override the engine AA method.
		if (CVarIsEnabled(TEXT("r.FidelityFX.FSR3.Enabled")) ||
		    CVarIsEnabled(TEXT("r.FidelityFX.FI.Enabled")))
		{
			return EMethod::FSR3;
		}
		if (CVarIsEnabled(TEXT("r.NGX.DLSS.Enable")) ||
		    CVarIsEnabled(TEXT("r.NGX.DLSS.Quality")))
		{
			return EMethod::DLSS;
		}
		if (CVarIsEnabled(TEXT("r.XeSS.Enabled")))
		{
			return EMethod::XeSS;
		}

		// Fall back to the engine's AA method CVar.
		// 0=None, 1=FXAA, 2=TAA, 3=MSAA, 4=TSR.
		if (IConsoleVariable* AA = FindCVar(TEXT("r.AntiAliasingMethod")))
		{
			switch (AA->GetInt())
			{
				case 0: return EMethod::None;
				case 1: return EMethod::FXAA;
				case 2: return EMethod::TAA;
				case 4: return EMethod::TSR;
				default: break;
			}
		}
		return EMethod::Unknown;
	}

	FString GetActiveName()
	{
		return ToString(GetActive());
	}

	// Helper: disable every plugin upscaler so the engine's AA method takes over.
	static void DisableAllPlugins()
	{
		static const TCHAR* const PluginCVars[] = {
			TEXT("r.FidelityFX.FSR3.Enabled"),
			TEXT("r.FidelityFX.FI.Enabled"),
			TEXT("r.NGX.DLSS.Enable"),
			TEXT("r.XeSS.Enabled"),
		};
		for (const TCHAR* Name : PluginCVars)
		{
			if (IConsoleVariable* CVar = FindCVar(Name))
			{
				CVar->Set(0, ECVF_SetByConsole);
			}
		}
	}

	static bool SetAAMethod(int32 Method)
	{
		if (IConsoleVariable* AA = FindCVar(TEXT("r.AntiAliasingMethod")))
		{
			AA->Set(Method, ECVF_SetByConsole);
			return true;
		}
		return false;
	}

	bool SetUpscaler(EMethod Method)
	{
		switch (Method)
		{
			case EMethod::None:
				DisableAllPlugins();
				return SetAAMethod(0);

			case EMethod::FXAA:
				DisableAllPlugins();
				return SetAAMethod(1);

			case EMethod::TAA:
				DisableAllPlugins();
				return SetAAMethod(2);

			case EMethod::TSR:
				DisableAllPlugins();
				return SetAAMethod(4);

			case EMethod::FSR3:
			{
				IConsoleVariable* FSR = FindCVar(TEXT("r.FidelityFX.FSR3.Enabled"));
				if (!FSR)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("[DemosUpscaler] FSR3 plugin not loaded — install "
						     "FFXFSR3 from GPUOpen and enable it in demos.uproject."));
					return false;
				}
				// Disable other plugins so only FSR3 is active.
				if (IConsoleVariable* DLSS = FindCVar(TEXT("r.NGX.DLSS.Enable"))) DLSS->Set(0, ECVF_SetByConsole);
				if (IConsoleVariable* XeSS = FindCVar(TEXT("r.XeSS.Enabled")))    XeSS->Set(0, ECVF_SetByConsole);
				FSR->Set(1, ECVF_SetByConsole);
				// FSR3 still wants TSR as the engine fallback path.
				SetAAMethod(4);
				return true;
			}

			case EMethod::DLSS:
			{
				IConsoleVariable* DLSS = FindCVar(TEXT("r.NGX.DLSS.Enable"));
				if (!DLSS)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("[DemosUpscaler] DLSS plugin not loaded — install "
						     "DLSSUpscaler from NVIDIA and enable it in demos.uproject."));
					return false;
				}
				if (IConsoleVariable* FSR  = FindCVar(TEXT("r.FidelityFX.FSR3.Enabled"))) FSR->Set(0, ECVF_SetByConsole);
				if (IConsoleVariable* XeSS = FindCVar(TEXT("r.XeSS.Enabled")))            XeSS->Set(0, ECVF_SetByConsole);
				DLSS->Set(1, ECVF_SetByConsole);
				SetAAMethod(4);
				return true;
			}

			case EMethod::XeSS:
			{
				IConsoleVariable* XeSS = FindCVar(TEXT("r.XeSS.Enabled"));
				if (!XeSS)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("[DemosUpscaler] XeSS plugin not loaded."));
					return false;
				}
				if (IConsoleVariable* FSR  = FindCVar(TEXT("r.FidelityFX.FSR3.Enabled"))) FSR->Set(0, ECVF_SetByConsole);
				if (IConsoleVariable* DLSS = FindCVar(TEXT("r.NGX.DLSS.Enable")))         DLSS->Set(0, ECVF_SetByConsole);
				XeSS->Set(1, ECVF_SetByConsole);
				SetAAMethod(4);
				return true;
			}

			default:
				return false;
		}
	}
}

// ----------------------------------------------------------------------------
// Console command: demos.Upscaler <Name>
//   e.g. "demos.Upscaler FSR", "demos.Upscaler TSR", "demos.Upscaler DLSS"
//
// Output of active mode is logged both to LogTemp and to the console so the
// user can see whether the switch was applied (or skipped because a plugin
// isn't loaded). Captured by PerfLogger on the next sample tick.
// ----------------------------------------------------------------------------
static FAutoConsoleCommand GDemosUpscalerCmd(
	TEXT("demos.Upscaler"),
	TEXT("Switch the active temporal upscaler. Usage: demos.Upscaler <TSR|FSR|DLSS|TAA|FXAA|None>. "
	     "If the requested plugin isn't loaded, the switch is skipped and a warning is logged."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() == 0)
		{
			UE_LOG(LogTemp, Display,
				TEXT("[DemosUpscaler] Active upscaler: %s"),
				*DemosUpscaler::GetActiveName());
			return;
		}

		const DemosUpscaler::EMethod Method = DemosUpscaler::ParseName(Args[0]);
		if (Method == DemosUpscaler::EMethod::Unknown)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[DemosUpscaler] Unknown method '%s'. "
				     "Valid: TSR, FSR, DLSS, TAA, FXAA, None."),
				*Args[0]);
			return;
		}

		const bool bOK = DemosUpscaler::SetUpscaler(Method);
		UE_LOG(LogTemp, Display,
			TEXT("[DemosUpscaler] SetUpscaler(%s) -> %s. Active now: %s"),
			*DemosUpscaler::ToString(Method),
			bOK ? TEXT("applied") : TEXT("skipped (plugin missing)"),
			*DemosUpscaler::GetActiveName());
	})
);
