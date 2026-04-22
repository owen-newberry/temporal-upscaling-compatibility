// Fill out your copyright notice in the Description page of Project Settings.

#include "MoverActor.h"
#include "MotionAuthorityActor.h"
#include "Engine/Engine.h"

AMoverActor::AMoverActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Movable);
	RootComponent = Mesh;

	// Auto-assign the engine built-in Cube so the actor is always visible
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AMoverActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("[MoverActor] BeginPlay fired: %s"), *GetName());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("MoverActor SPAWNED: %s"), *GetName()));
	}
}

void AMoverActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bUseAuthority = IsValid(AuthorityManager) && AuthorityManager->bAuthorityMode;

	if (bUseAuthority)
	{
		// Authority mode: submit inputs only — do NOT modify transform here
		AuthorityManager->SubmitInput(this, MoveVelocity, MoveForce);
	}
	else
	{
		// Direct mode: actor moves itself
		const FVector Delta = (MoveVelocity + MoveForce) * DeltaTime;
		AddActorWorldOffset(Delta);

		UE_LOG(LogTemp, Error, TEXT("[MoverActor] TICK %s | pos %s | delta %.2f cm"),
			*GetName(),
			*GetActorLocation().ToString(),
			Delta.Size());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				(uint64)GetUniqueID(), 0.f, FColor::Green,
				FString::Printf(TEXT("[MoverActor] %s | %s"), *GetName(), *GetActorLocation().ToString())
			);
		}
	}
}

