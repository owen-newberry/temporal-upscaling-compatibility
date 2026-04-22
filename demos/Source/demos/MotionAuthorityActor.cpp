// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionAuthorityActor.h"
#include "MotionLogger.h"
#include "Engine/Engine.h"

AMotionAuthorityActor::AMotionAuthorityActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMotionAuthorityActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMotionAuthorityActor::SubmitInput(AActor* Actor, FVector TargetLocation)
{
	if (!IsValid(Actor)) return;

	FMotionInput Input;
	Input.Actor          = Actor;
	Input.TargetLocation = TargetLocation;
	PendingInputs.Add(Input);
}

void AMotionAuthorityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAuthorityMode)
	{
		// Log a Dropped row for every input that was discarded this frame so the
		// CSV fully represents every frame regardless of mid-session mode toggles.
		for (const FMotionInput& Input : PendingInputs)
		{
			if (!IsValid(Input.Actor)) continue;
			FMotionLogger::Get().LogRow(
				GFrameCounter, GetWorld()->GetTimeSeconds(),
				GetWorld()->GetMapName(),
				Input.Actor->GetName(), TEXT("Dropped"),
				Input.Actor->GetActorLocation(), 0.f);
		}
		FMotionLogger::Get().Flush();
		PendingInputs.Reset();
		return;
	}

	for (const FMotionInput& Input : PendingInputs)
	{
		if (!IsValid(Input.Actor)) continue;

		const FVector OldPos     = Input.Actor->GetActorLocation();
		const FVector FrameDelta = Input.TargetLocation - OldPos;

		Input.Actor->SetActorLocation(Input.TargetLocation);

		// Jitter detection: flag large frame-to-frame delta changes
		if (const FVector* PrevDelta = LastDeltas.Find(Input.Actor))
		{
			const float Diff = (FrameDelta - *PrevDelta).Size();
			if (Diff > 50.0f)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[MotionAuthority] JITTER on %s: delta change = %.2f cm"),
					*Input.Actor->GetName(), Diff);
			}
		}

		LastDeltas.Add(Input.Actor, FrameDelta);

		FMotionLogger::Get().LogRow(
			GFrameCounter, GetWorld()->GetTimeSeconds(),
			GetWorld()->GetMapName(),
			Input.Actor->GetName(), TEXT("Authority"),
			Input.TargetLocation, FrameDelta.Size());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				(uint64)Input.Actor->GetUniqueID(), 0.f, FColor::Green,
				FString::Printf(TEXT("[Authority] %s | %s"), *Input.Actor->GetName(), *Input.TargetLocation.ToString())
			);
		}
	}

	PendingInputs.Reset();

	FMotionLogger::Get().Flush();
}

