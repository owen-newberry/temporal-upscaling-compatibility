// Fill out your copyright notice in the Description page of Project Settings.

#include "MoverActor.h"
#include "MotionAuthorityActor.h"
#include "MotionLogger.h"
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

	SpawnLocation = GetActorLocation();

	// Create a dynamic material instance so we can change color at runtime
	UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
	if (BaseMat)
	{
		DynMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mesh->SetMaterial(0, DynMaterial);
	}

	UE_LOG(LogTemp, Warning, TEXT("[MoverActor] BeginPlay: %s | spawn %s"),
		*GetName(), *SpawnLocation.ToString());
}

void AMoverActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeElapsed += DeltaTime;

	// Use world time so phase is deterministic and not affected by BeginPlay order
	const float WorldTime = GetWorld()->GetTimeSeconds();
	const FVector DesiredPos = SpawnLocation + FVector(
		Amplitude * FMath::Sin(2.f * PI * Frequency * WorldTime), 0.f, 0.f);

	const bool bUseAuthority = IsValid(AuthorityManager) && AuthorityManager->bAuthorityMode;

	// Green = authority mode (good), Red = direct mode (bad)
	if (DynMaterial)
	{
		const FLinearColor ModeColor = bUseAuthority
			? FLinearColor(0.f, 1.f, 0.f)
			: FLinearColor(1.f, 0.f, 0.f);
		DynMaterial->SetVectorParameterValue(TEXT("Color"), ModeColor);
	}

	const FVector PrevPos    = GetActorLocation();
	const float   FrameDelta = (DesiredPos - PrevPos).Size();

	if (bUseAuthority)
	{
		// Authority mode: submit target position — do NOT modify transform here
		AuthorityManager->SubmitInput(this, DesiredPos);
	}
	else
	{
		// Direct mode: actor moves itself
		SetActorLocation(DesiredPos);

		FMotionLogger::Get().LogRow(
			GFrameCounter, GetWorld()->GetTimeSeconds(),
			GetWorld()->GetMapName(), GetName(), TEXT("Direct"), DesiredPos, FrameDelta);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				(uint64)GetUniqueID(), 0.f, FColor::Red,
				FString::Printf(TEXT("[Direct] %s | %s"), *GetName(), *DesiredPos.ToString())
			);
		}
	}
}

