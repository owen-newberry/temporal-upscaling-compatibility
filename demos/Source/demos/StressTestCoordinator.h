// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StressTestCoordinator.generated.h"

class ATimestepActor;
class AMotionModelActor;
class AMotionAuthorityActor;
class AMoverActor;
class AWorkloadActor;

/**
 * Spawns all four upscaler-compatibility patterns side-by-side in a single
 * level so combined-load behaviour can be observed. Place one of these in an
 * empty map and press Play — the coordinator lays out four clusters around
 * its own transform on BeginPlay.
 *
 * Each pattern cluster uses its well-behaved variant by default so the scene
 * runs at a sensible frame rate. Toggle `bUnbudgetedWorkload` to stress-test
 * the workload pattern against an unbounded worker; other toggles disable
 * individual clusters for isolation runs.
 */
UCLASS()
class DEMOS_API AStressTestCoordinator : public AActor
{
	GENERATED_BODY()

public:
	AStressTestCoordinator();

	// ── Cluster toggles ─────────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Stress Test|Enable")
	bool bSpawnTimestepPair = true;

	UPROPERTY(EditAnywhere, Category = "Stress Test|Enable")
	bool bSpawnMotionModelPair = true;

	UPROPERTY(EditAnywhere, Category = "Stress Test|Enable")
	bool bSpawnAuthorityPair = true;

	UPROPERTY(EditAnywhere, Category = "Stress Test|Enable")
	bool bSpawnWorkloadPair = true;

	// ── Workload tuning ─────────────────────────────────────────────
	// False (default) = both workload actors budgeted, scene stays at >60fps.
	// True  = both actors unbudgeted, tanks FPS on purpose for the stress case.
	UPROPERTY(EditAnywhere, Category = "Stress Test|Workload")
	bool bUnbudgetedWorkload = false;

	// ── Layout ──────────────────────────────────────────────────────
	// Horizontal distance between pattern clusters (cm).
	UPROPERTY(EditAnywhere, Category = "Stress Test|Layout")
	float ClusterSpacingCm = 800.f;

	// Vertical separation between the two actors within a cluster (cm).
	UPROPERTY(EditAnywhere, Category = "Stress Test|Layout")
	float IntraClusterSpacingCm = 250.f;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnTimestepPair(const FVector& Origin);
	void SpawnMotionModelPair(const FVector& Origin);
	void SpawnAuthorityPair(const FVector& Origin);
	void SpawnWorkloadPair(const FVector& Origin);

	// Held so they don't get garbage-collected mid-session.
	UPROPERTY() TArray<AActor*> SpawnedActors;
};
