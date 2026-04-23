// Fill out your copyright notice in the Description page of Project Settings.

#include "WorkloadActor.h"
#include "DemoVisuals.h"
#include "MotionLogger.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformProcess.h"

AWorkloadActor::AWorkloadActor()
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

void AWorkloadActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation   = GetActorLocation();
	TimeElapsed     = 0.f;
	TotalDeferred   = 0;
	TotalProcessed  = 0;
	TaskQueue.Reset();

	UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
	if (BaseMat)
	{
		DynMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mesh->SetMaterial(0, DynMaterial);
	}

	const FLinearColor ModeColor = bBudgeted
		? FLinearColor(0.15f, 0.95f, 0.35f)
		: FLinearColor(0.95f, 0.20f, 0.15f);
	const FString ModeText = bBudgeted ? TEXT("BUDGETED") : TEXT("UNBUDGETED");
	DemoVisuals::ConfigureLabel(Label, Mesh, ModeText, ModeColor);
	DemoVisuals::ConfigureLight(Light, ModeColor);
	bLastVisualBudgeted = bBudgeted;

	TrailPoints.Reset();

	UE_LOG(LogTemp, Warning, TEXT("[WorkloadActor] BeginPlay: %s | mode=%s"),
		*GetName(), bBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"));
}

void AWorkloadActor::ProcessTask()
{
	// Wall-time busy loop: burn CPU until TaskDurationMs has elapsed.
	// FPlatformProcess::Sleep() on Windows rounds up to the OS timer
	// granularity (~1ms minimum, often 15ms), so sub-millisecond sleeps
	// effectively return immediately and produce no measurable cost.
	// Polling FPlatformTime::Seconds() gives us reliable microsecond-scale
	// work that scales predictably with TaskDurationMs.
	const double StartSec = FPlatformTime::Seconds();
	const double BurnSec  = (double)TaskDurationMs / 1000.0;
	volatile double Dummy = 0.0;
	while ((FPlatformTime::Seconds() - StartSec) < BurnSec)
	{
		Dummy += FMath::Sin(Dummy) + 1.0;
	}
	TotalProcessed++;
}

void AWorkloadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeElapsed += DeltaTime;

	// Refill queue each frame. Queue entries are placeholders — the actual
	// work is parameterized by TaskDurationMs inside ProcessTask().
	for (int32 i = 0; i < TasksPerFrame; ++i)
	{
		TaskQueue.Add(0);
	}

	// Cap queue to prevent unbounded growth if budget < tasks-per-frame
	const int32 MaxQueueSize = TasksPerFrame * 10;
	if (TaskQueue.Num() > MaxQueueSize)
	{
		const int32 Dropped = TaskQueue.Num() - MaxQueueSize;
		TaskQueue.RemoveAt(0, Dropped, EAllowShrinking::No);
		UE_LOG(LogTemp, Warning, TEXT("[WorkloadActor] %s: queue overflow, dropped %d tasks"),
			*GetName(), Dropped);
	}

	const double BudgetSeconds = (double)BudgetMs / 1000.0;
	const double StartTime     = FPlatformTime::Seconds();
	TasksDeferred = 0;

	if (bBudgeted)
	{
		// Process tasks until budget is exhausted, defer the rest
		while (TaskQueue.Num() > 0)
		{
			if ((FPlatformTime::Seconds() - StartTime) >= BudgetSeconds)
			{
				TasksDeferred = TaskQueue.Num();
				TotalDeferred += TasksDeferred;
				TaskQueue.Reset();
				break;
			}
			ProcessTask();
			TaskQueue.Pop(EAllowShrinking::No);
		}
	}
	else
	{
		// Unbudgeted: flush every task this frame regardless of cost
		while (TaskQueue.Num() > 0)
		{
			ProcessTask();
			TaskQueue.Pop(EAllowShrinking::No);
		}
	}

	const double FrameWorkMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	// Oscillate along X — spikes show as position jumps in unbudgeted mode
	const FVector PrevPos  = GetActorLocation();
	const FVector NewPos   = SpawnLocation + FVector(
		Amplitude * FMath::Sin(2.f * PI * Frequency * TimeElapsed), 0.f, 0.f);
	const float FrameDelta = (NewPos - PrevPos).Size();

	SetActorLocation(NewPos);

	if (DynMaterial)
	{
		DynMaterial->SetVectorParameterValue(TEXT("Color"),
			bBudgeted ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.f));
	}

	// Refresh label + point light if mode changed after BeginPlay ran.
	// Coordinators spawn actors with default properties, then set
	// bBudgeted — so the first BeginPlay always captures the default
	// "BUDGETED" label. Detect the mismatch here and update the visuals
	// to match the authoritative bBudgeted value.
	if (bBudgeted != bLastVisualBudgeted)
	{
		const FLinearColor ModeColor = bBudgeted
			? FLinearColor(0.15f, 0.95f, 0.35f)
			: FLinearColor(0.95f, 0.20f, 0.15f);
		DemoVisuals::ConfigureLabel(Label, Mesh,
			bBudgeted ? TEXT("BUDGETED") : TEXT("UNBUDGETED"),
			ModeColor);
		DemoVisuals::ConfigureLight(Light, ModeColor);
		bLastVisualBudgeted = bBudgeted;
	}

	DemoVisuals::PushTrailSample(TrailPoints, NewPos);
	DemoVisuals::DrawTrail(GetWorld(), TrailPoints,
		bBudgeted ? FLinearColor(0.15f, 0.95f, 0.35f)
		          : FLinearColor(0.95f, 0.20f, 0.15f));

	// PositionErrorCm is irrelevant for this demo (both modes use the same
	// time-based motion, so position will be identical across modes); pass -1.
	// FrameWorkMs IS the demo's real metric — per-actor CPU cost this frame.
	FMotionLogger::Get().LogRow(
		GFrameCounter, GetWorld()->GetTimeSeconds(),
		GetWorld()->GetMapName(),
		GetName(), bBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"),
		NewPos, FrameDelta,
		/*PositionErrorCm=*/ -1.f,
		/*FrameWorkMs=*/ static_cast<float>(FrameWorkMs));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			(uint64)GetUniqueID(), 0.f,
			bBudgeted ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[%s] %s | work=%.2fms deferred=%d"),
				bBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"),
				*GetName(), FrameWorkMs, TasksDeferred));
	}
}
