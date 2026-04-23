// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimestepActor.generated.h"

class UTextRenderComponent;
class UPointLightComponent;

/**
 * Demo 2 — Fixed vs Variable Timestep
 *
 * Both modes simulate a damped spring oscillator displaced from spawn.
 *
 * Variable mode (red):  integrates with raw DeltaTime — Euler integration blows up
 *                       when a large frame spike is injected (displacement diverges).
 * Fixed mode  (green):  integrates in 1/FixedHz sub-steps via an accumulator,
 *                       with a MaxCatchUpSeconds clamp so large hitches don't
 *                       cause the simulation to race forward many sub-steps in
 *                       one rendered frame. The spring oscillates at its
 *                       natural rate across spikes — per-frame position
 *                       deltas stay bounded, which is what temporal
 *                       upscalers need for stable history reuse.
 *
 * Enable bSimulateSpikes to periodically inject a large DeltaTime hitch.
 * The variable actor will visibly explode; the fixed actor stays smooth.
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

	// Upper bound on how much wall-clock time a single rendered frame can
	// feed into the sub-step accumulator. Prevents a long frame (hitch,
	// breakpoint, GC pause, focus loss) from being "recovered" by advancing
	// simulation many steps in one visible frame — which would look like the
	// spring racing forward. With this clamp, time spent inside a hitch is
	// effectively discarded: the spring resumes oscillating at its natural
	// rate instead of catching up. Canonical Gaffer-On-Games "Fix Your
	// Timestep" pattern. Default is 2 sub-steps at FixedHz (~33ms at 60Hz).
	UPROPERTY(EditAnywhere, Category = "Timestep", meta = (EditCondition = "bFixedTimestep"))
	float MaxCatchUpSeconds = 0.033f;

	// Spring stiffness — higher values make instability appear sooner
	UPROPERTY(EditAnywhere, Category = "Spring")
	float SpringK = 12.f;

	// Damping — keep at 0 so Euler's energy error is visible as growing amplitude
	UPROPERTY(EditAnywhere, Category = "Spring")
	float Damping = 0.f;

	// Initial displacement from spawn position (cm)
	UPROPERTY(EditAnywhere, Category = "Spring")
	float InitialDisplacement = 200.f;

	// Periodically inject a large DeltaTime hitch to trigger instability
	UPROPERTY(EditAnywhere, Category = "Timestep")
	bool bSimulateSpikes = true;

	UPROPERTY(EditAnywhere, Category = "Timestep", meta = (EditCondition = "bSimulateSpikes"))
	float SpikeInterval = 5.f;

	// Offset the first spike so two actors sharing a SpikeInterval don't fire simultaneously.
	// e.g. variable actor at 0.0s, fixed actor at 2.5s (half-interval) makes the contrast clean.
	UPROPERTY(EditAnywhere, Category = "Timestep", meta = (EditCondition = "bSimulateSpikes"))
	float SpikePhaseOffset = 0.f;

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

	UPROPERTY() UTextRenderComponent*  Label = nullptr;
	UPROPERTY() UPointLightComponent*  Light = nullptr;
	TArray<FVector> TrailPoints;

	UMaterialInstanceDynamic* DynMaterial = nullptr;
};
