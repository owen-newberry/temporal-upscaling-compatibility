// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Lightweight CSV logger for per-frame motion data.
 * Call Init() once at session start, LogRow() each frame, Flush() at session end.
 * Output: <ProjectSaved>/Logs/MotionCapture_<timestamp>.csv
 *
 * Columns: Frame, TimeSeconds, LevelName, ActorName, Mode, PosX, PosY, PosZ, FrameDeltaCm
 */
class DEMOS_API FMotionLogger
{
public:
	static FMotionLogger& Get();

	// Opens the CSV file and writes the header row.
	// Called automatically by DemosGameInstance::Init().
	void Init();

	// Resets the logger so the next Init() creates a new file.
	// Called by DemosGameInstance at session start and end.
	void Reset();

	// Appends one row. LevelName comes from GetWorld()->GetMapName().
	// PositionErrorCm: optional ground-truth error (Demo 3 only). Pass -1 to omit.
	void LogRow(int32 Frame, float TimeSeconds, const FString& LevelName,
	            const FString& ActorName, const FString& Mode,
	            FVector Position, float FrameDeltaCm, float PositionErrorCm = -1.f);

	// Flushes remaining buffer to disk.
	void Flush();

	bool IsInitialized() const { return bInitialized; }

private:
	FMotionLogger() = default;

	FString FilePath;
	TArray<FString> Buffer;
	bool bInitialized = false;
	int32 FlushInterval = 60; // write to disk every N rows
};
