// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimestepActor.generated.h"

/**
 * Demo 2 — Fixed vs Variable Timestep
 *
 * Both modes simulate a damped spring oscillator displaced from spawn.
 *
 * Variable mode (red):  integrates with raw DeltaTime — Euler integration blows up
 *                       when a large frame spike is injected (displacement diverges).
 * Fixed mode  (green):  integrates in 1/FixedHz sub-steps via an accumulator —
 *                       stable regardless of spike magnitude.
 *
 * Enable bSimulateSpikes to periodically inject a large DeltaTime hitch.
 * The variable actor will visibly explode; the fixed actor stays bounded.
 */
UCLASS()
class DEMOS_API ATimestepActor : public AActor
{
	GENERATED_BODY()

public:
	ATimestepActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	// True = fixed accumulator (stable). False = raw DeltaTime (unstable under spikes).
	UPROPERTY(EditAnywhere, Category = "Timestep")
	bool bFixedTimestep = true;

	UPROPERTY(EditAnywhere, Category = "Timestep", meta = (EditCondition = "bFixedTimestep"))
	float FixedHz = 60.f;

	// Spring stiffness — higher values make instability appear sooner
	UPROPERTY(EditAnywhere, Category = "Spring")
	float SpringK = 8.f;

	UPROPERTY(EditAnywhere, Category = "Spring")
	float Damping = 0.4f;

	// Initial displacement from spawn position (cm)
	UPROPERTY(EditAnywhere, Category = "Spring")
	float InitialDisplacement = 250.f;

	// Periodically inject a large DeltaTime hitch to trigger instability
	UPROPERTY(EditAnywhere, Category = "Timestep")
	bool bSimulateSpikes = true;

	UPROPERTY(EditAnywhere, Category = "Timestep", meta = (EditCondition = "bSimulateSpikes"))
	float SpikeInterval = 5.f;

	// Simulated hitch duration in seconds (e.g. 0.25 = quarter-second freeze)
	UPROPERTY(EditAnywhere, Category = "Timestep", meta = (EditCondition = "bSimulateSpikes"))
	float SpikeDeltaTime = 0.25f;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void StepSpring(float Dt);

	FVector SpawnLocation;
	float   Displacement = 0.f;  // cm along X from spawn
	float   Velocity     = 0.f;  // cm/s
	float   Accumulator  = 0.f;
	float   SpikeTimer   = 0.f;

	UMaterialInstanceDynamic* DynMaterial = nullptr;
};
