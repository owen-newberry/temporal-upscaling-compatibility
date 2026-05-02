# ase-capstone

## Project Description

This capstone investigates what makes games actually compatible with modern GPU upscaling (DLSS, FSR) and frame generation. The code lives in a small Unreal Engine 5 C++ project plus a Python analysis pipeline: this repository is a research prototype.

The problem is straightforward: upscaling tech like DLSS and FSR are everywhere now, but there's surprisingly little concrete guidance on how to structure your code so they actually work well. Student and indie projects constantly break temporal reconstruction without realizing it—bad frame pacing, inconsistent motion vectors, unstable simulation logic.

**Repository layout (high level):**

| Path | Role |
|------|------|
| `demos/` | UE5 `demos` project: four pattern validation maps (`L_MotionAuthority`, `L_FixedTimestep`, `L_TimeBasedAnim`, workload `L_WorkloadBudgeted` / `L_WorkloadUnbudgeted`) plus a combined-stress level (`L_CombinedStress`), C++ actors, and CSV log output under `demos/Saved/Logs/`. |
| `analysis/` | Python: aggregate motion/perf CSVs, optional motion-jitter charts, and Markdown performance reports. |
| `docs/` | Research paper, presentations, and pattern quick-reference. |
| `videos/` | Screen captures of each pattern demo. |

The UE project demonstrates three architectural patterns (fixed-timestep integration, per-frame CPU workload budgeting, and temporal coherence via motion authority and time-based motion) and measures their effects with logged captures and reports.

Deliverables: research paper, working UE5 prototype in `demos/`, and performance data and generated reports in `analysis/output/`.

---

## Problem Domain

Temporal upscaling (DLSS, FSR) and frame generation are pretty much standard now for hitting high frame rates, but they make assumptions about how your code works:

- **Stable frame pacing** — consistent frame-to-frame timing
- **Coherent motion vectors** — predictable object movement
- **Temporal continuity** — deterministic simulation that doesn't change behavior at different frame rates

Break these assumptions and you get ghosting, smearing, stuttering, and other visual garbage. Common culprits: tying game logic directly to frame rate, having multiple systems fight over object transforms, or using frame counters instead of actual time.

Questions this project answers:

- How do specific architectural patterns support upscaler compatibility?
- What's the measurable performance impact of implementing these patterns?
- Can you validate this stuff in a real game context instead of just theory?

I'm implementing patterns identified through research to demonstrate their practical application.

---

## Tech stack

| Area | Technologies |
|------|----------------|
| **Game prototype** | Unreal Engine **5.7**, C++ (`demos` game module), editor-authored levels and demo maps |
| **Temporal upscaling** | Unreal **TSR** by default; optional **DLSS** / **FSR** / **XeSS** when corresponding plugins are available (`DemosUpscaler`) |
| **Capture & logging** | In-engine CSV exports (`FMotionLogger`, `FPerfLogger`) under `demos/Saved/Logs/` |
| **Offline analysis** | **Python 3** — **pandas**, **NumPy**; **Matplotlib** for charts and paper figures; **SciPy** and **scikit-learn** for motion analysis helpers; `analysis/run_analysis.ps1` for scripted runs |
| **Docs & presentations** | Markdown (reports, research docs); **Marp** for slide PDFs (`docs/FP/`) |

---

## Features & Requirements

### Feature 1: Fixed-Timestep Simulation

| ID | User Story |
|----|------------|
| F1.1 | As a developer, I should separate simulation integration from the variable render frame (fixed-rate sub-steps with an accumulator; render `Tick` reads the latest integrated state). |
| F1.2 | As a system, fixed-timestep mode should produce stable, replayable oscillator behavior; variable mode shows divergence under the same inputs when spikes are injected. |
| F1.3 | As a developer, I should cap simulation catch-up per frame (`MaxCatchUpSeconds`) so hitches do not advance unbounded sub-steps in a single display frame. |
| F1.4 | As a developer, I should validate motion stability via `MotionLogger` CSV exports and `analysis/analyze_motion.py` / `aggregate_metrics.py` (position delta and jitter, not a separate in-editor MV debug mode). |

### Feature 2: Workload Budgeting (CPU work per frame)

| ID | User Story |
|----|------------|
| F2.1 | As a developer, I should implement a per-actor, per-frame wall-clock budget (`BudgetMs` on `AWorkloadActor`). |
| F2.2 | As a system, I should time-slice work: a queue of identical synthetic tasks runs until the budget is exhausted; the rest is deferred. |
| F2.3 | As a system, deferred tasks should carry over to later frames (there is a single task queue, not a multi-priority job scheduler with named importance levels). |
| F2.4 | As a developer, I should compare budgeted vs unbudgeted swarms side-by-side (`AWorkloadSwarmCoordinator`) to show frame-time impact at scale. |
| F2.5 | As a developer, I should log per-actor work time in `FrameWorkMs` (MotionLogger) to correlate motion error with CPU load. |
| F2.6 | As a developer, I should quantify frame-time and delta stability in `analysis/aggregate_metrics.py` (e.g. work-time columns, FPS spread). |

### Feature 3: Temporal Coherence (Authority & time-based motion)

| ID | User Story |
|----|------------|
| F3.1 | As a developer, I should implement single-writer motion: one authority commits the transform; a second “phantom” path illustrates the failure mode in Direct mode. |
| F3.2 | As a system, the demo should show coherent per-frame position deltas in Authority mode vs conflicted motion in Direct mode (again validated via capture + metrics). |
| F3.3 | As a developer, I should contrast time-based motion (`WorldTime` sine) with frame-based steps that assume a fixed FPS on `AMotionModelActor`. |
| F3.4 | As a system, the time-based path should stay consistent under simulated hitches; frame-based motion should drift. |

### Feature 4: Performance Measurement and Export

| ID | User Story |
|----|------------|
| F4.1 | As a developer, I should collect per-frame / per-actor data via `FMotionLogger` (positions, mode, `FrameWorkMs` where applicable). |
| F4.2 | As a developer, I should sample frame time, RHI GPU frame cycles, and process memory via `FPerfLogger`. |
| F4.3 | As a developer, I should log memory stats (`UsedPhysicalMB`, etc.) alongside timing in `PerformanceCapture_*.csv`. |
| F4.4 | As a developer, I should compute 1% low and 0.1% low FPS in `analysis/aggregate_metrics.py`. |
| F4.5 | As a developer, I should export captures as CSV under `demos/Saved/Logs/` for off-line analysis. |
| F4.6 | As a system, `analysis/generate_report.py` should build `analysis/output/performance_report.md` from aggregated runs. |

### Feature 5: Temporal Upscaling Modes and Validation

| ID | User Story |
|----|------------|
| F5.1 | As a developer, I should run the project with Unreal’s TSR path by default. |
| F5.2 | As a system, I should **attribute** `PerfLogger` samples to a named upscaler so analysis can compare impact. |
| F5.3 | As a developer, I should support qualitative and metric validation (recorded `videos/`, motion captures, and report text), not a built-in image-quality ground-truth suite in-engine. |

### Feature 6: Prototype & Stress Context

| ID | User Story |
|----|------------|
| F6.1 | As a developer, I should use isolated, editor-authored levels and optional coordinators instead of a single open-world map. |
| F6.2 | As a developer, I should use scripted / oscillating actors and cameras to exhibit each pattern. |
| F6.3 | As a developer, I should use synthetic CPU workload actors and swarms to stand in for variable-cost “AI” work. |
| F6.4 | As a developer, I should use in-level visuals (e.g. labels, materials, stress layout in `AStressTestCoordinator`) to make hitch behavior obvious on screen and on capture. |

---

## Schedule & Milestones

**Two-part capstone:**
1. **Research paper** — GPU evolution, engine case studies, and software patterns for upscaler-friendly game architecture
2. **Code + analysis** — UE5 `demos/` prototype, CSV captures, and Python `analysis/` pipeline  

The 10-week outline below matches the **weekly progress** section of the final project self-evaluation rubric (submitted to Canvas; same week-by-week text as the course artifact).

**Week 1**
- Survey academic and industry literature on GPU evolution
- Establish architectural inflection points in gaming hardware
- Document traditional rendering pipelines vs. AI-integrated approaches
- Read Nvidia Blackwell and Ada Lovelace technical documentation
- Begin drafting introduction section

**Week 2**
- Unreal Engine 5 (Lumen/Nanite architecture) documentation deep-dive
- Unity’s ECS/DOTS framework for parallel processing analysis
- DLSS/FSR technical implementation and SDK documentation
- Write methodology section and begin analysis sections

**Week 3**
- Nvidia Blackwell technical documentation and SDK analysis
- Ray tracing vs. rasterization performance implications research
- Neural rendering and frame generation impact on software design
- Benchmark analysis and performance comparisons
- Write findings and analysis sections
- Draft conclusions

**Week 4**
- Identify recurring engineering patterns from research
- Connect hardware capabilities to software design decisions
- Complete all sections of research paper
- Comprehensive revision and editing
- Finalize bibliography and citations
- Proofread and format paper

**Week 5** — 
- Set up Unreal Engine project structure and version control
- Develop fixed-timestep simulation (F1.1–F1.3, F4.1)
- Adjust demo levels to better highlight upscaling-related behavior (F6.1–F6.2)

**Week 6** — 
- Continue workload budgeting system (F2.1–F2.4)
- Begin implementing temporal coherence patterns: motion authority, time-based vs frame-based motion (F3.1–F3.3)

**Week 7** — 
- Test all patterns with upscaling enabled (F5.1; optional DLSS/FSR/XeSS via `DemosUpscaler` when plugins are available)
- Refine demo levels and documentation from test results (F1.4, F3.4, F6.1–F6.2)
- Record active upscaler in performance captures where applicable (F5.2)

**Week 8** — 
- Exercise DLSS/FSR (or TSR) in project settings and `DemosUpscaler` (F5.1, F5.2)
- Run comprehensive performance tests on all patterns
- Gather and analyze performance and visual quality data; qualitative review alongside metrics (F1.4, F2.5–F2.6, F4.1–F4.6, F5.3)

**Week 9** — 
- Add environmental / in-level elements for temporal testing (F6.4, combined-stress and visuals)
- Improve data collection scripts (`analysis/`, logging)
- Update research paper with empirical results

**Week 10** —  
- Prepare and record demonstration video (`docs/FP/demos.mp4` and pattern clips under `videos/`)
- Create Marp presentation PDF for final presentation (`docs/FP/finalpresentation.pdf`)
- Ensure all deliverables are accessible on GitHub and Canvas; proofread and format materials

**Final deliverables:**
- Research paper and updated empirical sections
- UE5 `demos/` project and pattern maps
- Performance CSVs, `analysis/output/` aggregates and reports, demo video, final presentation PDF
- Learning-with-AI Marp PDFs and peer evaluation (see course submission checklist)

---