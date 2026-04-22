// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionAuthorityActor.generated.h"

// Input submitted by a client actor each frame
USTRUCT()
struct FMotionInput
{
	GENERATED_BODY()

	AActor* Actor = nullptr;
	FVector Velocity = FVector::ZeroVector;
	FVector Force    = FVector::ZeroVector;
};

UCLASS()
class DEMOS_API AMotionAuthorityActor : public AActor
{
	GENERATED_BODY()

public:
	AMotionAuthorityActor();

	// True  = central manager applies all movement (Authority mode)
	// False = actors move themselves            (Direct mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Authority")
	bool bAuthorityMode = true;

	// Called by client actors each frame to register intended movement
	void SubmitInput(AActor* Actor, FVector Velocity, FVector Force);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	TArray<FMotionInput> PendingInputs;

	// Per-actor last-frame delta — used for jitter detection
	TMap<AActor*, FVector> LastDeltas;
};
