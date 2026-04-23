// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "WorkloadActor.generated.h"

class UTextRenderComponent;
class UPointLightComponent;

/**
 * Demo 4 — Workload Budgeting (Task Queue per Frame)
 *
 * Each frame a queue of CPU tasks is generated. Each task does a fixed amount of
 * floating-point work to simulate real game logic (AI, physics, etc.).
 *
 * Unbudgeted (red):  ALL tasks are flushed every frame.
 *                    When the queue is large, the frame overruns → big DeltaTime spike
 *                    next frame → position jump visible on the moving cube.
 *
 * Budgeted   (green): Tasks are processed up to BudgetMs per frame; the rest are
 *                     deferred to the next frame. Frame time stays bounded,
 *                     DeltaTime stays smooth, and the cube moves cleanly.
 *
 * The actor also oscillates along X so motion artifacts from frame spikes are visible.
 */
UCLASS()
class DEMOS_API AWorkloadActor : public AActor
{
	GENERATED_BODY()

public:
	AWorkloadActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	// True = respects budget (good). False = flushes entire queue (bad).
	UPROPERTY(EditAnywhere, Category = "Workload")
	bool bBudgeted = true;

	// How many tasks to add to the queue each frame
	UPROPERTY(EditAnywhere, Category = "Workload")
	int32 TasksPerFrame = 80;

	// Target wall-clock duration of a single task in milliseconds.
	// Uses a wall-time busy loop so the cost is reliable across hardware
	// (unlike pure iteration counts, which vary with CPU speed).
	// Default 0.1ms × 80 tasks = ~8ms of work per frame unbudgeted,
	// well above the 2ms budget so deferred counts stay meaningful.
	UPROPERTY(EditAnywhere, Category = "Workload")
	float TaskDurationMs = 0.1f;

	// Maximum milliseconds to spend on tasks per frame (budgeted mode only)
	UPROPERTY(EditAnywhere, Category = "Workload", meta = (EditCondition = "bBudgeted"))
	float BudgetMs = 2.f;

	UPROPERTY(EditAnywhere, Category = "Motion")
	float Amplitude = 200.f;

	UPROPERTY(EditAnywhere, Category = "Motion")
	float Frequency = 0.5f;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void ProcessTask();

	FVector SpawnLocation;
	float   TimeElapsed     = 0.f;
	int32   TasksDeferred   = 0;
	int32   TotalDeferred   = 0;
	int32   TotalProcessed  = 0;

	TArray<int32> TaskQueue;   // each entry = number of iterations for that task

	UPROPERTY() UTextRenderComponent* Label = nullptr;
	UPROPERTY() UPointLightComponent* Light = nullptr;
	TArray<FVector> TrailPoints;

	UMaterialInstanceDynamic* DynMaterial = nullptr;

	// Tracks what mode the visuals currently reflect so Tick can detect a
	// mismatch and refresh them. Needed because coordinators often set
	// bBudgeted *after* BeginPlay has already run with the default value.
	bool bLastVisualBudgeted = true;
};
