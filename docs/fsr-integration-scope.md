# FSR Integration — Scope & Status

Goal for F5 (and its FSR sub-feature): add runtime-switchable FSR alongside
TSR (built-in) and DLSS (if/when NVIDIA plugin is available) so the paper can
compare the patterns' behaviour under each upscaler.

## Status as of 2026-04-22

**Code-side integration: DONE.**

- `DemosUpscaler` namespace (`UpscalerControl.h/.cpp`) detects the active
  upscaler at runtime via public CVars, and exposes a `demos.Upscaler`
  console command for runtime switching (TSR / FSR / DLSS / TAA / FXAA / None).
- `FPerfLogger` now logs the active upscaler on every sample, so every
  CSV is self-describing.
- The code is plugin-agnostic: if the FSR3 / DLSS plugins aren't installed
  the switch commands are no-ops (with a warning), and the CSV will show
  the engine's active AA method (typically TSR) instead. No compile-time
  dependency on third-party plugins.

**Plugin install: USER ACTION REQUIRED.**

To actually run FSR captures, the AMD FidelityFX FSR plugin must be
installed — see Step 1 below.

**Capture + analysis sweep: PLANNED POST-PRESENTATION.**

Once the plugin is installed, capture time is ~2 hours and analysis
updates are ~1 hour (Step 5 below is unchanged from the original scope).

## What's actually involved

### 1. Install the plugin (~15 min, trivial unless something breaks)

AMD ships FSR as a UE marketplace plugin ("FidelityFX Super Resolution 2/3
for Unreal Engine") compatible with UE 5.x. Two install options:

- **Marketplace** (recommended): install via Epic Games Launcher → plugin
  auto-dropped into the engine's `Plugins/Marketplace/` folder.
- **Manual**: download AMD's release zip from GPUOpen, drop into
  `demos/Plugins/FSR2/` (or `FSR3/`), re-run `demos.uproject` → it picks
  it up on next build.

Enable in `demos.uproject`:

```json
"Plugins": [
    { "Name": "FSR2Temporal", "Enabled": true }
]
```

The first build after enabling will add ~2-3 min of cook time — no code
changes needed to the project's own modules.

### 2. Switch upscaler at runtime — DONE

**Status:** implemented as a `demos.Upscaler` console command.

Usage (from the UE console — backtick key in PIE):

```
demos.Upscaler           // print current active upscaler
demos.Upscaler TSR       // force TSR
demos.Upscaler FSR       // force FSR3 (if plugin loaded)
demos.Upscaler DLSS      // force DLSS (if plugin loaded)
demos.Upscaler TAA       // fall back to classic TAA
demos.Upscaler None      // no antialiasing
```

If the requested plugin isn't loaded, the command logs a warning and
leaves the previous upscaler active. This means the command works in
every project state — no compile errors, no missing symbols — which
makes the implementation safe to keep in `main` even before plugin
install.

### 3. Log the active upscaler per capture — DONE

`FPerfLogger` now writes an `Upscaler` column on every sample, read
from `DemosUpscaler::GetActiveName()`. Analyzers that group by
`(Demo, Mode, Upscaler)` will automatically bucket TSR/FSR/DLSS data
once captures with different upscalers exist.

### 4. Capture the paper data (~2 hours — the actual work)

For each of the four demos (plus combined-stress):

- Boot the level, set upscaler to TSR, run 60s capture.
- Switch to FSR, run 60s capture.
- (Switch to DLSS, run 60s capture — if installed.)

That's 4 demos × 3 upscalers × 1 minute = ~12 minutes of play time per run,
plus overhead. Plan 2 hours total for a clean dataset including re-runs
for any outliers.

### 5. `aggregate_metrics.py` and `generate_report.py` updates (~1 hour)

- `aggregate_metrics.py`: group by `(Demo, Mode, ActiveUpscaler)` instead of
  just `(Demo, Mode)`.
- `generate_report.py`: emit one subtable per demo with an "Upscaler" column
  so TSR / FSR / DLSS sit side-by-side.

Both scripts already iterate over CSVs and group by columns; adding one
more dimension is mechanical.

### 6. (Polish only) artifact screenshots (~2-3 hours)

The paper benefits from visual examples of the artifacts each upscaler
produces under each anti-pattern (ghosting, smearing, under-sampling).
Workflow:

- Trigger a known-bad frame (e.g. during a spike in Demo 2).
- `HighResShot 2` console command → screenshot.
- Repeat per upscaler → side-by-side figure in the paper.

This is polish, not required for the comparison tables.

## Risks / what could derail the estimate

- **Plugin version mismatch.** FSR2 plugin built against UE 5.3 won't
  necessarily load in 5.7. Need to check AMD's repo for a 5.6/5.7-compatible
  build, or build from source.
- **Shader recompile on first run.** 10-30 min of blocked editor time.
  Expected, not a blocker.
- **DLSS needs an NVIDIA GPU.** If the capture machine is AMD, drop DLSS
  from the comparison and scope the paper to TSR vs FSR (still a valid
  comparison — both are cross-vendor).
- **Capturing with OBS + upscaling.** Screen recording adds its own
  frame-time variance. Pure in-engine logging (`FPerfLogger`) is the
  source of truth; OBS is just for demo video in slides.

## Recommendation for tomorrow

The **code harness is already in place** — the `demos.Upscaler` command
and the CSV `Upscaler` column mean capturing with FSR is a matter of
installing the plugin, running the command, and re-capturing. You can
truthfully say in the talk:

> "The measurement pipeline is upscaler-agnostic. Switching to FSR or
> DLSS is one console command; every CSV self-identifies the active
> upscaler. Capturing the three-way comparison is future work — but
> the harness is ready to receive it."

If anyone asks for specifics, point at `UpscalerControl.h` and the
`Upscaler` column in a `PerformanceCapture_*.csv` — that's the
demonstrable part.

## Quick-start once the plugin is installed

1. Download AMD's FidelityFX FSR 3 plugin for UE 5.x from
   https://gpuopen.com/fidelityfx-superresolution-3/ (or the UE
   Marketplace listing).
2. Extract into `demos/Plugins/FFXFSR3/` (or engine-wide
   `Plugins/Marketplace/`).
3. Add to `demos.uproject`:
   ```json
   { "Name": "FFXFSR3", "Enabled": true }
   ```
4. Rebuild the project (Live Coding won't pick up new plugins).
5. In PIE, open the console and run: `demos.Upscaler FSR`. Check the
   log for `[DemosUpscaler] SetUpscaler(FSR3) -> applied. Active now:
   FSR3`.
6. Run any demo capture as usual — the CSV's `Upscaler` column will
   read `FSR3` for every row captured while it's active.
