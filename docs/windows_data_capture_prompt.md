# Windows Data-Capture Prompt — Final Paper Numbers and Plots

Paste everything below this line into a fresh Cursor / Claude / GPT chat on the
Windows machine. The agent should read this once, then execute the plan.

---

## Context

You are working in the `temporal-upscaling-compatibility` repository on a
Windows machine. This repo contains the Unreal Engine 5.7 prototype for the
honors thesis paper *"Software Design Patterns for Temporal Upscaling
Compatibility and Performance in Real-Time Games"*. The paper itself lives in
a separate repo (`honor2026`) and is already drafted on a Mac. Your job is to
produce the final dataset that fills the two `[TBD]` tables and the four
figure placeholders in the paper.

You do **not** need to edit any LaTeX. You produce numbers and PNGs; the
authors will paste them into the paper repo.

### What the paper says happens in each demo

The paper describes four minimal Unreal Engine demos. Each demo is one level
containing two visually identical cubes — one with the design pattern
applied, one with the pattern violated — under a fixed camera. The only
experimental variable per demo is the pattern toggle.

| Demo | Pattern | Modes (bad / good) | Implementation file |
|------|---------|--------------------|---------------------|
| 1 | Single-Writer Motion Authority | `Direct` / `Authority` | `MotionAuthorityActor.cpp` |
| 2 | Fixed-Timestep Simulation | `Variable` / `Fixed` | `TimestepActor.cpp` |
| 3 | Time-Based Animation | `FrameBased` / `TimeBased` | `MotionModelActor.cpp` |
| 4 | Bounded Workload Budgeting | `Unbudgeted` / `Budgeted` | `WorkloadActor.cpp` |

The methodology is upstream — per-frame world-space position deltas are a
software-side proxy for screen-space motion vectors. You do **not** need to
measure pixel-space motion vectors.

### Logging is already wired up

The repo already has all the logging infrastructure you need. Do not rebuild
it from scratch.

- `FMotionLogger` (in `demos/Source/demos/MotionLogger.{h,cpp}`) emits one row
  per actor per frame to `demos/Saved/Logs/MotionCapture_<timestamp>.csv`.
  Schema: `Frame,TimeSeconds,LevelName,ActorName,Mode,PosX,PosY,PosZ,FrameDeltaCm,PositionErrorCm,FrameWorkMs`.
- `FPerfLogger` (in `demos/Source/demos/PerfLogger.{h,cpp}`) emits one row per
  sample (default 10 Hz) to `demos/Saved/Logs/PerformanceCapture_<timestamp>.csv`.
  Schema: `TimeSeconds,LevelName,Upscaler,FrameTimeMs,GPUFrameMs,UsedPhysicalMB,PeakPhysicalMB,UsedVirtualMB`.
- Both loggers are owned by `UDemosGameInstance`, which is bound in
  `demos/Config/DefaultEngine.ini` via `GameInstanceClass=/Script/demos.DemosGameInstance`.
  They reset on session start and flush on shutdown.
- `WorkloadActor.cpp` already calls `FMotionLogger::Get().LogRow(...,
  FrameWorkMs)` so Demo 4's per-actor work cost lands in the CSV.

### Analysis pipeline is also already there

- `analysis/aggregate_metrics.py` walks every `MotionCapture_*.csv` (skipping
  files prefixed `_archive_`), classifies each row by `Mode` into one of the
  four demos, and produces `analysis/output/summary_metrics.csv` with these
  columns per `(Demo, CSV, Mode)` row: `Count, MeanDelta, StdDelta,
  MedianDelta, P95Delta, MaxDelta, MeanError, MaxError, MeanWorkMs,
  MaxWorkMs, P95WorkMs, MeanFPS, StdFrameTimeMs, MaxFrameTimeMs, P1_FPS,
  P01_FPS, Avg1LowFPS, Avg01LowFPS`. FPS is derived from `diff(TimeSeconds)`
  inside each actor's row stream.
- `analysis/analyze_motion.py` produces per-CSV plots: `_positions.png`,
  `_jitter.png`, `_distribution.png`, and (for Demo 3 only)
  `_position_error.png`. Run it once per CSV.
- `analysis/generate_report.py` writes a Markdown report at
  `analysis/output/performance_report.md` with per-demo interpretation
  paragraphs.

The Python deps are: `pandas`, `numpy`, `matplotlib`, `scipy`, `scikit-learn`.

## What to produce

### Two tables of numbers (paper's Demo 1 and Demo 4 results)

**Demo 1 results table** — for each of `Direct` and `Authority`:

- `Mean Delta (cm)`         — `MeanDelta` from summary
- `Std Delta (cm)`          — `StdDelta`
- `P95 Delta (cm)`          — `P95Delta`
- `Max Delta (cm)`          — `MaxDelta`
- `Mean FPS`                — `MeanFPS`

Plus the percentage change between modes (`Authority - Direct) / Direct *
100`) for the three variance/tail rows (Std, P95, Max). Mean FPS should be
near-identical between modes (the paper text claims "no measurable mean-FPS
penalty").

**Demo 4 results table** — for each of `Unbudgeted` and `Budgeted`:

- `Mean work / frame (ms)`  — `MeanWorkMs` from summary
- `Max work / frame (ms)`   — `MaxWorkMs`
- `Max frame time (ms)`     — `MaxFrameTimeMs` (this is the wall-clock frame
  time, **not** the per-actor work cost — this row exposes the budgeting
  pattern's effect on the *frame*, not just the actor)
- `Mean FPS`                — `MeanFPS`

Plus the multiplicative speed-up `Budgeted_MeanFPS / Unbudgeted_MeanFPS`
(rendered as e.g. `1.43x`).

### Four PNG plots (paper's figures)

Each must be generated cleanly (legible axes, legend, title) at sufficient
resolution to print at column width (~3.5 in / 2400 px wide is fine).

| File name (final) | What it shows |
|-------------------|---------------|
| `demo1_jitter.png` | `FrameDeltaCm` over `TimeSeconds`, with both `Direct` and `Authority` actors overlaid as separate series. |
| `demo2_positions.png` | `PosX` (or whichever axis the spring oscillates on — check the actor) over `TimeSeconds`, both `Variable` and `Fixed` actors overlaid. The 300 ms hitches should be visible as discontinuities, with Variable flying past the spring's amplitude. |
| `demo3_schedule_error.png` | `PositionErrorCm` over `TimeSeconds`, both `FrameBased` and `TimeBased` actors overlaid. Stalls should be visible as long flat lines on FrameBased; TimeBased should be ~0 throughout. |
| `demo4_frametime.png` | `FrameWorkMs` over `TimeSeconds`, both `Unbudgeted` and `Budgeted` actors overlaid. Unbudgeted should show large spikes; Budgeted should be capped near `BudgetMs`. |

`analyze_motion.py` already produces close-to-correct plots
(`_jitter.png`, `_positions.png`, `_position_error.png`); you may need to
either rename and curate the best one of each, or write a small custom
plotting script that produces exactly the four figures above with consistent
styling. Either approach is fine.

## Run protocol

1. **Build the project** in Visual Studio (`Development Editor`,
   `Win64`) and open `demos.uproject` in UE 5.7. Confirm the project loads
   without missing-module errors.
2. **Confirm the GameInstance binding** is still active by checking that
   `[/Script/EngineSettings.GameMapsSettings] GameInstanceClass=/Script/demos.DemosGameInstance`
   is present in `demos/Config/DefaultEngine.ini`. Without this, neither
   logger will fire.
3. **Run each demo in Standalone, not Play-in-Editor.** PIE has per-frame
   editor overhead that distorts FPS metrics. Use Editor Preferences →
   Play → New Editor Window (Standalone), or run from the launcher with
   `-game`.
4. **Each session: target ~5,000–10,000 logged frames per demo** (about 1–3
   minutes of capture at 60 Hz). The CSV grows large but the analysis
   pipeline handles it. Do not re-use a single session for multiple demos
   — load a fresh level per demo so each level's CSV is clean.
5. **Use the project's default temporal upscaler (TSR).** Cross-upscaler
   sweeps (TAAU, FSR via the GPUOpen plugin) are *not required* for the
   paper's current claims. If you have time at the end and want to
   strengthen the paper's upscaler-toggle claim, capture each demo a second
   time with `UpscalerControl::Set("TAAU")` (and `"FSR"` if the FSR plugin is
   present) and report whether the per-frame-Δ profile meaningfully
   changes — the prediction is *no*, since the patterns operate upstream
   of the upscaler.
6. **After each session**, the new CSV(s) appear in `demos/Saved/Logs/`.
   Don't move or rename them yet — let the analysis pipeline see them.

## Verification: does the existing presentation data already cover this?

Before re-running, **check the existing CSVs** in `demos/Saved/Logs/` from
the presentation. Run:

```powershell
python analysis/aggregate_metrics.py
```

and inspect `analysis/output/summary_metrics.csv`. If you find rows for
*all four demos* with non-NaN `MeanWorkMs` for Demo 4 and reasonable
`MeanFPS` values everywhere, you can use those numbers directly and skip
re-running. If `MeanWorkMs` is NaN for Demo 4, or any demo has only one mode
(missing the A/B pair), or `Count` is suspiciously small, re-run that demo.

The paper currently uses the presentation's Demo 2 and Demo 3 numbers as a
reference. If your fresh runs differ from these by more than ~5%, flag it
and report both sets of numbers — the authors will decide which to keep:

| Demo 2 (Variable / Fixed) | Mean Δ | σΔ | P95 Δ | Max Δ |
|---|---|---|---|---|
| Reference | 26.9 / 13.6 | 32.3 / 12.2 | 52.6 / 23.0 | 544.7 / 198.0 |

| Demo 3 (FrameBased / TimeBased) | Mean Δ | σΔ | Max Δ | Mean sched err | Max sched err |
|---|---|---|---|---|---|
| Reference | 6.40 / 11.83 | 3.42 / 5.93 | 10.47 / 78.29 | 162.0 / 0.00 | 400.0 / 0.00 |

Demo 1 and Demo 4 reference numbers do **not** exist yet — those are what
this exercise is producing.

## Code changes you may need to make

The current code looks complete based on a recent inspection of the Mac-side
clone, but you should verify these things on your Windows checkout because
the branches may diverge:

1. **`MotionLogger.h` schema includes `FrameWorkMs`.** If the header still
   has the older 10-column header without `FrameWorkMs`, update both the
   header string in `MotionLogger.cpp::Init()` and the `LogRow` printf so
   Demo 4 captures are usable. (As of the last Mac-side check this was
   already correct.)
2. **`WorkloadActor.cpp` actually passes `FrameWorkMs` to `LogRow`.** The
   busy-loop's wall-clock cost should be measured with
   `FPlatformTime::Seconds()` and forwarded as the eighth argument. (As of
   the last Mac-side check this was already correct around line 175–182.)
3. **`UDemosGameInstance::Init()` initializes both loggers and registers a
   ticker that calls `FPerfLogger::Get().SampleOnce(World)` periodically.**
   If `PerformanceCapture_*.csv` files are missing from `Saved/Logs/` after
   a Standalone session, this is the likely culprit.
4. **`PerfSampleIntervalSeconds` defaults to 0.1 (10 Hz).** That's fine for
   memory and GPU snapshots; per-frame FPS lows are derived from
   `MotionLogger`'s per-actor `TimeSeconds` diffs in
   `aggregate_metrics.compute_frame_times`, so you do *not* need to bump
   the perf-sample rate to per-frame.

If any of those four checks fail, fix the code, recompile, and re-run.

## Final analysis steps

After all four demos have a fresh CSV in `demos/Saved/Logs/`:

```powershell
python analysis/aggregate_metrics.py
python analysis/generate_report.py
# For each MotionCapture_*.csv individually:
python analysis/analyze_motion.py demos/Saved/Logs/MotionCapture_<timestamp>.csv
```

Then:

1. Open `analysis/output/summary_metrics.csv` and lift the Demo 1 and Demo 4
   rows. Format them as the two tables described under **What to produce**.
2. Open `analysis/output/performance_report.md` and skim the per-demo
   paragraphs for sanity — flag anything that contradicts the paper's
   interpretation (e.g. if Demo 1 *Authority* somehow has *higher* σΔ than
   Demo 1 *Direct*, that's a bug in the actor or the scenario, not a real
   result).
3. Pick the four PNGs from `analysis/output/` that best correspond to the
   paper's four figures (see filename mapping above), and **rename them
   exactly** to `demo1_jitter.png`, `demo2_positions.png`,
   `demo3_schedule_error.png`, `demo4_frametime.png`. If
   `analyze_motion.py`'s default plots aren't styled cleanly enough for
   print, write a small `make_paper_figures.py` that pulls the relevant
   columns from each CSV and produces these four files directly with
   `matplotlib`.

## What to send back to the authors

Produce a single message with:

1. The two tables (Demo 1, Demo 4) with the exact numeric values, formatted
   as you would paste them into LaTeX. Two decimal places is fine; one is
   fine for cm-scale values.
2. The four renamed PNG files attached or pushed to a branch in the repo
   with a clear commit message (e.g. `chore: capture final paper figures
   from clean Standalone runs`).
3. Any deviations from the reference numbers in the table above for Demo 2
   and Demo 3, with a short note about why (different upscaler? different
   hitch interval? different session length?).
4. The session metadata: machine specs (CPU, GPU, RAM, OS build), UE 5.7
   build hash, upscaler used, and approximate session length per demo. This
   lets the authors include a one-paragraph "Hardware and capture
   environment" note in the paper if they want.

That's it. Don't push to the `honor2026` repo or modify any LaTeX. Keep all
changes inside `temporal-upscaling-compatibility`.
