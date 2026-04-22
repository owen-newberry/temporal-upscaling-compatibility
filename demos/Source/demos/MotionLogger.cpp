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

	const FString Header = TEXT("Frame,TimeSeconds,ActorName,Mode,PosX,PosY,PosZ,FrameDeltaCm\n");
	FFileHelper::SaveStringToFile(Header, *FilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_EvenIfReadOnly);

	Buffer.Reset();
	bInitialized = true;

	UE_LOG(LogTemp, Warning, TEXT("[MotionLogger] Logging to: %s"), *FilePath);
}

void FMotionLogger::LogRow(int32 Frame, float TimeSeconds, const FString& ActorName,
                           const FString& Mode, FVector Position, float FrameDeltaCm)
{
	if (!bInitialized) return;

	Buffer.Add(FString::Printf(TEXT("%d,%.4f,%s,%s,%.2f,%.2f,%.2f,%.4f\n"),
		Frame, TimeSeconds, *ActorName, *Mode,
		Position.X, Position.Y, Position.Z, FrameDeltaCm));

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
