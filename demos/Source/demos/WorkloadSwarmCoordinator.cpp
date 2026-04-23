// Fill out your copyright notice in the Description page of Project Settings.

#include "WorkloadSwarmCoordinator.h"

#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "WorkloadActor.h"

AWorkloadSwarmCoordinator::AWorkloadSwarmCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWorkloadSwarmCoordinator::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector GridOrigin = GetActorLocation() + GridOriginOffsetCm;

	// Deterministic RNG seeded from the coordinator's mode so budgeted and
	// unbudgeted swarms get identical jitter patterns (any visible
	// difference is then purely the pattern's doing, not RNG noise).
	FRandomStream Rand(bBudgeted ? 0xBADC0DE : 0xFEEDFACE);

	SpawnedActors.Reset(GridColumns * GridRows);

	for (int32 Row = 0; Row < GridRows; ++Row)
	{
		for (int32 Col = 0; Col < GridColumns; ++Col)
		{
			// Centre the grid on its origin so the whole swarm sits
			// symmetrically around the coordinator's position.
			const float X = (Col - (GridColumns - 1) * 0.5f) * ColumnSpacingCm;
			const float Y = (Row - (GridRows - 1)    * 0.5f) * RowSpacingCm;
			const FVector SpawnPos = GridOrigin + FVector(X, Y, 0.f);

			AWorkloadActor* Actor = World->SpawnActor<AWorkloadActor>(
				AWorkloadActor::StaticClass(), SpawnPos,
				FRotator::ZeroRotator, Params);
			if (!Actor)
			{
				continue;
			}

			// Forward all per-actor tuning so the swarm behaves as one
			// coherent experimental condition.
			Actor->bBudgeted      = bBudgeted;
			Actor->TasksPerFrame  = TasksPerFrame;
			Actor->TaskDurationMs = TaskDurationMs;
			Actor->BudgetMs       = BudgetMs;
			Actor->Amplitude      = Amplitude;

			// Small per-actor frequency jitter so the grid decorrelates
			// visually. Deterministic — same seed across runs with the
			// same bBudgeted value (see FRandomStream above).
			const float Jitter = Rand.FRandRange(-FrequencyJitter, FrequencyJitter);
			Actor->Frequency = FMath::Max(0.05f, BaseFrequency + Jitter);

			SpawnedActors.Add(Actor);
		}
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[WorkloadSwarm] Spawned %d actors (%dx%d grid) | mode=%s | budget=%.1fms | "
		     "per-actor load ~%.1fms"),
		SpawnedActors.Num(),
		GridColumns, GridRows,
		bBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"),
		BudgetMs,
		TasksPerFrame * TaskDurationMs);
}
