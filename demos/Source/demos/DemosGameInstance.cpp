// Fill out your copyright notice in the Description page of Project Settings.

#include "DemosGameInstance.h"
#include "MotionLogger.h"

void UDemosGameInstance::Init()
{
	Super::Init();

	// Reset the logger so each PIE session / standalone run gets a fresh CSV file.
	// Without this the singleton stays initialized from the previous session and
	// all subsequent sessions either append to the old file or log nothing.
	FMotionLogger::Get().Reset();
	FMotionLogger::Get().Init();

	UE_LOG(LogTemp, Warning, TEXT("[DemosGameInstance] Session started — logger initialized."));
}

void UDemosGameInstance::Shutdown()
{
	// Flush any remaining buffered rows before the session ends.
	FMotionLogger::Get().Flush();
	FMotionLogger::Get().Reset();

	UE_LOG(LogTemp, Warning, TEXT("[DemosGameInstance] Session ended — logger flushed."));

	Super::Shutdown();
}
