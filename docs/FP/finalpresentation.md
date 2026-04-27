---
marp: true
theme: default
paginate: true
---

# Software Design Patterns for Temporal Upscaling<br>Compatibility and Performance in Real-Time Games

**Owen Newberry** · ASE Capstone · Spring 2026

---

## What is Upscaling in Video Games?

Modern games render more pixels than the GPU can afford.
**4K + 60 FPS + real-time lighting** is genuinely impossible on most hardware.

**The workaround:** render at lower resolution and guess the rest of the
image by studying **previous frames**.

That guess-from-the-past trick is called **temporal upscaling**.

| Upscaler | Vendor | Ships in |
|---|---|---|
| **DLSS** | NVIDIA | RTX cards, since 2018 |
| **FSR**  | AMD    | GPU-agnostic, since 2021 |
| **TSR**  | Epic   | Built into Unreal Engine 5 |
| **XeSS** | Intel  | Arc + cross-vendor, since 2022 |

---

## The Problem

> Modern temporal upscaling (DLSS, FSR, TSR) requires clean software architecture and specific design to work effectively.

Upscaling techniques only work if the game behaves **predictably**:

- Objects move with stable, smooth motion vectors
- Physics doesn't teleport during frame spikes
- Animations don't stutter when the frame rate changes

When games break these rules, the upscaler's
predictions become **wrong**. The result: **smearing, ghosting, flicker, and jitter.**

---

## The Solution

Identify four **software design patterns** for game-development
that each fix one specific way real games violate temporal predictability.

---

## Four Patterns, One Goal: Temporal Predictability

| # | Pattern | What it addresses |
|---|---|---|
| 1 | **Fixed-timestep simulation** | Numerical stability of physics under frame spikes |
| 2 | **Time-based animation** | Motion/position independent of frame rate |
| 3 | **Single-writer motion authority** | Two systems fighting over one transform |
| 4 | **Workload budgeting** | Per-frame CPU cost that spikes during heavy work |

Each pattern fixes one failure mode that breaks the upscaler's ability to reuse information from previous frames.

---

## The Methodology

1. Created visual demos in Unreal Engine 5.7 — each has a good design actor (pattern applied) and a bad design actor (pattern violated).
2. An in-engine logging harness (`FMotionLogger` + `FPerfLogger`) capturing per-frame motion, timing, and CPU work to CSV.
3. A Python analysis pipeline that produces a self-updating performance report at the end of every session.

---

## Numbers

- Research paper finished and published
- Presented at NKU Celebration of Student Research
  
- 100% burndown rate
- (6/6) features implemented
- (27/2) requirements implemented

---

## Live Demo

- [Recorded Demos](https://youtu.be/zN_T3va4zfw)

---

## Why This Matters

**For engine programmers**

- Four concrete, code-level changes with **measured impact**
- Tooling to validate their own codebase against the same metrics
- Better perfomance and quality -> better user experience -> more players/better player retention

**For players**

- Fewer smearing / ghosting / stutter artifacts in games built on these
  principles
- Games with high performance and quality with minimal visual issues are more enjoyable

---

## Why This Matters

- Upscaling makes games **more accessible to a wider range of components and platforms.**
- This allows more players to enjoy a game especially when it leverages upscaling well, **keeping consistency and quality while increasing performance.**

---

## What I learned

- Unreal Engine 5 basics
  - UE5 has multiple built in functions for physics and animations
  - However these functionalities don't cover all possible use cases
- TSR (upscaling) integration in Unreal Engine 5
  - TSR is native to UE5, but integration is not as simple as a checkbox

---

## What Went Well

- Scheduling and planning:
  - I was able to devote a lot of my time to this project
  - I didn't have to deviate from the outlined milestones for each sprint
- Analysis of design patterns:
  - The creation of the demo projects in Unreal Engine 5 provided great insight on the effectiveness of patterns
- Overal quality over work
  - After spending lots of time on this research and implementation, my paper is rock solid and reasoning is sound

---

## What Went Poorly

- Learning curve with Unreal Engine 5
  - Had to refresh C++ fundamentals 
  - Familiarizing myself with the structure of Unreal Engine 5
  - Tool usage took some time to learn
- Struggle to find concrete resources 
  - Beyond what NVIDIA/AMD puts out, finding informational sources took a lot of time to discern if they were of academic quality.

---

## Issues I Encountered

- Setting up my Unreal Project with Git
  - Had issues with file size with the project in the Git repository
  - Had to spend time learning what files were neccessary vs shouldnt be tracked/could be regenerated on an individual machine
- Issues with Unreal Engine behavior with rendering mode
  - Some functions would work only in Play In Editor, not the Standalone view
  - Play In Editor limits the performance and could cause issues with data validity
  - Address with debugging and leveraged AI to figure out what changes needed to be made

---

## How AI Improved this Project

- **Research:** Used AI to help explain different topics for deep understanding, find resources, etc.
- **Debugging:** Used an AI agent to help solve issue with the codebase, whether simple C++ errors or UE5 specific issues
- **Implementation:** Experimented with using AI agents to write parts of the code, like the Python data analysis pipeline, some C++ classes for UE5 usage

---

## Thanks

**Owen Newberry** · ASE Capstone · Spring 2026

- Repo: [github.com/owen-newberry/temporal-upscaling-compatibility](https://github.com/owen-newberry/temporal-upscaling-compatibility)
- Paper: [Software Design Patterns for Temporal Upscaling Compatibility and Performance in Real-Time Games](https://github.com/owen-newberry/temporal-upscaling-compatibility/blob/main/docs/ResearchPaper/paper.pdf)
- [Video Demonstration](https://github.com/owen-newberry/temporal-upscaling-compatibility/blob/main/docs/FP/demos.mp4) (download)
- [Video Demonstration](https://youtu.be/zN_T3va4zfw) (YouTube, no download)
