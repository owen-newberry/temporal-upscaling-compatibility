# Workload Budgeting: Trade-offs & Limitations

This note captures the honest trade-offs of the workload-budgeting pattern
demonstrated in Demo 4 (`L_WorkloadBudget`). Use it as source material for
the Discussion / Limitations section of the final paper so the pattern is
pitched responsibly rather than as a free lunch.

## TL;DR

Workload budgeting caps *per-frame CPU cost* for a class of tasks, trading
worst-case frame-time spikes for best-case per-task latency. It only works
when the system designer can correctly classify which work is
**non-urgent, batch-friendly, and eventually-consistent**. Misapplied, it
replaces a visible problem (hitches) with an invisible one (stale or
incorrect simulation state).

## What budgeting is actually good for

- Asset / texture streaming
- Far-field AI updates (path smoothing, perception ticks)
- Decorative physics / particle updates
- Background analytics & telemetry flush
- Pool maintenance, GC-like sweeps
- Anything with loose latency requirements where "a few frames late" is fine

## What budgeting breaks when misapplied

### 1. Deferred work accumulates into latency

If the incoming rate exceeds the budget, the deferred queue grows. A task
enqueued on frame 100 may not execute until frame 180. In our demo we cap
the queue at `TasksPerFrame * 10` and silently drop older entries — in
production that drop shows up as "the NPC never reacted" or "the texture
never streamed in."

### 2. Stale state / correctness bugs

The budget is on wall time, not on task importance. Critical work (a
physics tick for the locally-owned player, input handling, network ack)
gets deferred alongside cosmetic work unless you prioritize explicitly.
Symptoms:

- NPCs frozen for 300 ms and then snapping
- Physics rigid bodies that should collide this frame not resolving until
  the next
- Input lag when input handlers share a queue with background work

### 3. Priority is hard; FIFO is usually wrong

The demo uses a plain `TArray` (FIFO). Real systems need priority queues,
dependency tags, and anti-starvation policies. A naive priority scheme can
starve low-priority tasks indefinitely (far-field AI that never gets a
frame under load).

### 4. Budget tuning is brittle / hardware-dependent

A 2 ms budget is ~5 tasks on a Steam Deck and ~50 on a 9800X3D. Game
behavior (NPC reactiveness, UI responsiveness) therefore varies with
hardware. Dynamic budgets tied to frame-time headroom help, but introduce
a feedback loop that can oscillate and needs damping.

### 5. Can mask deeper perf problems

Slapping a budget on a system that needlessly runs every frame treats the
symptom. The better fix is usually:

- Only recompute when inputs change (dirty flags / events)
- Run at a lower rate (10 Hz vs 60 Hz)
- Fix an `O(n²)` algorithm

Budgeting lets you ship without doing the harder structural work, and the
debt compounds.

### 6. Makes debugging non-deterministic

"Why did this NPC behave weirdly?" → because under load its tick spent
400 ms in a deferred queue. Repros depend on CPU load, which depends on
everything else running. Engine programmers consistently cite this as one
of the harder bug classes to track down.

### 7. The budget check itself has cost

We sample `FPlatformTime::Seconds()` per task. For very cheap tasks the
timing bookkeeping can be a meaningful fraction of the budget itself. Real
systems amortize the check (every N tasks), but then overshoot the budget
by up to one batch.

### 8. Breaks implicit "runs every frame" contracts

Lots of gameplay code is written assuming it ticks. If half the systems
tick and half don't, coupled systems drift: animation driven by a
budgeted update system, rendering reading an unbudgeted transform →
visible jitter or logical desync.

### 9. Doesn't help when work is essential

If every task *must* finish this frame (player physics, critical render
transforms, ack-critical network work), budgeting produces bugs, not
performance. The honest answer there is "do less work" or "do it
earlier / later in the frame," not "defer it."

## Why it still matters for the upscaler thesis

Temporal upscalers amplify frame-time variance as visible artifacts
(ghosting, smearing) because their history reprojection assumes a
roughly-consistent dt. A system whose mean frame time is fine but whose
*standard deviation* and *max* are high will look worse under DLSS/FSR/TSR
than a system with slightly higher mean but tighter variance. Budgeting
targets the variance directly, which is why it is a first-class pattern
for upscaler compatibility — but only when the deferred work can tolerate
the latency that budgeting introduces.

## Paper framing (draft paragraph)

> Workload budgeting trades worst-case frame-time variance for per-task
> latency. It is not a performance optimization — total CPU work is
> unchanged or slightly higher, since the bookkeeping itself has cost.
> What changes is the *shape* of that work across frames, which is
> precisely what temporal upscalers are sensitive to. The pattern is
> therefore most valuable for non-urgent, batch-friendly tasks
> (streaming, far-field AI, decorative physics) and should not be
> applied to urgent or correctness-critical work without an explicit
> priority scheme.
