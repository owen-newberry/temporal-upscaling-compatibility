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

**The workaround:** render at half resolution and guess the rest of the
image by studying **previous frames**.

That guess-from-the-past trick is called **temporal upscaling**.

| Upscaler | Vendor | Ships in |
|---|---|---|
| **DLSS** | NVIDIA | RTX cards, since 2018 |
| **FSR**  | AMD    | GPU-agnostic, since 2021 |
| **TSR**  | Epic   | Built into Unreal Engine 5 |
| **XeSS** | Intel  | Arc + cross-vendor, since 2022 |

> Almost every AAA game released in the last 3 years ships with one turned on **by default**.

---

## The Problem

> Modern temporal upscaling (DLSS, FSR, TSR) requires clean software architecture and specific design to work effectively.

The guess-from-the-past trick only works if the game behaves **predictably**:

- Objects move with stable, smooth motion vectors
- Physics doesn't teleport during frame spikes
- Animations don't stutter when the frame rate changes

When games break these rules — which most do, accidentally — the upscaler's
guesses become **wrong**. The result: **smearing, ghosting, flicker, and jitter.**

And nobody writes down what *"predictable"* means for engine programmers.

---

## The Solution

Borrow four **well-known software design patterns** from game-development
literature that each fix one specific way real games violate temporal predictability.

Then **measure them** — in a real engine, on real hardware, with a
reproducible harness — so engine programmers have numbers behind the intuition.

> The contribution isn't the patterns themselves.
> It's the **framing** (upscaler compatibility) and the **measurement harness**.

---

## Four Patterns, One Goal: Temporal Predictability

| # | Pattern | What it addresses |
|---|---|---|
| 1 | **Fixed-timestep simulation** | Numerical stability of physics under frame spikes |
| 2 | **Time-based animation** | Motion/position independent of frame rate |
| 3 | **Single-writer motion authority** | Two systems fighting over one transform |
| 4 | **Workload budgeting** | Per-frame CPU cost that spikes during heavy work |

Sources: Gaffer On Games · Valve GDC networking talks · NVIDIA developer
docs · console-platform performance guidelines.

Each pattern fixes **one** failure mode that breaks the upscaler's ability
to reuse information from previous frames.

---

## The Paper

A **literature-review-and-framework** paper that:

- Surveys the four patterns across prior engineering literature
  (Gaffer On Games, Valve GDC talks, NVIDIA developer docs, console
  performance guidelines)
- Organizes them under a **single framing** — *upscaler compatibility* —
  that the source material did not explicitly use
- Argues that each pattern addresses a **specific failure mode** of
  temporal reprojection (the math behind DLSS / FSR / TSR)

The patterns aren't new. The **synthesis** is.

---

## The Methodology

To turn a literature review into **empirical evidence**, I built:

1. **Four minimal demos** in Unreal Engine 5.7 — each has a *good-citizen*
   cube (pattern applied) and a *naive* cube (pattern violated), running
   **side-by-side** in the same scene, same camera, same frame.
2. An in-engine **logging harness** (`FMotionLogger` + `FPerfLogger`)
   capturing per-frame motion, timing, and CPU work to CSV.
3. A **Python analysis pipeline** (pandas, matplotlib, scikit-learn) that
   produces a self-updating performance report at the end of every session.

> The four cubes are teaching aids. The **harness** is the real contribution —
> anyone can run it on their own engine to measure their own patterns.

---

## Live Demo

**What to watch for:**

- **Green cube** = good citizen (pattern applied correctly)
- **Red cube** = naive (pattern violated)
- Floating labels name the mode
- On-screen frame-time spikes when the naive cube misbehaves

**Fallback:** if live breaks, every behavior on-screen is also in the CSVs —
jump to the numbers on the next slides.

---

## Fixed-Timestep — Results

Same spring, same 300 ms hitch injected every few seconds.
**Variable** integrates one step per rendered frame; **Fixed** chops each
frame into 16 ms sub-steps capped by a catch-up clamp.

| Metric | Variable (naive Euler) | Fixed (sub-stepped) | Δ |
|---|---:|---:|---:|
| **Max Δ (cm)** | **305.9** | **200.0** (bounded by spring amplitude) | bounded |
| **σ Δ (cm)** | **9.31** | **5.25** | **−44%** |
| Mean Δ (cm) | 4.89 | 3.73 | — |
| Mean FPS | 119.9 | 119.9 | identical cost |

_Variable mode **overshoots the spring's physical maximum** during hitches — the simulation is producing nonsense, and those are the motion vectors the upscaler has to reproject. Fixed mode stays inside the amplitude envelope where it belongs._

<!-- Speaker note: cite Max Δ and σ Δ, NOT P95 Δ. Fixed's sub-stepping
produces controlled catch-up jumps per rendered frame, so P95 is slightly
higher (11.4 vs 9.5) but that's correct behavior, not a defect. -->

---

## Time-Based Animation — Results

Both cubes target the same sine-wave trajectory. One reads the wall clock
each frame (**Time-Based**); the other advances a counter every frame
assuming a fixed 60 FPS (**Frame-Based**).

| Metric | Frame-Based (counter) | Time-Based (wall clock) |
|---|---:|---:|
| **Max schedule deviation** | **400 cm** (4 meters off-target) | — (matches schedule by construction) |
| **Mean schedule deviation** | **165 cm** | — |
| σ Δ / frame | 3.74 cm | 3.99 cm (larger — includes honest catch-up jumps during hitches) |

_The counter-based cube **drifts up to 4 meters away from where the animation
is supposed to be** during a 400 ms hitch. The wall-clock cube catches up
to its intended position on the next frame, producing a bigger motion
vector — but that motion vector is **correct**, and the upscaler reprojects
it cleanly. Frame-based produces smaller motion vectors but **lies** about
where the object is._

<!-- Honest framing: Time-Based "zero error" is by construction (ground
truth IS the time-based formula). The meaningful claim is that
Frame-Based fails to hit its own design target by 4m during a hitch. Don't
say "Time-Based is error-free" as a standalone boast. -->

---

## Single-Writer Motion Authority — Results

Two systems both try to move the same cube every frame.
One cube has an **authority** that arbitrates; the other lets whoever writes last win.

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

Identical per-frame CPU workload (a tight busy-loop). One actor caps
itself at 2 ms per frame; the other spends whatever it needs.

| Metric | Unbudgeted | Budgeted | Δ |
|---|---:|---:|---:|
| **Mean FPS** | **22.3** | **111.7** | **5×** |
| Mean work / frame | 20.0 ms | 2.00 ms | budget held to configured target |
| Max work / frame | 20.7 ms | 2.77 ms | peak clamp honored |
| Max frame time | 116.4 ms | 98.0 ms | — |

_Same total work. Same code. Same actor. The only difference is how the
work is **spread across frames**. Budgeted mode hits its configured
target exactly — mean work lands at 2.00 ms against a configured 2 ms budget._

Stress-test variant (swarm of 8 intensive actors): **56 FPS budgeted vs 33 FPS unbudgeted** — the effect holds at scale.

---

## The Catch: Budgeting Trades Jitter for Latency

The patterns are **not free**. They defer work — they don't delete it.

Workload budgeting is the clearest example:

- A 20 ms CPU spike becomes a smooth 2 ms / frame over 10 frames
- Great for the upscaler (variance drops, FPS climbs 5×)
- But deferred work is **delayed** work

**Applied to the wrong system, you trade a visible problem for an invisible one:**

| Deferred system | Symptom |
|---|---|
| Player physics | Character walks through walls |
| Input handling | Visible input lag |
| Far-field AI | NPC frozen for 300 ms, then snaps |

**The design question isn't "should I budget?" — it's "which systems can tolerate deferral?"**

> **Predictable, not faster.**

---

## Why This Matters

**For researchers**

- First synthesis of four scattered game-dev patterns under a single
  framing — *upscaler compatibility*
- A reproducible measurement harness that future papers can extend with
  new patterns, new upscalers, new engines

**For engine programmers**

- Four concrete, code-level changes with **measured impact**
- Tooling to validate their own codebase against the same metrics

**For players**

- Fewer smearing / ghosting / stutter artifacts in games built on these
  principles

**Next step.** Three-way comparison — **TSR vs FSR vs DLSS** — across all four
patterns. Code harness is already upscaler-agnostic (plugin-switchable at
runtime); only the AMD/NVIDIA plugin installs and capture runs remain.

---

## Thanks — Questions Welcome

**Owen Newberry** · ASE Capstone · Spring 2026

- Repo: [github.com/owen-newberry/temporal-upscaling-compatibility](https://github.com/owen-newberry/temporal-upscaling-compatibility)

