// Fill out your copyright notice in the Description page of Project Settings.

#include "TimestepActor.h"
#include "MotionLogger.h"
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
}

void ATimestepActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	Displacement  = InitialDisplacement;
	Velocity      = 0.f;
	Accumulator   = 0.f;
	SpikeTimer    = 0.f;

	UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
	if (BaseMat)
	{
		DynMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mesh->SetMaterial(0, DynMaterial);
	}

	FMotionLogger::Get().Init();
	UE_LOG(LogTemp, Warning, TEXT("[TimestepActor] BeginPlay: %s | mode=%s"),
		*GetName(), bFixedTimestep ? TEXT("Fixed") : TEXT("Variable"));
}

void ATimestepActor::StepSpring(float Dt)
{
	// Damped spring: acceleration = -k*displacement - damping*velocity
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
			EffectiveDt = SpikeDeltaTime;
			SpikeTimer  = 0.f;
			UE_LOG(LogTemp, Warning, TEXT("[TimestepActor] %s: SPIKE dt=%.3fs"),
				*GetName(), SpikeDeltaTime);
		}
	}

	if (bFixedTimestep)
	{
		// Accumulate real time and drain in fixed sub-steps
		const float FixedDt = 1.f / FMath::Max(FixedHz, 1.f);
		Accumulator += EffectiveDt;
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

	FMotionLogger::Get().LogRow(
		GFrameCounter, GetWorld()->GetTimeSeconds(),
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
