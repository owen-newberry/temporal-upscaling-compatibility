// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
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

	// Velocity submitted to the manager (or applied directly)
	UPROPERTY(EditAnywhere, Category = "Motion Authority")
	FVector MoveVelocity = FVector(100.f, 0.f, 0.f);

	// Additional force submitted each frame
	UPROPERTY(EditAnywhere, Category = "Motion Authority")
	FVector MoveForce = FVector::ZeroVector;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
