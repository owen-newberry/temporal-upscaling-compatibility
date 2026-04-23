// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Engine/GameInstance.h"
#include "DemosGameInstance.generated.h"

/**
 * Custom GameInstance for the demos project.
 * Owns the FMotionLogger lifecycle so the CSV file is properly
 * reset and flushed between PIE sessions and standalone runs.
 * Also drives FPerfLogger system-metric sampling at a fixed interval.
 */
UCLASS()
class DEMOS_API UDemosGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	// Seconds between PerfLogger samples. 10 Hz gives readable trends without
	// bloating the CSV; bump up for shorter capture sessions if needed.
	UPROPERTY(EditAnywhere, Config, Category = "Demos|Performance")
	float PerfSampleIntervalSeconds = 0.1f;

private:
	FTSTicker::FDelegateHandle PerfSampleHandle;
};
