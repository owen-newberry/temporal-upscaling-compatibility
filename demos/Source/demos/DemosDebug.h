// Temporary diagnostics for map load / PIE vs Standalone — set to 0 to disable.
#pragma once

#include "CoreMinimal.h"

#ifndef DEMOS_STANDALONE_DIAGNOSTICS
#define DEMOS_STANDALONE_DIAGNOSTICS 0
#endif

class AActor;
class UGameInstance;

#if DEMOS_STANDALONE_DIAGNOSTICS

// Call once a few hundred ms after UGameInstance::Init (e.g. from a one-shot
// timer) so a UWorld is bound. Logs to Output Log and (client games) on-screen.
// Module-internal (no DEMOS_API — avoids MSVC/UE issues with __declspec on free functions)
void DemosLogStartupMapContext(UGameInstance* GameInstance);

void DemosLogCoordinatorBeginPlay(AActor* Coordinator, int32 TotalSpawned, bool bSpawnedWorkload,
                                 bool bUnbudgetedWorkload);

void DemosLogWorkloadPairSpawn(AActor* Coordinator, int32 Succeeded, int32 Failed, bool bModeBudgeted);

void DemosLogSwarmNullWorld(AActor* Coordinator);
void DemosLogSwarmDone(AActor* Coordinator, int32 N, const TCHAR* ModeStr);

#else

inline void DemosLogStartupMapContext(UGameInstance* GameInstance) { (void)GameInstance; }
inline void DemosLogCoordinatorBeginPlay(AActor*, int32, bool, bool) {}
inline void DemosLogWorkloadPairSpawn(AActor*, int32, int32, bool) {}
inline void DemosLogSwarmNullWorld(AActor*) {}
inline void DemosLogSwarmDone(AActor*, int32, const TCHAR*) {}

#endif
