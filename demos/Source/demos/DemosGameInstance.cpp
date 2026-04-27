// Fill out your copyright notice in the Description page of Project Settings.

#include "DemosGameInstance.h"
#include "DemosDebug.h"
#include "MotionLogger.h"
#include "PerfLogger.h"
#include "Containers/Ticker.h"
#include "Templates/SharedPointer.h"

void UDemosGameInstance::Init()
{
	Super::Init();

	// Reset the motion logger so each PIE session / standalone run gets a
	// fresh CSV file. Without this the singleton stays initialized from the
	// previous session and all subsequent sessions either append to the old
	// file or log nothing.
	FMotionLogger::Get().Reset();
	FMotionLogger::Get().Init();

	// Same lifecycle for the system-metric logger.
	FPerfLogger::Get().Reset();
	FPerfLogger::Get().Init();

	// Drive PerfLogger sampling from the core ticker. TWeakObjectPtr keeps the
	// callback safe if the GameInstance is torn down before the ticker fires.
	TWeakObjectPtr<UDemosGameInstance> WeakThis(this);
	PerfSampleHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakThis](float /*Dt*/)
		{
			if (UDemosGameInstance* Self = WeakThis.Get())
			{
				FPerfLogger::Get().SampleOnce(Self->GetWorld());
			}
			return true; // keep ticking
		}),
		PerfSampleIntervalSeconds);

	UE_LOG(LogTemp, Warning,
		TEXT("[DemosGameInstance] Session started — loggers initialized (perf sample %.2fs)."),
		PerfSampleIntervalSeconds);

#if DEMOS_STANDALONE_DIAGNOSTICS
	// UGameInstance::Init can run before a UWorld exists. Log map / world type
	// a quarter-second later so Standalone vs PIE and the real map are visible.
	{
		TWeakObjectPtr<UDemosGameInstance> WSelf(this);
		const TSharedPtr<float> Acc = TSharedPtr<float>(new float(0.0f));
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
			[WSelf, Acc](float DeltaTime)
		{
			*Acc += DeltaTime;
			if (*Acc < 0.25f)
			{
				return true; // keep ticking
			}
			if (UDemosGameInstance* G = WSelf.Get())
			{
				DemosLogStartupMapContext(G);
			}
			return false; // one shot
		}),
		0.0f);
	}
#endif
}

void UDemosGameInstance::Shutdown()
{
	if (PerfSampleHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PerfSampleHandle);
		PerfSampleHandle.Reset();
	}

	FMotionLogger::Get().Flush();
	FMotionLogger::Get().Reset();

	FPerfLogger::Get().Flush();
	FPerfLogger::Get().Reset();

	UE_LOG(LogTemp, Warning, TEXT("[DemosGameInstance] Session ended — loggers flushed."));

	Super::Shutdown();
}
