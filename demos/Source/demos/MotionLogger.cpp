// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionLogger.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

FMotionLogger& FMotionLogger::Get()
{
	static FMotionLogger Instance;
	return Instance;
}

void FMotionLogger::Init()
{
	if (bInitialized) return;

	// Build a timestamped filename so each Play session gets its own file
	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString LogDir    = FPaths::ProjectSavedDir() / TEXT("Logs");
	FilePath = LogDir / FString::Printf(TEXT("MotionCapture_%s.csv"), *Timestamp);

	IFileManager::Get().MakeDirectory(*LogDir, true);

	const FString Header = TEXT("Frame,TimeSeconds,LevelName,ActorName,Mode,PosX,PosY,PosZ,FrameDeltaCm,PositionErrorCm,FrameWorkMs\n");
	FFileHelper::SaveStringToFile(Header, *FilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_EvenIfReadOnly);

	Buffer.Reset();
	bInitialized = true;

	UE_LOG(LogTemp, Warning, TEXT("[MotionLogger] Logging to: %s"), *FilePath);
}

void FMotionLogger::Reset()
{
	if (bInitialized)
	{
		Flush();
	}
	bInitialized = false;
	FilePath     = TEXT("");
	Buffer.Reset();
}

void FMotionLogger::LogRow(int32 Frame, float TimeSeconds, const FString& LevelName,
                           const FString& ActorName, const FString& Mode,
                           FVector Position, float FrameDeltaCm,
                           float PositionErrorCm, float FrameWorkMs)
{
	if (!bInitialized) return;

	// Strip PIE prefix (e.g. "UEDPIE_0_L_MotionAuthority" → "L_MotionAuthority")
	FString CleanLevel = LevelName;
	if (CleanLevel.StartsWith(TEXT("UEDPIE_")))
	{
		int32 LastUnder;
		if (CleanLevel.FindLastChar('_', LastUnder))
			CleanLevel = CleanLevel.RightChop(LastUnder + 1);
	}

	// Sanitize actor name — commas would corrupt the CSV
	FString SafeActor = ActorName;
	SafeActor.ReplaceInline(TEXT(","), TEXT(";"));

	// Optional per-demo columns — write empty strings when unused so the
	// schema is stable across demos without forcing every caller to fill in
	// values it has no meaning for.
	const FString ErrorStr = (PositionErrorCm >= 0.f)
		? FString::Printf(TEXT("%.4f"), PositionErrorCm)
		: FString();
	const FString WorkStr = (FrameWorkMs >= 0.f)
		? FString::Printf(TEXT("%.4f"), FrameWorkMs)
		: FString();

	Buffer.Add(FString::Printf(TEXT("%d,%.4f,%s,%s,%s,%.2f,%.2f,%.2f,%.4f,%s,%s\n"),
		Frame, TimeSeconds, *CleanLevel, *SafeActor, *Mode,
		Position.X, Position.Y, Position.Z, FrameDeltaCm, *ErrorStr, *WorkStr));

	if (Buffer.Num() >= FlushInterval)
	{
		Flush();
	}
}

void FMotionLogger::Flush()
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
