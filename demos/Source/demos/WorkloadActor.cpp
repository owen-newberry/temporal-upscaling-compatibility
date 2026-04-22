// Fill out your copyright notice in the Description page of Project Settings.

#include "WorkloadActor.h"
#include "MotionLogger.h"
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

	UE_LOG(LogTemp, Warning, TEXT("[WorkloadActor] BeginPlay: %s | mode=%s"),
		*GetName(), bBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"));
}

void AWorkloadActor::ProcessTask()
{
	// Sleep 0.05ms per task — guaranteed cost on any hardware regardless of CPU speed.
	// At TasksPerFrame=80 unbudgeted this is 4ms total per frame (~25fps cap).
	// Budgeted mode cuts out after BudgetMs=2ms, keeping frame time bounded.
	FPlatformProcess::Sleep(0.00005f);
	TotalProcessed++;
}

void AWorkloadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeElapsed += DeltaTime;

	// Refill queue each frame
	for (int32 i = 0; i < TasksPerFrame; ++i)
	{
		TaskQueue.Add(IterationsPerTask);
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

	FMotionLogger::Get().LogRow(
		GFrameCounter, GetWorld()->GetTimeSeconds(),
		GetWorld()->GetMapName(),
		GetName(), bBudgeted ? TEXT("Budgeted") : TEXT("Unbudgeted"),
		NewPos, FrameDelta);

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
