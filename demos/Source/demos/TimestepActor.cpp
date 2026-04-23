// Fill out your copyright notice in the Description page of Project Settings.

#include "TimestepActor.h"
#include "DemoVisuals.h"
#include "MotionLogger.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"

ATimestepActor::ATimestepActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Movable);
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
		Mesh->SetStaticMesh(CubeMesh.Object);

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Mesh);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Mesh);
}

void ATimestepActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	Displacement  = InitialDisplacement;
	Velocity      = 0.f;
	Accumulator   = 0.f;
	// Negative initial timer delays the first spike by SpikePhaseOffset seconds.
	SpikeTimer    = -SpikePhaseOffset;

	UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
	if (BaseMat)
	{
		DynMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mesh->SetMaterial(0, DynMaterial);
	}

	const FLinearColor ModeColor = bFixedTimestep
		? FLinearColor(0.15f, 0.95f, 0.35f)
		: FLinearColor(0.95f, 0.20f, 0.15f);
	const FString ModeText = bFixedTimestep ? TEXT("FIXED") : TEXT("VARIABLE");
	DemoVisuals::ConfigureLabel(Label, Mesh, ModeText, ModeColor);
	DemoVisuals::ConfigureLight(Light, ModeColor);

	TrailPoints.Reset();

	UE_LOG(LogTemp, Warning, TEXT("[TimestepActor] BeginPlay: %s | mode=%s"),
		*GetName(), bFixedTimestep ? TEXT("Fixed") : TEXT("Variable"));
}

void ATimestepActor::StepSpring(float Dt)
{
	// Symplectic Euler: velocity updated first, then displacement uses the new velocity.
	// Stable for oscillators even with Damping=0, unlike forward Euler which gains energy.
	const float Accel = -SpringK * Displacement - Damping * Velocity;
	Velocity     += Accel * Dt;
	Displacement += Velocity * Dt;
}

void ATimestepActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Optionally inject a simulated frame spike
	float EffectiveDt = DeltaTime;
	if (bSimulateSpikes)
	{
		SpikeTimer += DeltaTime;
		if (SpikeTimer >= SpikeInterval)
		{
			// Additive: simulates a real hitch on top of a normal frame
			EffectiveDt = DeltaTime + SpikeDeltaTime;
			SpikeTimer  = 0.f;
			UE_LOG(LogTemp, Warning, TEXT("[TimestepActor] %s: SPIKE dt=%.3fs"),
				*GetName(), SpikeDeltaTime);
		}
	}

	if (bFixedTimestep)
	{
		// Accumulate real time and drain in fixed sub-steps.
		// Gaffer-style clamp: cap the time a single frame can deposit into
		// the accumulator. Without this, a 250ms hitch would cause ~16
		// sub-steps in one frame, making the spring visibly race forward.
		// With the clamp the hitch time is effectively discarded and the
		// spring keeps oscillating at its natural rate across the spike —
		// which is the behaviour temporal upscalers need (per-frame position
		// deltas stay consistent in magnitude).
		const float FixedDt       = 1.f / FMath::Max(FixedHz, 1.f);
		const float ClampedDt     = FMath::Min(EffectiveDt, MaxCatchUpSeconds);
		Accumulator += ClampedDt;
		while (Accumulator >= FixedDt)
		{
			StepSpring(FixedDt);
			Accumulator -= FixedDt;
		}
	}
	else
	{
		// Variable: single large integration step — numerically unstable under spikes
		StepSpring(EffectiveDt);
	}

	const FVector PrevPos  = GetActorLocation();
	const FVector NewPos   = SpawnLocation + FVector(Displacement, 0.f, 0.f);
	const float   FrameDelta = (NewPos - PrevPos).Size();

	SetActorLocation(NewPos);

	if (DynMaterial)
	{
		DynMaterial->SetVectorParameterValue(TEXT("Color"),
			bFixedTimestep ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.f));
	}

	DemoVisuals::PushTrailSample(TrailPoints, NewPos);
	DemoVisuals::DrawTrail(GetWorld(), TrailPoints,
		bFixedTimestep ? FLinearColor(0.15f, 0.95f, 0.35f)
		               : FLinearColor(0.95f, 0.20f, 0.15f));

	FMotionLogger::Get().LogRow(
		GFrameCounter, GetWorld()->GetTimeSeconds(),
		GetWorld()->GetMapName(),
		GetName(), bFixedTimestep ? TEXT("Fixed") : TEXT("Variable"),
		NewPos, FrameDelta);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			(uint64)GetUniqueID(), 0.f,
			bFixedTimestep ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[%s] %s | disp=%.1f"),
				bFixedTimestep ? TEXT("Fixed") : TEXT("Variable"),
				*GetName(), Displacement));
	}
}
