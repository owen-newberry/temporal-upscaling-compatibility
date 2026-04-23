// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UTextRenderComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class UWorld;

/**
 * Shared visual-polish helpers used by the demo actors.
 *
 * The four demos each place a pair of cubes oscillating in a scene. Without
 * visual affordances it's hard for an audience to tell which cube is which
 * at a glance, and the flat cube rendering looks bench-y. These helpers add:
 *
 *  1. A floating text label above each mesh describing its mode.
 *  2. A colored point light attached to the mesh so green/red modes glow.
 *  3. A fading motion trail drawn behind the mesh using debug lines.
 *
 * Each actor calls the helper(s) it needs from its constructor (for components
 * that must exist as CDO children) and BeginPlay (for runtime configuration).
 */
namespace DemoVisuals
{
	/** Configure a label so it reads `Text`, floats above Mesh, and faces -X
	 *  (the default editor-camera direction). Sets world size and color. */
	void ConfigureLabel(UTextRenderComponent* Label,
	                    const UStaticMeshComponent* Mesh,
	                    const FString& Text,
	                    const FLinearColor& Color);

	/** Configure a point light to match the mode color with a sensible
	 *  intensity/radius for a single-cube demo. */
	void ConfigureLight(UPointLightComponent* Light,
	                    const FLinearColor& Color);

	/** Draw a fading trail of the last `Positions` samples using the given
	 *  color. Call from an actor's Tick with a ring-buffer of its recent
	 *  world positions. */
	void DrawTrail(UWorld* World,
	               const TArray<FVector>& Positions,
	               const FLinearColor& Color);

	/** Push a new sample into a ring buffer of the given max length. */
	void PushTrailSample(TArray<FVector>& Positions,
	                     const FVector& NewPos,
	                     int32 MaxSamples = 40);
}
