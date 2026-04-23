// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorkloadSwarmCoordinator.generated.h"

class AWorkloadActor;

/**
 * Spawns a grid of AWorkloadActor instances, all configured to the same
 * mode, to stand in for "N independent game subsystems all doing CPU
 * work every frame" — the scenario where workload budgeting actually
 * proves its value.
 *
 * Usage for the live demo:
 *   1. Place TWO of these in the level.
 *   2. Left one: bBudgeted = true,  GridOriginOffsetCm = (0, -500, 0).
 *   3. Right one: bBudgeted = false, GridOriginOffsetCm = (0,  500, 0).
 *   4. Hit Play.
 *
 * You'll see two grids of cubes: budgeted side stays smooth, unbudgeted
 * side visibly hitches and drags the whole game's frame rate down —
 * because each cube is adding its own un-budgeted chunk of work to the
 * frame every tick. With 8 actors on each side the unbudgeted total
 * balloons from 2ms to ~16-40ms per frame and FPS falls off a cliff.
 *
 * Any per-capture logging happens inside each spawned WorkloadActor
 * (MotionLogger row-per-actor), so the existing analysis pipeline works
 * unchanged — you just get more rows.
 */
UCLASS()
class DEMOS_API AWorkloadSwarmCoordinator : public AActor
{
	GENERATED_BODY()

public:
	AWorkloadSwarmCoordinator();

	// ── Mode ────────────────────────────────────────────────────────
	// All spawned actors share this mode so the coordinator's colour is
	// meaningful — one swarm = one experimental condition.
	UPROPERTY(EditAnywhere, Category = "Workload Swarm")
	bool bBudgeted = true;

	// ── Grid layout ─────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Grid", meta = (ClampMin = "1", ClampMax = "64"))
	int32 GridColumns = 4;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Grid", meta = (ClampMin = "1", ClampMax = "64"))
	int32 GridRows = 2;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Grid")
	float ColumnSpacingCm = 220.f;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Grid")
	float RowSpacingCm = 220.f;

	// Extra offset applied to the whole grid (useful for placing two
	// swarms on either side of a camera without overlapping).
	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Grid")
	FVector GridOriginOffsetCm = FVector::ZeroVector;

	// ── Per-actor tuning ────────────────────────────────────────────
	// These are forwarded to every spawned AWorkloadActor verbatim. The
	// defaults are chosen so that (Budget=1.5ms, Tasks×Duration=3ms per
	// actor per frame) creates a visible-but-not-crushing load for 8
	// actors budgeted (<12ms total, ~85fps) versus all-at-once unbudgeted
	// (8 × 3ms = 24ms, ~40fps).
	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Per-Actor")
	int32 TasksPerFrame = 30;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Per-Actor")
	float TaskDurationMs = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Per-Actor")
	float BudgetMs = 1.5f;

	// ── Motion tuning ───────────────────────────────────────────────
	// Per-actor amplitude and frequency. Small random jitter is applied
	// to each spawned cube's frequency so the grid doesn't move in lock-
	// step — gives the visual a "many independent agents" feel rather
	// than "a wave of identical things."
	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Motion")
	float Amplitude = 120.f;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Motion")
	float BaseFrequency = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Workload Swarm|Motion", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float FrequencyJitter = 0.15f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<AWorkloadActor*> SpawnedActors;
};
