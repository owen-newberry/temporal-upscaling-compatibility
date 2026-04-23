// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionModelActor.h"
#include "DemoVisuals.h"
#include "MotionLogger.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
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

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Mesh);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Mesh);
}

void AMotionModelActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	FramePhase    = InitialFramePhase;
	SpikeTimer    = 0.f;

	UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
	if (BaseMat)
	{
		DynMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mesh->SetMaterial(0, DynMaterial);
	}

	const FLinearColor ModeColor = bTimeBased
		? FLinearColor(0.15f, 0.95f, 0.35f)
		: FLinearColor(0.95f, 0.20f, 0.15f);
	const FString ModeText = bTimeBased ? TEXT("TIME-BASED") : TEXT("FRAME-BASED");
	DemoVisuals::ConfigureLabel(Label, Mesh, ModeText, ModeColor);
	DemoVisuals::ConfigureLight(Light, ModeColor);

	TrailPoints.Reset();

	UE_LOG(LogTemp, Warning, TEXT("[MotionModelActor] BeginPlay: %s | mode=%s"),
		*GetName(), bTimeBased ? TEXT("TimeBased") : TEXT("FrameBased"));
}

void AMotionModelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float WorldTime = GetWorld()->GetTimeSeconds();

	// Ground-truth position at this moment in time (used for error measurement)
	const float TrueX = Amplitude * FMath::Sin(2.f * PI * Frequency * WorldTime);

	float PosX = 0.f;

	if (bTimeBased)
	{
		// Correct: always derived from total world time — frame rate irrelevant.
		// Self-corrects instantly after any hitch.
		PosX = TrueX;
	}
	else
	{
		// Wrong: phase advances by a fixed step per frame, assuming a target FPS.
		// During a stall the actor freezes; after it resumes from the wrong phase.
		// When FPS > AssumedFPS the oscillation runs fast; when FPS < AssumedFPS it runs slow.

		if (bSimulateSpikes)
		{
			// Check if a new spike should fire
			SpikeTimer += DeltaTime;
			if (SpikeTimer >= SpikeInterval && SpikeRemaining <= 0.f)
			{
				SpikeRemaining = SpikeDuration;
				SpikeTimer     = 0.f;
				UE_LOG(LogTemp, Warning,
					TEXT("[MotionModelActor] %s: STALL started (%.2fs)"), *GetName(), SpikeDuration);
			}

			// While stalling, consume time but don't advance phase
			if (SpikeRemaining > 0.f)
			{
				SpikeRemaining -= DeltaTime;
				PosX = Amplitude * FMath::Sin(FramePhase); // hold last position
			}
			else
			{
				const float PhasePerFrame = 2.f * PI * Frequency / FMath::Max(AssumedFPS, 1.f);
				FramePhase += PhasePerFrame;
				PosX = Amplitude * FMath::Sin(FramePhase);
			}
		}
		else
		{
			const float PhasePerFrame = 2.f * PI * Frequency / FMath::Max(AssumedFPS, 1.f);
			FramePhase += PhasePerFrame;
			PosX = Amplitude * FMath::Sin(FramePhase);
		}
	}

	const FVector PrevPos    = GetActorLocation();
	const FVector NewPos     = SpawnLocation + FVector(PosX, 0.f, 0.f);
	const float   FrameDelta = (NewPos - PrevPos).Size();
	// Positional error vs ground-truth sine — this is the key metric for Demo 3
	const float   PosError   = FMath::Abs(PosX - TrueX);

	SetActorLocation(NewPos);

	if (DynMaterial)
	{
		DynMaterial->SetVectorParameterValue(TEXT("Color"),
			bTimeBased ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.f));
	}

	DemoVisuals::PushTrailSample(TrailPoints, NewPos);
	DemoVisuals::DrawTrail(GetWorld(), TrailPoints,
		bTimeBased ? FLinearColor(0.15f, 0.95f, 0.35f)
		           : FLinearColor(0.95f, 0.20f, 0.15f));

	FMotionLogger::Get().LogRow(
		GFrameCounter, WorldTime,
		GetWorld()->GetMapName(),
		GetName(), bTimeBased ? TEXT("TimeBased") : TEXT("FrameBased"),
		NewPos, FrameDelta, PosError);

	const float CurrentFPS = (DeltaTime > 0.f) ? (1.f / DeltaTime) : 0.f;

	if (GEngine)
	{
		const bool bStalling = !bTimeBased && (SpikeRemaining > 0.f);
		const FString StallStr = bStalling
			? FString::Printf(TEXT(" [STALL %.2fs]"), SpikeRemaining)
			: TEXT("");

		GEngine->AddOnScreenDebugMessage(
			(uint64)GetUniqueID(), 0.f,
			bTimeBased ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[%s] %s | x=%.1f | err=%.1fcm | %.0f fps%s"),
				bTimeBased ? TEXT("TimeBased") : TEXT("FrameBased"),
				*GetName(), PosX, PosError, CurrentFPS, *StallStr));
	}
}
