// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Lightweight CSV logger for per-frame motion data.
 * Call Init() once at session start, LogRow() each frame, Flush() at session end.
 * Output: <ProjectSaved>/Logs/MotionCapture_<timestamp>.csv
 *
 * Columns: Frame, TimeSeconds, ActorName, Mode, PosX, PosY, PosZ, FrameDeltaCm
 */
class DEMOS_API FMotionLogger
{
public:
	static FMotionLogger& Get();

	// Opens the CSV file and writes the header row. Safe to call multiple times.
	void Init();

	// Appends one row. Mode should be "Authority" or "Direct".
	void LogRow(int32 Frame, float TimeSeconds, const FString& ActorName,
	            const FString& Mode, FVector Position, float FrameDeltaCm);

	// Flushes remaining buffer and closes the file handle.
	void Flush();

	bool IsInitialized() const { return bInitialized; }

private:
	FMotionLogger() = default;

	FString FilePath;
	TArray<FString> Buffer;
	bool bInitialized = false;
	int32 FlushInterval = 60; // write to disk every N rows
};
