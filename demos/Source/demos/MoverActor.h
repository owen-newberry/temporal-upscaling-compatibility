// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MoverActor.generated.h"

class AMotionAuthorityActor;
class UTextRenderComponent;
class UPointLightComponent;

UCLASS()
class DEMOS_API AMoverActor : public AActor
{
	GENERATED_BODY()

public:
	AMoverActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	// Assign in editor — leave null for always-direct mode
	UPROPERTY(EditAnywhere, Category = "Motion Authority")
	AMotionAuthorityActor* AuthorityManager;

	// Half the travel distance in cm
	UPROPERTY(EditAnywhere, Category = "Motion")
	float Amplitude = 200.f;

	// Oscillations per second
	UPROPERTY(EditAnywhere, Category = "Motion")
	float Frequency = 0.5f;

	// ── "Phantom writer" — second system competing for the transform ──
	//
	// Simulates a rival system (scripted animation, physics contact
	// solver, network position correction, AI pathfinder) that also
	// wants to move this cube every frame. Exists so Demo 1 actually
	// demonstrates the multi-writer failure mode that the single-writer
	// authority pattern is supposed to fix.
	//
	//  • Direct mode:    phantom writes over the primary's position via
	//                    a second SetActorLocation call → visible jitter.
	//  • Authority mode: the authority is the only code path that commits
	//                    transforms, and the phantom never calls
	//                    AuthorityManager->SubmitInput — so it's
	//                    implicitly rejected. Motion stays clean.
	UPROPERTY(EditAnywhere, Category = "Phantom Writer")
	bool bEnablePhantomWriter = true;

	UPROPERTY(EditAnywhere, Category = "Phantom Writer", meta = (EditCondition = "bEnablePhantomWriter"))
	float PhantomAmplitudeCm = 15.f;

	UPROPERTY(EditAnywhere, Category = "Phantom Writer", meta = (EditCondition = "bEnablePhantomWriter"))
	float PhantomFrequencyHz = 7.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	FVector SpawnLocation;
	UMaterialInstanceDynamic* DynMaterial = nullptr;

	UPROPERTY() UTextRenderComponent* Label = nullptr;
	UPROPERTY() UPointLightComponent* Light = nullptr;
	TArray<FVector> TrailPoints;
	bool bLastAuthorityState = false;
};
