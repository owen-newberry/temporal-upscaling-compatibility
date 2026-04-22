// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionModelActor.generated.h"

/**
 * Demo 3 — Time-based vs Frame-based Motion
 *
 * Time-based (green):  position = Amplitude * Sin(2π * Frequency * WorldTime)
 *                      Correct — speed is independent of frame rate.
 *
 * Frame-based (red):   position advances by a fixed amount each Tick, ignoring DeltaTime.
 *                      Wrong  — oscillation slows down when FPS drops, speeds up when FPS rises.
 *
 * Enable bSimulateSpikes to inject periodic frame hitches so the divergence is visible:
 * the frame-based actor's oscillation stalls during the hitch; the time-based actor
 * catches up instantly on the next frame.
 */
UCLASS()
class DEMOS_API AMotionModelActor : public AActor
{
	GENERATED_BODY()

public:
	AMotionModelActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	// True = time-based (correct). False = frame-based (frame-rate dependent).
	UPROPERTY(EditAnywhere, Category = "Motion Model")
	bool bTimeBased = true;

	UPROPERTY(EditAnywhere, Category = "Motion")
	float Amplitude = 200.f;

	UPROPERTY(EditAnywhere, Category = "Motion")
	float Frequency = 0.5f;

	// Target FPS assumed by the frame-based mode for its per-frame step size
	UPROPERTY(EditAnywhere, Category = "Motion Model", meta = (EditCondition = "!bTimeBased"))
	float AssumedFPS = 60.f;

	UPROPERTY(EditAnywhere, Category = "Motion Model")
	bool bSimulateSpikes = true;

	UPROPERTY(EditAnywhere, Category = "Motion Model", meta = (EditCondition = "bSimulateSpikes"))
	float SpikeInterval = 4.f;

	// SpikeDeltaTime removed — spikes on time-based path have no effect (by design).
	// Frame-based path demonstrates divergence by NOT catching up after a hitch.

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	FVector SpawnLocation;
	float   FramePhase  = 0.f;   // accumulated phase for frame-based mode (radians)
	float   SpikeTimer  = 0.f;

	UMaterialInstanceDynamic* DynMaterial = nullptr;
};
