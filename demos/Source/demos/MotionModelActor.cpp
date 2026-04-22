// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionModelActor.h"
#include "MotionLogger.h"
#include "Engine/Engine.h"

AMotionModelActor::AMotionModelActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Movable);
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
		Mesh->SetStaticMesh(CubeMesh.Object);
}

void AMotionModelActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	FramePhase    = 0.f;
	SpikeTimer    = 0.f;

	UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
	if (BaseMat)
	{
		DynMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mesh->SetMaterial(0, DynMaterial);
	}

	UE_LOG(LogTemp, Warning, TEXT("[MotionModelActor] BeginPlay: %s | mode=%s"),
		*GetName(), bTimeBased ? TEXT("TimeBased") : TEXT("FrameBased"));
}

void AMotionModelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Spike only applies to frame-based path (time-based uses WorldTime, which
	// self-corrects — ignoring EffectiveDt here is intentional and is the demo's point).
	if (!bTimeBased && bSimulateSpikes)
	{
		SpikeTimer += DeltaTime;
		if (SpikeTimer >= SpikeInterval)
		{
			SpikeTimer = 0.f;
			UE_LOG(LogTemp, Warning, TEXT("[MotionModelActor] %s: SPIKE injected"), *GetName());
			// Frame-based path uses FramePhase which advances by 1 step regardless — the
			// point is that it DOESN'T catch up, while time-based does automatically.
		}
	}

	float PosX = 0.f;

	if (bTimeBased)
	{
		// Correct: always derived from total world time — frame rate irrelevant
		const float WorldTime = GetWorld()->GetTimeSeconds();
		PosX = Amplitude * FMath::Sin(2.f * PI * Frequency * WorldTime);
	}
	else
	{
		// Wrong: phase advances by a fixed amount per frame, assuming a target FPS.
		// At lower-than-assumed FPS the oscillation runs slow; at higher FPS it runs fast.
		const float PhasePerFrame = 2.f * PI * Frequency / FMath::Max(AssumedFPS, 1.f);
		FramePhase += PhasePerFrame;  // ignores EffectiveDt entirely
		PosX = Amplitude * FMath::Sin(FramePhase);
	}

	const FVector PrevPos  = GetActorLocation();
	const FVector NewPos   = SpawnLocation + FVector(PosX, 0.f, 0.f);
	const float   FrameDelta = (NewPos - PrevPos).Size();

	SetActorLocation(NewPos);

	if (DynMaterial)
	{
		DynMaterial->SetVectorParameterValue(TEXT("Color"),
			bTimeBased ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.f));
	}

	FMotionLogger::Get().LogRow(
		GFrameCounter, GetWorld()->GetTimeSeconds(),
		GetWorld()->GetMapName(),
		GetName(), bTimeBased ? TEXT("TimeBased") : TEXT("FrameBased"),
		NewPos, FrameDelta);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			(uint64)GetUniqueID(), 0.f,
			bTimeBased ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[%s] %s | x=%.1f"),
				bTimeBased ? TEXT("TimeBased") : TEXT("FrameBased"),
				*GetName(), PosX));
	}
}
