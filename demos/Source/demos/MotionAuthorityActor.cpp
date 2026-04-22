// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionAuthorityActor.h"

AMotionAuthorityActor::AMotionAuthorityActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMotionAuthorityActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMotionAuthorityActor::SubmitInput(AActor* Actor, FVector Velocity, FVector Force)
{
	if (!IsValid(Actor)) return;

	FMotionInput Input;
	Input.Actor    = Actor;
	Input.Velocity = Velocity;
	Input.Force    = Force;
	PendingInputs.Add(Input);
}

void AMotionAuthorityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAuthorityMode)
	{
		PendingInputs.Reset();
		return;
	}

	for (const FMotionInput& Input : PendingInputs)
	{
		if (!IsValid(Input.Actor)) continue;

		const FVector OldPos       = Input.Actor->GetActorLocation();
		const FVector FrameDelta   = (Input.Velocity + Input.Force) * DeltaTime;
		const FVector NewPos       = OldPos + FrameDelta;

		Input.Actor->SetActorLocation(NewPos);

		UE_LOG(LogTemp, Verbose, TEXT("[MotionAuthority] %s | %s -> %s | delta %.2f cm"),
			*Input.Actor->GetName(),
			*OldPos.ToString(),
			*NewPos.ToString(),
			FrameDelta.Size());

		// Jitter detection: flag large frame-to-frame delta changes
		if (const FVector* PrevDelta = LastDeltas.Find(Input.Actor))
		{
			const float Diff = (FrameDelta - *PrevDelta).Size();
			if (Diff > 50.0f) // threshold in cm
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[MotionAuthority] JITTER on %s: delta change = %.2f cm"),
					*Input.Actor->GetName(), Diff);
			}
		}

		LastDeltas.Add(Input.Actor, FrameDelta);
	}

	PendingInputs.Reset();
}

