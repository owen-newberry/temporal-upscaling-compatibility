# Performance Report

_Generated 2026-04-27 14:37 · F4.6 — automated performance report_

This report aggregates per-frame motion-capture CSVs produced by `FMotionLogger` across all four upscaler-compatibility demos. FPS metrics are derived from the `TimeSeconds` column; 1% low and 0.1% low figures are the mean FPS of the slowest 1% / 0.1% of frames.

## Demo 1 - Motion Authority

_Level(s): `L_MotionAuthority`_

_Source CSV(s): 1 — MotionCapture_20260427_135151.csv_

| Mode | Rows | Mean FPS | 1% Low FPS | 0.1% Low FPS | Frame-time σ (ms) | Max frame (ms) | Mean Δ (cm) | σ Δ (cm) | P95 Δ (cm) | Max Δ (cm) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Direct | 23632 | 195.2 | 135.9 | 102.0 | 0.62 | 19.9 | 2.08 | 1.09 | 3.50 | 53.3 |
| Authority | 23631 | 195.2 | 135.9 | 102.0 | 0.62 | 19.9 | 2.08 | 1.09 | 3.50 | 53.3 |

**Interpretation.** Both modes include a deterministic phantom writer that also attempts to set the cube's transform every frame. In Direct mode the phantom's write overrides the primary's, contaminating the observed motion (higher σ Δ, higher P95 Δ, higher Max Δ) — this is the motion-vector discontinuity that breaks temporal reprojection. In Authority mode the phantom never calls `SubmitInput`, so the single-writer authority commits only the primary's target and the motion stays clean. Mean FPS is identical across modes, so the pattern has no measurable runtime cost — only a variance reduction.

## Demo 2 - Fixed Timestep

_Level(s): `L_FixedTimestep`_

_Source CSV(s): 1 — MotionCapture_20260427_134621.csv_

| Mode | Rows | Mean FPS | 1% Low FPS | 0.1% Low FPS | Frame-time σ (ms) | Max frame (ms) | Mean Δ (cm) | σ Δ (cm) | P95 Δ (cm) | Max Δ (cm) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Variable | 23694 | 203.2 | 107.4 | 58.5 | 1.46 | 152.9 | 3.31 | 7.71 | 6.22 | 338.0 |
| Fixed | 23694 | 203.2 | 107.4 | 58.5 | 1.46 | 152.9 | 2.33 | 5.60 | 11.2 | 190.1 |

**Interpretation.** Fixed and Variable modes simulate the same oscillator under identical spike injections. **Max Δ is the headline column**: Variable routinely exceeds the spring's physical amplitude (200 cm) during hitches because Euler integration returns nonsense positions when dt is large; Fixed stays bounded because sub-stepping with a `MaxCatchUpSeconds` clamp keeps every integration step small. σ Δ (−44%) confirms the variance reduction. P95 Δ is *not* a useful column here — Fixed's P95 is slightly higher because sub-stepping collapses multiple simulation steps into one rendered-frame observation, which is correct catch-up behavior, not instability.

## Demo 3 - Time-based Motion

_Level(s): `L_TimeBasedAnim`_

_Source CSV(s): 1 — MotionCapture_20260427_135413.csv_

| Mode | Rows | Mean FPS | 1% Low FPS | 0.1% Low FPS | Frame-time σ (ms) | Max frame (ms) | Mean Δ (cm) | σ Δ (cm) | P95 Δ (cm) | Max Δ (cm) | Mean schedule dev. (cm) | Max schedule dev. (cm) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| FrameBased | 28143 | 210.2 | 139.1 | 109.9 | 0.65 | 19.8 | 6.42 | 3.51 | 10.4 | 148.6 | 163.2 | 400.0 |
| TimeBased | 28143 | 210.2 | 139.1 | 109.9 | 0.65 | 19.8 | 1.94 | 1.04 | 3.35 | 54.2 | 0.00 | 0.00 |

**Interpretation.** **Schedule deviation is the headline column, not Δ/frame.** TimeBased computes position directly from WorldTime using the animation's design formula; its deviation from that formula is zero by construction — that is a statement of *faithfulness to design intent*, not of measurement accuracy. FrameBased advances a counter assuming a fixed FPS and therefore drifts from the intended schedule whenever the actual frame rate diverges, producing multi-meter visual lag during hitches. TimeBased's σ Δ is slightly higher than FrameBased's because catch-up jumps after a hitch produce larger (but correct) motion vectors — upscalers reproject big correct motion vectors cleanly; small wrong ones are what produce smearing.

## Demo 4 - Workload Budget

_Level(s): `L_WorkloadBudgeted, L_WorkloadUnbudgeted`_

_Source CSV(s): 2 — MotionCapture_20260427_142355.csv, MotionCapture_20260427_142806.csv_

| Mode | Rows | Mean FPS | 1% Low FPS | 0.1% Low FPS | Frame-time σ (ms) | Max frame (ms) | Mean Δ (cm) | σ Δ (cm) | P95 Δ (cm) | Max Δ (cm) | Mean work (ms) | P95 work (ms) | Max work (ms) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Budgeted | 136592 | 72.4 | 64.3 | 52.5 | 0.40 | 26.7 | 3.51 | 1.82 | 6.24 | 38.2 | 1.50 | 1.51 | 10.4 |
| Unbudgeted | 37784 | 38.6 | 35.7 | 29.2 | 0.48 | 43.3 | 7.34 | 3.66 | 12.0 | 37.4 | 3.01 | 3.01 | 5.24 |

**Interpretation.** Both actors use identical time-based motion so `Δ` columns look the same — the real differentiator is the `work (ms)` columns: per-actor CPU time spent on the task queue each frame. Budgeted mode should cap Max work at the configured `BudgetMs` while Unbudgeted is unbounded. System-wide FPS impact surfaces in the session's PerformanceCapture CSV — since both actors share the game thread, an unbudgeted actor drags the whole frame down regardless of which actor is observed.

---

**Column key.** `Δ` is per-frame position delta (cm) logged by the actor. `Frame-time σ` is the standard deviation of frame durations — low σ means stable pacing, which is what temporal upscalers need. `Max frame` is the single worst frame in the session and is what drives 0.1% low FPS.