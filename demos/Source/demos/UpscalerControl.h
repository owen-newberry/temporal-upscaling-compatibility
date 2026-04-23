// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Thin runtime wrapper for detecting and switching the active temporal
 * upscaler. Purposefully plugin-agnostic: the code compiles and runs
 * whether or not AMD FSR / NVIDIA DLSS / Intel XeSS plugins are
 * installed. If a given plugin isn't loaded, its CVars simply don't
 * exist and the switch becomes a no-op (which the CSV will capture).
 *
 * Detection strategy:
 *   - Query the well-known public CVars each upscaler exposes.
 *   - If FSR3 plugin's `r.FidelityFX.FSR3.Enabled` == 1, call it FSR3.
 *   - Else if DLSS plugin's `r.NGX.DLSS.Enable` == 1, call it DLSS.
 *   - Else read `r.AntiAliasingMethod` and map to TSR / TAA / FXAA / None.
 *
 * Switching strategy:
 *   - SetUpscaler(...) sets the relevant CVars via IConsoleManager. If
 *     the target upscaler's CVars don't exist, the call logs a warning
 *     and reverts to TSR so the engine keeps rendering.
 *
 * This is called from a FAutoConsoleCommand ("demos.Upscaler TSR|FSR|DLSS|TAA")
 * and from FPerfLogger once per sample so every capture row records the
 * active upscaler. That's what lets the analysis pipeline compare
 * demos across upscaler techs without manually tagging sessions.
 */
namespace DemosUpscaler
{
	enum class EMethod : uint8
	{
		Unknown,
		None,
		FXAA,
		TAA,
		TSR,
		FSR3,
		DLSS,
		XeSS,
	};

	/** Short human-readable name for a method (used in CSV + UI). */
	DEMOS_API FString ToString(EMethod Method);

	/** Current active upscaler, derived from live CVars. Safe to call every frame. */
	DEMOS_API EMethod GetActive();

	/** Short string of the current active upscaler — shorthand for ToString(GetActive()). */
	DEMOS_API FString GetActiveName();

	/**
	 * Try to set the engine to use the given method. Returns true if the
	 * required CVars were found and changed; false if the plugin isn't
	 * loaded (in which case the engine continues with whatever was
	 * previously active).
	 */
	DEMOS_API bool SetUpscaler(EMethod Method);

	/** Parse a user-typed string like "fsr" / "tsr" / "dlss" / "off" to EMethod. */
	DEMOS_API EMethod ParseName(const FString& Name);
}
