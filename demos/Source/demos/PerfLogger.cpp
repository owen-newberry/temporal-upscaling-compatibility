// Fill out your copyright notice in the Description page of Project Settings.

#include "PerfLogger.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "UpscalerControl.h"

FPerfLogger& FPerfLogger::Get()
{
	static FPerfLogger Instance;
	return Instance;
}

void FPerfLogger::Init()
{
	if (bInitialized) return;

	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString LogDir    = FPaths::ProjectSavedDir() / TEXT("Logs");
	FilePath = LogDir / FString::Printf(TEXT("PerformanceCapture_%s.csv"), *Timestamp);

	IFileManager::Get().MakeDirectory(*LogDir, true);

	const FString Header =
		TEXT("TimeSeconds,LevelName,Upscaler,FrameTimeMs,GPUFrameMs,")
		TEXT("UsedPhysicalMB,PeakPhysicalMB,UsedVirtualMB\n");

	FFileHelper::SaveStringToFile(Header, *FilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_EvenIfReadOnly);

	Buffer.Reset();
	bInitialized = true;

	UE_LOG(LogTemp, Warning, TEXT("[PerfLogger] Logging to: %s"), *FilePath);
}

void FPerfLogger::Reset()
{
	if (bInitialized)
	{
		Flush();
	}
	bInitialized = false;
	FilePath     = TEXT("");
	Buffer.Reset();
}

static FString SanitizeLevelName(const FString& Raw)
{
	FString Clean = Raw;
	if (Clean.StartsWith(TEXT("UEDPIE_")))
	{
		int32 LastUnder;
		if (Clean.FindLastChar('_', LastUnder))
			Clean = Clean.RightChop(LastUnder + 1);
	}
	return Clean;
}

void FPerfLogger::SampleOnce(UWorld* World)
{
	if (!bInitialized) return;

	// FApp::GetDeltaTime is the last frame's total wall-clock duration in
	// seconds — the single number the paper really needs for frame-pacing
	// stability (1% / 0.1% lows derive from it).
	const double FrameTimeMs = FApp::GetDeltaTime() * 1000.0;

	// Public RHI accessor for GPU frame cost. Replaces the deprecated
	// GGPUFrameTime global; returns cycles measured on the GPU timing pass.
	const double GpuFrameMs = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());

	const FPlatformMemoryStats Mem = FPlatformMemory::GetStats();
	const double ToMB = 1.0 / (1024.0 * 1024.0);
	const double UsedPhysMB = Mem.UsedPhysical     * ToMB;
	const double PeakPhysMB = Mem.PeakUsedPhysical * ToMB;
	const double UsedVirtMB = Mem.UsedVirtual      * ToMB;

	const float   TimeSeconds = World ? World->GetTimeSeconds() : 0.f;
	const FString LevelName   = World
		? SanitizeLevelName(World->GetMapName())
		: FString(TEXT(""));
	const FString UpscalerName = DemosUpscaler::GetActiveName();

	Buffer.Add(FString::Printf(
		TEXT("%.4f,%s,%s,%.3f,%.3f,%.2f,%.2f,%.2f\n"),
		TimeSeconds, *LevelName, *UpscalerName,
		FrameTimeMs, GpuFrameMs,
		UsedPhysMB, PeakPhysMB, UsedVirtMB));

	if (Buffer.Num() >= FlushInterval)
	{
		Flush();
	}
}

void FPerfLogger::Flush()
{
	if (!bInitialized || Buffer.Num() == 0) return;

	FString Combined;
	Combined.Reserve(Buffer.Num() * 80);
	for (const FString& Row : Buffer)
	{
		Combined += Row;
	}
	Buffer.Reset();

	FFileHelper::SaveStringToFile(Combined, *FilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_Append | FILEWRITE_EvenIfReadOnly);
}
