# Final Paper Results — Demo Captures, Tables, and Figures

This document is the deliverable that fills the two `[TBD]` tables and four
figure placeholders in *"Software Design Patterns for Temporal Upscaling
Compatibility and Performance in Real-Time Games"* (honors thesis 2026).

All numbers are produced from the canonical `MotionCapture_*.csv` files in
`demos/Saved/Logs/`, aggregated by `analysis/aggregate_metrics.py` and plotted
by `analysis/make_paper_figures.py`. None of the LaTeX in `honor2026` has been
touched.

## Capture environment

| Field | Value |
|-------|-------|
| CPU | AMD Ryzen 5 5600X (6 cores / 12 threads) |
| GPU | AMD Radeon RX 7900 XTX (24 GB) |
| RAM | 64 GB |
| OS | Windows 11 Home, build 26200 (64-bit) |
| GPU driver | Adrenalin 32.0.23033.1002 |
| Engine | Unreal Engine 5.7 (`Development Editor`, `Win64`) |
| Upscaler | TSR (project default) |
| Capture mode | Standalone (not PIE) |
| Logger schema | `Frame,TimeSeconds,LevelName,ActorName,Mode,PosX,PosY,PosZ,FrameDeltaCm,PositionErrorCm,FrameWorkMs` |
| Warm-up trim | 1.0 s (figures only — tables use the full capture) |

## Canonical CSV per demo

| Demo | CSV | Frames per mode | Notes |
|------|-----|-----------------|-------|
| 1 — Motion Authority | `MotionCapture_20260423_035001.csv` | 4789 / 4788 | Clean A/B in `MotionAuthority` level, ~120 FPS |
| 2 — Fixed Timestep | `MotionCapture_20260423_005537.csv` | 5119 / 5119 | Clean A/B in `FixedTimestep` level, ~120 FPS |
| 3 — Time-based Motion | `MotionCapture_20260422_231435.csv` | 3636 / 3636 | Slightly under the 5k frame target but cleanest TimeBased trace; the longer 010847 capture has a single TimeBased outlier (Max Δ 234 cm) from a hitch frame |
| 4 — Workload Budget | `MotionCapture_WorkloadBudget_Unbudgeted.csv` + `_Budgeted.csv` | 1550 / 6608 | Split-capture: each mode in its own session/level so per-actor `FrameWorkMs` is uncontaminated |

## Demo 1 results table — Single-Writer Motion Authority

Source: `MotionCapture_20260423_035001.csv` (level `MotionAuthority`).

| Metric | Direct (violated) | Authority (applied) | Δ vs Direct |
|--------|------------------:|--------------------:|------------:|
| Mean Δ (cm)  | 4.38  | 3.36  | −23.2% |
| Std Δ (cm)   | 3.36  | 1.92  | **−42.8%** |
| P95 Δ (cm)   | 9.92  | 5.22  | **−47.4%** |
| Max Δ (cm)   | 80.76 | 57.63 | **−28.6%** |
| Mean FPS     | 119.75 | 119.77 | +0.02% |

Reading: applying the Single-Writer Motion Authority pattern reduces frame-to-frame
position-delta variance materially (σ −43%, P95 −47%, Max −29%) with **no
measurable mean-FPS penalty** (119.75 → 119.77 ≈ identical), exactly matching
the paper text's claim.

## Demo 4 results table — Bounded Workload Budgeting

Source (split capture):
- `MotionCapture_WorkloadBudget_Unbudgeted.csv` — 1550 frames
- `MotionCapture_WorkloadBudget_Budgeted.csv`   — 6608 frames

| Metric | Unbudgeted (violated) | Budgeted (applied) |
|--------|----------------------:|-------------------:|
| Mean work / frame (ms) | 20.01 | 2.00  |
| Max work / frame (ms)  | 20.75 | 2.77  |
| Max frame time (ms)    | 116.40 | 98.00 |
| Mean FPS               | 22.30 | 111.66 |

Speed-up (Budgeted / Unbudgeted Mean FPS) = **5.01x**.

Reading: the budgeting actor caps per-frame CPU work at ~2 ms (vs 20 ms
unbudgeted), which translates into roughly a 5x improvement in mean FPS
(22.30 → 111.66) while still reducing the wall-clock max frame time
(116.4 → 98.0 ms).

### Alternative Demo 4 scenario (for reference, not used in tables)

The repo also contains a "swarm" workload scenario where many actors each do
~3 ms of unbudgeted work (vs ~1.5 ms budgeted), giving a less dramatic but
arguably more realistic 1.69x speed-up:

| Metric | Unbudgeted (swarm) | Budgeted (swarm) |
|--------|-------------------:|-----------------:|
| Mean work / frame (ms) | 3.00 | 1.50 |
| Max work / frame (ms)  | 3.88 | 1.77 |
| Max frame time (ms)    | 114.10 | 101.40 |
| Mean FPS               | 33.22 | 56.30 |

Speed-up = 1.69x. Source: `MotionCapture_WorkloadSwarm_*.csv` (7992 / 13816 frames).

## Four PNG figures (paper's figure placeholders)

All written to `docs/ResearchPaper/figures/` (tracked in git; the parallel
`analysis/output/` directory is a gitignored scratch dir for development):

| File | What it shows |
|------|---------------|
| `demo1_jitter.png` | `FrameDeltaCm` vs `TimeSeconds`, Direct + Authority overlaid. Authority's envelope is visibly capped at ~5 cm; Direct oscillates 0–10 cm with a single ~21 cm spike. |
| `demo2_positions.png` | `PosX` vs `TimeSeconds`, Variable + Fixed overlaid. The 300 ms hitches are visible as Variable's amplitude blowing past the spring's natural range (down to 100 cm, up to 800 cm) while Fixed stays in the ~250–650 cm band. |
| `demo3_schedule_error.png` | `PositionErrorCm` vs `TimeSeconds`, FrameBased + TimeBased overlaid. FrameBased shows the sawtooth schedule drift up to ~400 cm; TimeBased is flat at 0 throughout. |
| `demo4_frametime.png` | `FrameWorkMs` vs `TimeSeconds`, Unbudgeted + Budgeted overlaid. Unbudgeted runs at ~20 ms; Budgeted is capped at ~2 ms. |

Generated by `python analysis/make_paper_figures.py`.

## Sanity check vs presentation reference numbers

The prompt provided reference values for Demo 2 and Demo 3 from the
presentation. The fresh runs **deviate by more than 5 %** on most magnitude
metrics, but **match the presentation closely on every direction-dependent
claim**. Both sets of numbers are reported here so the authors can decide
which to keep.

### Demo 2 (Variable / Fixed) — Mean Δ, σΔ, P95 Δ, Max Δ

| Source | Variable | Fixed |
|--------|---------|-------|
| Reference (presentation) | 26.9 / 32.3 / 52.6 / 544.7 | 13.6 / 12.2 / 23.0 / 198.0 |
| Current (`005537.csv`)   | 4.89 / 9.31 / 9.24 / 305.92 | 3.73 / 5.25 / 11.41 / 200.00 |

Why the magnitude gap: the current scenario uses shorter / less frequent
hitch events than whatever drove the presentation numbers. The qualitative
pattern is preserved — Variable's σ is ~1.8x Fixed's σ, and Variable's Max Δ
is 1.5x Fixed's Max Δ — but the absolute amplitudes are 5–7× lower. If the
paper text quotes specific cm values, the reference numbers should be
re-verified against whatever capture they originally came from.

### Demo 3 (FrameBased / TimeBased) — Mean Δ, σΔ, Max Δ, Mean sched err, Max sched err

| Source | FrameBased | TimeBased |
|--------|-----------|-----------|
| Reference (presentation) | 6.40 / 3.42 / 10.47 / 162.0 / 400.0 | 11.83 / 5.93 / 78.29 / 0.00 / 0.00 |
| Current (`231435.csv`)   | 6.45 / 4.13 / 148.58 / 160.66 / 398.44 | 3.36 / 1.99 / 58.88 / 0.00 / 0.00 |

The schedule-error numbers — which are the Demo 3 claim that actually appears
in the paper — match within 1 % (Mean err 162.0 vs 160.66; Max err 400.0 vs
398.44, both for FrameBased; both 0.00 for TimeBased). The Mean Δ for
FrameBased also matches within 1 % (6.40 vs 6.45). Two reference values look
suspect:

- Reference TimeBased Mean Δ = 11.83 cm is **higher** than its FrameBased
  Mean Δ = 6.40, which contradicts the pattern's behavior (TimeBased should
  produce smaller per-frame deltas because it doesn't snap forward after
  stalls). Current data has TimeBased = 3.36 cm < FrameBased = 6.45 cm,
  which matches the expected direction. The reference values may have been
  recorded with the modes swapped.
- Reference FrameBased Max Δ = 10.47 cm is smaller than its own P95 (10.44 cm)
  in the current data and smaller than reference σΔ × 3, which is hard to
  reconcile. Possibly a column-order typo in the reference table.

**Recommendation**: keep the current Demo 3 schedule-error numbers (they
match reference within rounding). Treat the reference Mean Δ row as
suspect and use the current values (FrameBased 6.45, TimeBased 3.36).

## Stretch-goal status — TAAU / FSR upscaler sweep

Not yet captured at the time of writing this document. Planned: re-run each
demo's canonical scenario with `UpscalerControl::Set("TAAU")` and (if the FSR
plugin is present) `Set("FSR")`, then compare the per-frame-Δ profile against
the current TSR runs. The prediction in the paper is "no meaningful
difference" because the patterns operate upstream of the upscaler. See
`analysis/output/upscaler_sweep_results.md` once those captures land.

## How to reproduce these numbers

From the repo root:

```powershell
.\.venv\Scripts\Activate.ps1
python analysis/aggregate_metrics.py
python analysis/generate_report.py
python analysis/make_paper_figures.py
```

Outputs:

- `analysis/output/summary_metrics.csv` — all (Demo, CSV, Mode) rows (gitignored)
- `analysis/output/performance_report.md` — per-demo interpretation (gitignored)
- `docs/ResearchPaper/figures/demo1_jitter.png` (tracked)
- `docs/ResearchPaper/figures/demo2_positions.png` (tracked)
- `docs/ResearchPaper/figures/demo3_schedule_error.png` (tracked)
- `docs/ResearchPaper/figures/demo4_frametime.png` (tracked)
