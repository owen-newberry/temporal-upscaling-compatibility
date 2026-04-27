// Fill out your copyright notice in the Description page of Project Settings.

#include "DemosDebug.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/CommandLine.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"

#if DEMOS_STANDALONE_DIAGNOSTICS
#include "EngineUtils.h"
#endif

#if DEMOS_STANDALONE_DIAGNOSTICS

void DemosLogStartupMapContext(UGameInstance* GameInstance)
{
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[DemosDebug] DemosLogStartupMapContext: GameInstance is null"));
		return;
	}

	UWorld* World = GameInstance->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[DemosDebug] No UWorld on GameInstance yet (timer too early or no map)."));
		return;
	}

	int32 TotalActors = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		++TotalActors;
	}

	const bool bPIE  = World->IsPlayInEditor();
	const bool bAuth = World->GetNetMode() < NM_Client;
	const FString WorldObj  = World->GetName();
	const FString MapStr  = World->GetMapName();
	const int32 WorldTypeInt = static_cast<int32>(World->WorldType);

	UE_LOG(LogTemp, Warning, TEXT("========== [DemosDebug] startup map context =========="));
	UE_LOG(LogTemp, Warning, TEXT("  GetMapName()     : %s"), *MapStr);
	UE_LOG(LogTemp, Warning, TEXT("  World object     : %s"), *WorldObj);
	UE_LOG(LogTemp, Warning, TEXT("  WorldType        : %d  (1=Game,4=PIE, etc.)"), WorldTypeInt);
	UE_LOG(LogTemp, Warning, TEXT("  IsPlayInEditor   : %s"), bPIE ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("  IsListenServer+  : %s"), bAuth ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("  TActorCount      : %d (approx. all actors)"), TotalActors);
	UE_LOG(LogTemp, Warning, TEXT("  Engine / UE      : %s"), *FEngineVersion::Current().ToString());
	UE_LOG(LogTemp, Warning, TEXT("  Project          : %s"), *FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()));
	{
		const FString Cmd(FCommandLine::Get());
		UE_LOG(LogTemp, Warning, TEXT("  Command line     : %s"),
			*Cmd.Left(500));
	}
	UE_LOG(LogTemp, Warning, TEXT("========== (search Output Log for [DemosDebug]) =========="));

	// On-screen: useful when running -game and looking at the viewport (not the log).
	if (bAuth && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			100, 45.f, FColor::Cyan,
			FString::Printf(TEXT("[DemosDebug] Map: %s"), *MapStr));
		GEngine->AddOnScreenDebugMessage(
			101, 45.f, FColor::Cyan,
			FString::Printf(TEXT("[DemosDebug] PIE=%s  WorldType=%d  Actors~=%d"), bPIE ? TEXT("yes") : TEXT("no"), WorldTypeInt, TotalActors));
	}
}

void DemosLogCoordinatorBeginPlay(AActor* Coordinator, int32 TotalSpawned,
	bool bSpawnedWorkload, bool bUnbudgetedWorkload)
{
	if (!Coordinator) return;
	UWorld* W = Coordinator->GetWorld();
	UE_LOG(LogTemp, Warning, TEXT("[DemosDebug] StressTestCoordinator %s in map %s | TotalSpawned=%d bSpawnWorkload=%s mode=%s"),
		*Coordinator->GetName(), W ? *W->GetMapName() : TEXT("?"),
		TotalSpawned,
		bSpawnedWorkload ? TEXT("true") : TEXT("false"),
		bUnbudgetedWorkload ? TEXT("Unbudgeted") : TEXT("Budgeted"));
}

void DemosLogWorkloadPairSpawn(AActor* Coordinator, int32 Succeeded, int32 Failed, bool bModeBudgeted)
{
	UE_LOG(LogTemp, Warning, TEXT("[DemosDebug] Workload pair spawn: ok=%d fail=%d mode=%s (coordinator %s)"),
		Succeeded, Failed, bModeBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"), Coordinator ? *Coordinator->GetName() : TEXT("null"));
	if (Failed > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[DemosDebug] WorkloadActor SpawnActor failed %d time(s) — class/cooking/world?"), Failed);
	}
}

void DemosLogSwarmNullWorld(AActor* Coordinator)
{
	UE_LOG(LogTemp, Error, TEXT("[DemosDebug] WorkloadSwarmCoordinator %s: GetWorld() is null in BeginPlay"), Coordinator ? *Coordinator->GetName() : TEXT("?"));
}

void DemosLogSwarmDone(AActor* Coordinator, int32 N, const TCHAR* ModeStr)
{
	UE_LOG(LogTemp, Warning, TEXT("[DemosDebug] WorkloadSwarm %s: spawned %d actors | %s"), Coordinator ? *Coordinator->GetName() : TEXT("?"), N, ModeStr);
}

#endif
