// Fill out your copyright notice in the Description page of Project Settings.

#include "MoverActor.h"
#include "DemoVisuals.h"
#include "MotionAuthorityActor.h"
#include "MotionLogger.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"

AMoverActor::AMoverActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetMobility(EComponentMobility::Movable);
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	Label->SetupAttachment(Mesh);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Mesh);
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

	bLastAuthorityState = IsValid(AuthorityManager) && AuthorityManager->bAuthorityMode;
	const FLinearColor ModeColor = bLastAuthorityState
		? FLinearColor(0.15f, 0.95f, 0.35f)
		: FLinearColor(0.95f, 0.20f, 0.15f);
	const FString ModeText = bLastAuthorityState ? TEXT("AUTHORITY") : TEXT("DIRECT");
	DemoVisuals::ConfigureLabel(Label, Mesh, ModeText, ModeColor);
	DemoVisuals::ConfigureLight(Light, ModeColor);

	TrailPoints.Reset();

	UE_LOG(LogTemp, Warning, TEXT("[MoverActor] BeginPlay: %s | spawn %s"),
		*GetName(), *SpawnLocation.ToString());
}

void AMoverActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FMotionLogger::Get().Flush();
	Super::EndPlay(EndPlayReason);
}

void AMoverActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Use world time so phase is deterministic and not affected by BeginPlay order.
	// Motion is on the Y axis so the oscillation reads as "front-to-back" from
	// the L_MotionAuthority level's camera, matching the framing of the other
	// demos (whose cameras are oriented 90° differently, so their X-axis motion
	// also reads as front-to-back).
	const float WorldTime = GetWorld()->GetTimeSeconds();
	const FVector DesiredPos = SpawnLocation + FVector(
		0.f, Amplitude * FMath::Sin(2.f * PI * Frequency * WorldTime), 0.f);

	// Phantom writer: a rival system that also wants this cube's transform.
	// High-frequency, low-amplitude offset layered on top of the primary
	// motion — deterministic so captures are reproducible. See MoverActor.h
	// for the full rationale. This vector is what a SECOND subsystem
	// (scripted animation, physics contact, network correction, etc.)
	// would request. It is applied DIRECTLY in Direct mode and IGNORED
	// in Authority mode — because Authority only commits inputs that
	// came through AuthorityManager::SubmitInput(), and the phantom
	// never submits.
	const float PhantomOffsetCm = bEnablePhantomWriter
		? PhantomAmplitudeCm * FMath::Sin(2.f * PI * PhantomFrequencyHz * WorldTime)
		: 0.f;
	const FVector PhantomDesiredPos = DesiredPos + FVector(0.f, PhantomOffsetCm, 0.f);

	const bool bUseAuthority = IsValid(AuthorityManager) && AuthorityManager->bAuthorityMode;

	const FLinearColor ModeColor = bUseAuthority
		? FLinearColor(0.15f, 0.95f, 0.35f)
		: FLinearColor(0.95f, 0.20f, 0.15f);

	if (DynMaterial)
	{
		DynMaterial->SetVectorParameterValue(TEXT("Color"),
			bUseAuthority ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.f));
	}

	// Refresh label + light if the authority mode was toggled live.
	if (bUseAuthority != bLastAuthorityState)
	{
		DemoVisuals::ConfigureLabel(Label, Mesh,
			bUseAuthority ? TEXT("AUTHORITY") : TEXT("DIRECT"),
			ModeColor);
		DemoVisuals::ConfigureLight(Light, ModeColor);
		bLastAuthorityState = bUseAuthority;
	}

	const FVector PrevPos = GetActorLocation();

	if (bUseAuthority)
	{
		// Authority mode: primary submits, phantom is silently rejected
		// (never calls SubmitInput). Authority commits one coherent
		// position next frame — see MotionAuthorityActor::Tick.
		AuthorityManager->SubmitInput(this, DesiredPos);
	}
	else
	{
		// Direct mode: primary writes its intended position, then the
		// phantom overwrites it in the same frame. Whoever writes last
		// wins — this is the classic multi-writer failure mode that
		// shows up as visible jitter.
		SetActorLocation(DesiredPos);
		if (bEnablePhantomWriter)
		{
			SetActorLocation(PhantomDesiredPos);
		}

		// Log the ACTUAL committed position (not the primary's desired),
		// so the CSV reflects what the engine would render and what any
		// temporal upscaler would see as the cube's motion vector.
		const FVector CommittedPos = GetActorLocation();
		const float   CommittedDelta = (CommittedPos - PrevPos).Size();

		FMotionLogger::Get().LogRow(
			GFrameCounter, GetWorld()->GetTimeSeconds(),
			GetWorld()->GetMapName(), GetName(), TEXT("Direct"),
			CommittedPos, CommittedDelta);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				(uint64)GetUniqueID(), 0.f, FColor::Red,
				FString::Printf(TEXT("[Direct] %s | phantom=%.1fcm | %s"),
					*GetName(), PhantomOffsetCm, *CommittedPos.ToString())
			);
		}
	}

	// Trail always reflects the actor's committed location (the authority
	// writes it in its own tick in AUTHORITY mode, this actor writes it in
	// DIRECT mode). Works cleanly either way.
	DemoVisuals::PushTrailSample(TrailPoints, GetActorLocation());
	DemoVisuals::DrawTrail(GetWorld(), TrailPoints, ModeColor);
}

