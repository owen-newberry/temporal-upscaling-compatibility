// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MoverActor.generated.h"

class AMotionAuthorityActor;

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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	FVector SpawnLocation;
	float TimeElapsed = 0.f;
	UMaterialInstanceDynamic* DynMaterial = nullptr;
};
