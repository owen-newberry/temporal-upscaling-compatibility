// Fill out your copyright notice in the Description page of Project Settings.

#include "DemoVisuals.h"

#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

namespace DemoVisuals
{
	void ConfigureLabel(UTextRenderComponent* Label,
	                    const UStaticMeshComponent* Mesh,
	                    const FString& Text,
	                    const FLinearColor& Color)
	{
		if (!Label) return;

		Label->SetText(FText::FromString(Text));
		Label->SetTextRenderColor(Color.ToFColor(true));
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetVerticalAlignment(EVRTA_TextCenter);
		Label->SetWorldSize(40.f);

		// Float ~150cm above the cube's top face. Mesh bounds give us the
		// actual visible height so the label sits just above any scale.
		float TopOffset = 75.f; // default for a unit cube (100cm) at scale 1
		if (Mesh && Mesh->GetStaticMesh())
		{
			const FBox Bounds = Mesh->GetStaticMesh()->GetBounds().GetBox();
			TopOffset = Bounds.Max.Z * Mesh->GetComponentScale().Z + 50.f;
		}
		Label->SetRelativeLocation(FVector(0.f, 0.f, TopOffset));

		// Rotate 180° around Z so the text faces -X, which is the default
		// perspective-camera look direction when you drop an actor at origin.
		Label->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	}

	void ConfigureLight(UPointLightComponent* Light,
	                    const FLinearColor& Color)
	{
		if (!Light) return;

		Light->SetLightColor(Color);
		Light->SetIntensity(5000.f);          // generous for visual impact
		Light->SetAttenuationRadius(600.f);
		Light->SetCastShadows(false);         // cheap, avoids shadow popping
		Light->SetRelativeLocation(FVector::ZeroVector);
	}

	void DrawTrail(UWorld* World,
	               const TArray<FVector>& Positions,
	               const FLinearColor& Color)
	{
		if (!World || Positions.Num() < 2) return;

		const int32 N = Positions.Num();
		for (int32 i = 1; i < N; ++i)
		{
			// Fade alpha from 0 (oldest) to 1 (newest).
			const float T = (float)i / (float)(N - 1);
			FLinearColor Lin = Color;
			Lin.A = T;
			const FColor C = Lin.ToFColor(true);

			DrawDebugLine(
				World,
				Positions[i - 1],
				Positions[i],
				C,
				/*bPersistent=*/ false,
				/*LifeTime=*/    0.f,           // one-frame
				/*DepthPriority=*/0,
				/*Thickness=*/   2.f * T + 0.5f // thicker for newer segments
			);
		}
	}

	void PushTrailSample(TArray<FVector>& Positions,
	                     const FVector& NewPos,
	                     int32 MaxSamples)
	{
		Positions.Add(NewPos);
		while (Positions.Num() > MaxSamples)
		{
			Positions.RemoveAt(0, 1, EAllowShrinking::No);
		}
	}
}
