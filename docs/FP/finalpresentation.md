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

## Live Demo

- [Record Demos]()

---

## Fixed-Timestep — Results

Same spring position system, same 300 ms hitch injected every few seconds.

| Metric | Variable (naive Euler) | Fixed (sub-stepped) | Δ |
|---|---:|---:|---:|
| **Max Δ (cm)** | **305.9** | **200.0** (bounded by spring amplitude) | bounded |
| **σ Δ (cm)** | **9.31** | **5.25** | **−44%** |
| Mean Δ (cm) | 4.89 | 3.73 | — |
| Mean FPS | 119.9 | 119.9 | identical cost |

---

## Time-Based Animation — Results

Both cubes target the same sine-wave trajectory. One reads the wall clock
each frame (**Time-Based**); the other advances a counter every frame (**Frame-Based**).

| Metric | Frame-Based (counter) | Time-Based (wall clock) |
|---|---:|---:|
| **Max schedule deviation** | **400 cm** (4 meters off-target) | — (matches schedule by construction) |
| **Mean schedule deviation** | **165 cm** | — |
| σ Δ / frame | 3.74 cm | 3.99 cm (larger — includes honest catch-up jumps during hitches) |

---

## Single-Writer Motion Authority — Results

Two systems both try to move the same cube every frame. One cube has an authority that controls movement; the other lets whoever writes last win.

| Metric | Direct (last-write-wins) | Authority (arbitrated) | Δ |
|---|---:|---:|---:|
| Mean Δ / frame | 4.38 cm | 3.36 cm | primary only |
| **σ Δ / frame** | **3.36 cm** | **1.92 cm** | **−43%** |
| **P95 Δ** | **9.92 cm** | **5.22 cm** | **−47%** |
| Max Δ | 80.8 cm | 57.6 cm | −29% |
| Mean FPS | 119.8 | 119.8 | identical cost |

_Identical scene, identical camera, identical rival-writer. The only difference is whether the second writer goes through the authority or writes the transform directly._

---

## Workload Budgeting — Results

Identical per-frame CPU workload (a tight busy-loop). One actor caps itself at 2 ms per frame; the other spends whatever it needs.

| Metric | Unbudgeted | Budgeted | Δ |
|---|---:|---:|---:|
| **Mean FPS** | **22.3** | **111.7** | **5×** |
| Mean work / frame | 20.0 ms | 2.00 ms | budget held to configured target |
| Max work / frame | 20.7 ms | 2.77 ms | peak clamp honored |
| Max frame time | 116.4 ms | 98.0 ms | — |

The difference is how the work is spread across frames. Budgeted mode hits a configured target exactly, mean work lands at 2.00 ms against a configured 2 ms budget.

---

## The Catch: Budgeting Trades Jitter for Latency

Most of these patterns aren't without some drawbacks/limitations. They defer work, they don't delete it.

Workload budgeting is the clearest example:

- A 20 ms CPU spike becomes a smooth 2 ms / frame over 10 frames
- Great for the upscaler (variance drops, FPS climbs 5×)
- But deferred work is **delayed** work

**Applied to the wrong system, you trade a visible problem for an invisible one:**

---

## The Catch: Budgeting Trades Jitter for Latency

| Deferred system | Symptom |
|---|---|
| Player physics | Character walks through walls |
| Input handling | Visible input lag |
| Far-field AI | NPC frozen for 300 ms, then snaps |

**The design question isn't "should I budget?" — it's "which systems can tolerate deferral?" "Which systems are low priority?"**

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

## Thanks

**Owen Newberry** · ASE Capstone · Spring 2026

- Repo: [github.com/owen-newberry/temporal-upscaling-compatibility](https://github.com/owen-newberry/temporal-upscaling-compatibility)
- Paper: [Software Design Patterns for Temporal Upscaling Compatibility and Performance in Real-Time Games](https://github.com/owen-newberry/temporal-upscaling-compatibility/blob/main/docs/ResearchPaper/paper.pdf)
- [Video Demonstration](https://github.com/owen-newberry/temporal-upscaling-compatibility/blob/main/docs/FP/demos.mp4) (download)
- [Video Demonstration]() (YouTube, no download)
