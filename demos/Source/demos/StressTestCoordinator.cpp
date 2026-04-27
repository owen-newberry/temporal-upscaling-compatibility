// Fill out your copyright notice in the Description page of Project Settings.

#include "StressTestCoordinator.h"

#include "DemosDebug.h"
#include "Engine/World.h"
#include "TimestepActor.h"
#include "MotionModelActor.h"
#include "MotionAuthorityActor.h"
#include "MoverActor.h"
#include "WorkloadActor.h"

AStressTestCoordinator::AStressTestCoordinator()
{
	// No tick needed — coordinator is a one-shot spawner.
	PrimaryActorTick.bCanEverTick = false;
}

void AStressTestCoordinator::BeginPlay()
{
	Super::BeginPlay();

	const FVector Root = GetActorLocation();

	// Lay clusters out on a 2×2 grid around the coordinator so each cluster's
	// cube pair is visible from a typical overview camera position.
	if (bSpawnTimestepPair)
	{
		SpawnTimestepPair(Root + FVector( ClusterSpacingCm, -ClusterSpacingCm, 0.f));
	}
	if (bSpawnMotionModelPair)
	{
		SpawnMotionModelPair(Root + FVector( ClusterSpacingCm,  ClusterSpacingCm, 0.f));
	}
	if (bSpawnAuthorityPair)
	{
		SpawnAuthorityPair(Root + FVector(-ClusterSpacingCm, -ClusterSpacingCm, 0.f));
	}
	if (bSpawnWorkloadPair)
	{
		SpawnWorkloadPair(Root + FVector(-ClusterSpacingCm,  ClusterSpacingCm, 0.f));
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[StressTest] Spawned %d actors | workload=%s"),
		SpawnedActors.Num(),
		bUnbudgetedWorkload ? TEXT("Unbudgeted") : TEXT("Budgeted"));
	DemosLogCoordinatorBeginPlay(this, SpawnedActors.Num(), bSpawnWorkloadPair, bUnbudgetedWorkload);
}

void AStressTestCoordinator::SpawnTimestepPair(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Variable (red) — first, no phase offset. Will diverge under spikes.
	if (ATimestepActor* Var = World->SpawnActor<ATimestepActor>(
			ATimestepActor::StaticClass(),
			Origin + FVector(0.f, -IntraClusterSpacingCm * 0.5f, 0.f),
			FRotator::ZeroRotator, Params))
	{
		Var->bFixedTimestep   = false;
		Var->bSimulateSpikes  = true;
		Var->SpikePhaseOffset = 0.f;
		SpawnedActors.Add(Var);
	}

	// Fixed (green) — half-interval phase offset so spikes don't fire together.
	if (ATimestepActor* Fix = World->SpawnActor<ATimestepActor>(
			ATimestepActor::StaticClass(),
			Origin + FVector(0.f, IntraClusterSpacingCm * 0.5f, 0.f),
			FRotator::ZeroRotator, Params))
	{
		Fix->bFixedTimestep   = true;
		Fix->bSimulateSpikes  = true;
		Fix->SpikePhaseOffset = 2.5f;
		SpawnedActors.Add(Fix);
	}
}

void AStressTestCoordinator::SpawnMotionModelPair(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Frame-based (red) — PI/4 initial phase so divergence is visible at t=0.
	if (AMotionModelActor* Frame = World->SpawnActor<AMotionModelActor>(
			AMotionModelActor::StaticClass(),
			Origin + FVector(0.f, -IntraClusterSpacingCm * 0.5f, 0.f),
			FRotator::ZeroRotator, Params))
	{
		Frame->bTimeBased        = false;
		Frame->bSimulateSpikes   = true;
		Frame->InitialFramePhase = 0.785f; // ~PI/4
		SpawnedActors.Add(Frame);
	}

	// Time-based (green) — frame-rate independent reference.
	if (AMotionModelActor* Time = World->SpawnActor<AMotionModelActor>(
			AMotionModelActor::StaticClass(),
			Origin + FVector(0.f, IntraClusterSpacingCm * 0.5f, 0.f),
			FRotator::ZeroRotator, Params))
	{
		Time->bTimeBased      = true;
		Time->bSimulateSpikes = true;
		SpawnedActors.Add(Time);
	}
}

void AStressTestCoordinator::SpawnAuthorityPair(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Central motion authority — no mesh, lives at cluster origin.
	AMotionAuthorityActor* Authority = World->SpawnActor<AMotionAuthorityActor>(
		AMotionAuthorityActor::StaticClass(), Origin,
		FRotator::ZeroRotator, Params);
	if (!Authority) return;

	Authority->bAuthorityMode = true;
	SpawnedActors.Add(Authority);

	// Two mover clients — one Direct (naive), one Authority (good citizen),
	// mirroring the isolated L_MotionAuthority scene so the combined-stress
	// capture produces a genuine Demo 1 comparison. The Direct mover keeps
	// its phantom writer enabled → contaminated motion. The Authority mover
	// submits only the primary input → authority-arbitrated motion.
	for (int32 i = 0; i < 2; ++i)
	{
		const float YOffset = (i == 0 ? -1.f : 1.f) * IntraClusterSpacingCm * 0.5f;
		if (AMoverActor* Mover = World->SpawnActor<AMoverActor>(
				AMoverActor::StaticClass(),
				Origin + FVector(0.f, YOffset, 0.f),
				FRotator::ZeroRotator, Params))
		{
			// i == 0 → Direct mode (no authority manager wired).
			// i == 1 → Authority mode (routes writes through the arbitrator).
			Mover->AuthorityManager = (i == 0) ? nullptr : Authority;
			Mover->Frequency        = (i == 0) ? 0.5f : 0.7f; // slight mismatch
			SpawnedActors.Add(Mover);
		}
	}
}

void AStressTestCoordinator::SpawnWorkloadPair(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Both actors get the same mode — flip the coordinator's toggle to switch
	// the scene between "all budgeted" (good) and "all unbudgeted" (stress).
	const bool bBudgeted = !bUnbudgetedWorkload;
	int32 WorkOk = 0;
	int32 WorkFail = 0;

	for (int32 i = 0; i < 2; ++i)
	{
		const float YOffset = (i == 0 ? -1.f : 1.f) * IntraClusterSpacingCm * 0.5f;
		if (AWorkloadActor* Work = World->SpawnActor<AWorkloadActor>(
				AWorkloadActor::StaticClass(),
				Origin + FVector(0.f, YOffset, 0.f),
				FRotator::ZeroRotator, Params))
		{
			Work->bBudgeted = bBudgeted;
			SpawnedActors.Add(Work);
			++WorkOk;
		}
		else
		{
			++WorkFail;
		}
	}
	DemosLogWorkloadPairSpawn(this, WorkOk, WorkFail, bBudgeted);
}
