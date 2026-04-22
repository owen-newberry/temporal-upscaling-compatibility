// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DemosGameInstance.generated.h"

/**
 * Custom GameInstance for the demos project.
 * Owns the FMotionLogger lifecycle so the CSV file is properly
 * reset and flushed between PIE sessions and standalone runs.
 */
UCLASS()
class DEMOS_API UDemosGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
