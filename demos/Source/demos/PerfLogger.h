// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Lightweight CSV logger for system-level performance metrics.
 *
 * Samples frame pacing (FApp::GetDeltaTime), GPU frame cycles (RHIGetGPUFrameCycles),
 * and process memory (FPlatformMemory::GetStats) at a fixed interval driven by
 * DemosGameInstance. Covers F4.2 (CPU/GPU) and F4.3 (memory) from the README
 * without depending on engine-internal globals that aren't stable across UE
 * releases.
 *
 * Output: <ProjectSaved>/Logs/PerformanceCapture_<timestamp>.csv
 *
 * Columns:
 *   TimeSeconds, LevelName, Upscaler,
 *   FrameTimeMs, GPUFrameMs,
 *   UsedPhysicalMB, PeakPhysicalMB, UsedVirtualMB
 *
 * The Upscaler column captures the active temporal upscaler (TSR, FSR3,
 * DLSS, TAA, ...) via DemosUpscaler::GetActiveName(). This lets the
 * analysis pipeline compare identical demos across upscaler technologies
 * without manually tagging sessions — the CSV is self-describing.
 */
class DEMOS_API FPerfLogger
{
public:
	static FPerfLogger& Get();

	void Init();
	void Reset();
	void Flush();

	// Samples frame-time + GPU cycles + memory and writes one row. Safe to
	// call from the core ticker; the values it reads are all public engine
	// APIs updated every frame.
	void SampleOnce(class UWorld* World);

	bool IsInitialized() const { return bInitialized; }

private:
	FPerfLogger() = default;

	FString FilePath;
	TArray<FString> Buffer;
	bool bInitialized = false;
	int32 FlushInterval = 30;
};
