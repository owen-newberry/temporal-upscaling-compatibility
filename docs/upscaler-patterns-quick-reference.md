
# Software Design Patterns → Upscaler Performance Connections

**Quick reference document linking software engineering patterns to temporal upscaling/frame-gen performance impacts with sources**

---

## 1. Fixed-Timestep Simulation → Motion Vector Stability & History Quality

### What is this pattern?

Fixed-Timestep Simulation decouples the game's physics/simulation tick from the render loop. Instead of advancing the game state once per rendered frame (which runs at variable speed), the simulation advances in fixed, equal-sized time increments (e.g., every 16.67 ms). A separate "render loop" runs as fast as the hardware allows, interpolating between the two most recent simulation states for smooth visuals. An accumulator tracks any leftover real time between simulation ticks so no time is lost or gained.

The canonical implementation uses an accumulator: real elapsed time is added to the accumulator each frame, then the simulation is stepped forward in fixed increments until the accumulator is exhausted.

### Connection

Fixed-timestep simulation produces deterministic, consistent per-frame motion, which directly improves motion vector accuracy and temporal history reuse in upscalers.

### How it improves performance

- **Stable motion vectors**: Simulation advances in fixed increments, so motion between frames is predictable. Upscalers can accurately reproject previous frames, reducing reprojection error and the need for expensive fallback filters.
- **Better history reuse**: Temporal upscalers accumulate samples across frames; consistent timing means history remains valid longer, so fewer pixels need to be "rebuilt" from scratch each frame.
- **Decoupled render resolution**: Fixed simulation frequency stays constant even when render resolution drops (for upscaling), so gameplay feel is preserved while GPU workload decreases.

### Key quotes from sources

- "The render loop should run as fast as the hardware allows, producing smooth visuals. The physics loop should run at a strict, fixed interval, ensuring stable integration." — André Leite, "Taming Time in Game Engines"
- "The purpose of a fixed step is not related to performance, but to run-time stability." — GameDev.net discussion on fixed timesteps; stability directly feeds into temporal coherence for upscalers.
- "Temporal AA and upscaling can integrate samples appropriately in target resolution pixels... Motion data from the scene indicates how much and which direction the objects had moved from the previous frame." — Apple MetalFX WWDC 2022

### Sources

- **Gaffer On Games – "Fix Your Timestep!"**  
  Classic explanation of fixed-timestep accumulator pattern and why it ensures determinism and smooth motion.  
  https://gafferongames.com/post/fix_your_timestep/

- **André Leite – "Taming Time in Game Engines"**  
  Modern take on fixed timestep with accumulator; emphasizes separation of simulation and render clocks for stability.  
  https://andreleite.com/posts/2025/game-loop/fixed-timestep-game-loop/

- **Unity Manual – "Fixed updates"**  
  Official Unity docs on FixedUpdate and deterministic physics timing.  
  https://docs.unity3d.com/6000.3/Documentation/Manual/fixed-updates.html

- **Bevy Engine – "Run physics in a fixed timestep"**  
  Example showing how to handle player input and advance physics in fixed timesteps for consistent simulation.  
  https://bevy.org/examples/movement/physics-in-fixed-timestep/

- **Unreal Engine – Temporal Super Resolution docs**  
  Explains how TSR uses motion vectors and depth to reproject history; inconsistent motion breaks reprojection and causes blur/ghosting.  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-super-resolution-in-unreal-engine

- **Apple WWDC 2022 – "Boost performance with MetalFX Upscaling"**  
  "Motion data from the scene indicates how much and which direction the objects had moved from the previous frame. Temporal AA and upscaling uses the motion information to back track and find corresponding locations in the previous frame."  
  https://developer.apple.com/videos/play/wwdc2022/10103/

---

## 2. Single-Writer Motion Authority → Coherent Motion Vectors & Reduced GPU Overhead

### What is this pattern?

Single-Writer Motion Authority is an architectural rule: exactly one system owns and writes the final world-space transform (position, rotation, scale) for each entity per frame. No other system may modify that transform directly; anything that needs to influence movement submits a request or velocity to the authoritative system, which resolves all inputs and writes the result once.

This is closely related to the ECS (Entity-Component-System) principle of clear data ownership, and to the "single writer principle" in data-oriented design. In practice it means having one "movement system" or "transform flush" step that runs last in the frame before render, rather than scattered `SetPosition()` calls spread across gameplay, animation, and physics code.

### Connection

Centralizing transform and motion writes ensures that motion vectors are derived from a single, consistent state per entity, eliminating conflicts and reducing redundant motion-vector computation.

### How it improves performance

- **Accurate motion vectors**: Only one system writes final transforms, so "previous → current" motion is always well-defined. This improves reprojection accuracy and reduces ghosting/smearing artifacts that force the upscaler to discard history.
- **Batch motion-vector generation**: Centralized authority enables efficient batch computation of motion vectors (one pass over all entities), reducing per-frame CPU/GPU overhead.
- **Fewer disocclusions**: Cleaner motion data means the upscaler can better track pixels across frames and avoid treating valid history as "disoccluded," which would otherwise trigger expensive re-rendering or fallback logic.

### Key quotes from sources

- "You may discover there are times where some object's motion vectors are being calculated and drawn by each code path handling geometry in the scene, and that this can cause issues that affect these objects." — UE5 Temporal Super Resolution docs
- "Temporal upscalers... require additional input from the engine, such as depth and motion vector information." — Jon Peddie Research on Arm's temporal upscaler
- "Camera jitter and motion vector data... we wanted to make sure that the developer can retain full control over all resources and allocations." — GDC 2023 FSR 2.0 talk

### Sources

- **Unreal Engine – Temporal Super Resolution docs**  
  Discusses how multiple code paths writing motion vectors can cause inconsistencies and artifacts.  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-super-resolution-in-unreal-engine

- **AMD GDC 2023 – "Temporal Upscaling: Past, Present & Future" (slides)**  
  Covers motion vector requirements, jitter handling, and how clean motion data improves upscaler quality.  
  https://gpuopen.com/download/GDC-2023-Temporal-Upscaling.pdf

- **Jon Peddie Research – "Arm enters the RT scaling race"**  
  Explains that temporal upscalers require depth and motion vector information from the engine, and that reactive masks handle edge cases.  
  https://www.jonpeddie.com/news/arm-enters-the-rt-scaling-race/

- **Apple WWDC 2022 – "Boost performance with MetalFX Upscaling"**  
  "Motion data from the scene indicates how much and which direction the objects had moved from the previous frame."  
  https://developer.apple.com/videos/play/wwdc2022/10103/

- **AMD FSR 2.x SDK/manual**  
  Official integration guidance on motion vectors, jitter, and required inputs.  
  https://gpuopen.com/manuals/fidelityfx_sdk/techniques/super-resolution-temporal/

---

## 3. Time-Based Effects/Animations → Consistent Visual Motion Under Variable Frame Rates

### What is this pattern?

Time-Based Effects/Animations is the practice of driving all visual changes — particle systems, shader animations, UI transitions, procedural effects — by elapsed real time (e.g., `sin(time * speed)`) rather than by a per-frame counter or tick index. The effect state at any moment is a pure function of wall-clock (or game-clock) time, not of how many frames have been rendered.

This contrasts with "frame-based" animation where an effect advances by a fixed step each rendered frame regardless of how much time has passed. Time-based updates mean the visual result is identical whether the game runs at 30, 60, or 144 FPS, and remain correct when frame generation artificially increases the displayed frame count.

### Connection

Time-based updates (using elapsed time rather than "per frame") keep visual motion consistent even when render resolution, upscaling, or frame generation changes effective frame pacing.

### How it improves performance

- **Stable temporal inputs**: Effects and animations progress predictably with time, so motion vectors and frame-to-frame changes remain smooth and interpretable by the upscaler.
- **Exploit upscaling/frame-gen fully**: Time-based logic means you can drop base render resolution or enable frame generation without breaking visual timing, allowing more aggressive performance tuning.
- **Reduced artifact triggers**: Frame-based updates can cause sudden jumps or stalls when frame rate changes; time-based updates avoid these, keeping history valid and reducing disocclusion/ghosting.

### Key quotes from sources

- "The render loop should run as fast as the hardware allows... The physics loop should run at a strict, fixed interval." — Fixed timestep ensures time-based progression is decoupled from variable rendering.
- "Temporal AA and upscaling is here to stay. Amortizing pixels temporally increases visual fidelity and boosts performance." — MetalFX talk emphasizing time-coherent rendering for performance.
- "Temporal upscaling... generally results in higher-quality images from lower-resolution targets but requires additional input from the engine, such as depth and motion vector information." — Jon Peddie Research

### Sources

- **André Leite – "Taming Time in Game Engines"**  
  https://andreleite.com/posts/2025/game-loop/fixed-timestep-game-loop/

- **Apple WWDC 2022 – "Boost performance with MetalFX Upscaling"**  
  "With ever-increasing shading costs and pixel counts, temporal AA and upscaling is here to stay. Amortizing pixels temporally increases visual fidelity and boosts performance."  
  https://developer.apple.com/videos/play/wwdc2022/10103/

- **Jon Peddie Research – "Arm enters the RT scaling race"**  
  https://www.jonpeddie.com/news/arm-enters-the-rt-scaling-race/

- **Unreal Engine – Temporal Super Resolution docs**  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-super-resolution-in-unreal-engine

- **AMD GDC 2023 – "Temporal Upscaling: Past, Present & Future"**  
  https://gpuopen.com/download/GDC-2023-Temporal-Upscaling.pdf

---

## 4. Bounded Workload Budgeting → Stable Frame Times & Better Temporal Coherence

### What is this pattern?

Bounded Workload Budgeting is a design strategy where expensive, variable-cost tasks (streaming, AI pathfinding, asset loading, LOD updates, garbage collection) are given a per-frame time budget and split into smaller slices that execute across multiple frames rather than all at once. Work that exceeds the budget is deferred to future frames. This is also called "time-slicing" or "amortized work spreading."

Common implementations include job queues with a maximum execution time per frame, coroutine-based update loops that yield when a timer expires, and incremental algorithms (e.g., incremental GC, incremental BVH rebuilds). The goal is to smooth out CPU/GPU spikes so that each frame takes roughly the same amount of time to produce.

### Connection

Per-frame time budgets and time-sliced tasks keep frame times smooth and predictable, which improves temporal history quality and reduces stutter/jitter in upscalers and frame generation.

### How it improves performance

- **Smoother frame pacing**: Avoiding big spikes means frame-to-frame content changes are gradual, so temporal history remains valid and useful longer, improving upscaler efficiency.
- **More consistent GPU workload**: Predictable per-frame work leaves stable headroom for the upscaler/frame-gen pass itself, avoiding situations where a spike forces the upscaler to run with less time or lower quality.
- **Reduced disocclusion artifacts**: Sudden frame-time jumps can cause content to "teleport" or skip; budgeting reduces this, so reprojection and history accumulation work better.

### Key quotes from sources

- "A central challenge in video restoration using image-based restoration models is ensuring temporal consistency across frames while maintaining sharpness of each frame." — arXiv paper on temporal consistency; stability across frames is key.
- "Temporal AA and upscaling uses the motion information to back track and find corresponding locations in the previous frame in order to correctly gather samples." — Apple MetalFX; smooth content evolution helps tracking.
- "TSR attempts to mitigate over blurring of pixels... but it can't solve the problem of transforming details that are 1 pixel thick that move only half a pixel in the frame." — UE5 TSR docs; frame-time stability helps sub-pixel motion consistency.

### Sources

- **arXiv – "Improving Temporal Consistency and Fidelity at Inference-time in..."**  
  Discusses temporal consistency, flicker reduction, and how stable frame-to-frame content improves temporal models.  
  https://arxiv.org/html/2510.25420v1

- **Unreal Engine – Temporal Super Resolution docs**  
  https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-super-resolution-in-unreal-engine

- **Apple WWDC 2022 – "Boost performance with MetalFX Upscaling"**  
  https://developer.apple.com/videos/play/wwdc2022/10103/

- **AMD GDC 2023 – "Temporal Upscaling: Past, Present & Future"**  
  https://gpuopen.com/download/GDC-2023-Temporal-Upscaling.pdf

---

## 5. Summary Table: Pattern → Upscaler Impact

| Pattern | Upscaler Data Improved | Performance Gain Mechanism | Key Sources |
|---------|------------------------|---------------------------|-------------|
| **Fixed-Timestep Simulation** | Motion vectors, temporal coherence | Stable motion → better history reuse, accurate reprojection, can drop render res without breaking gameplay | Gaffer On Games, André Leite, Apple WWDC 2022, Bevy Engine |
| **Single-Writer Motion Authority** | Motion vector accuracy, consistency | Centralized transforms → batch motion-vector gen, fewer conflicts/disocclusions, reduced GPU overhead | Unreal Engine TSR docs, AMD GDC 2023, Apple WWDC 2022, Jon Peddie Research |
| **Time-Based Effects** | Visual motion stability | Consistent progression under variable frame rates → exploit upscaling/frame-gen fully, fewer artifacts | Unreal Engine TSR docs, AMD GDC 2023, André Leite, Apple WWDC 2022, Jon Peddie Research |
| **Bounded Workload Budgeting** | Frame-time stability | Smooth pacing → better temporal history quality, stable headroom for upscaler, reduced teleporting/disocclusions | Unreal Engine TSR docs, AMD GDC 2023, arXiv, Apple WWDC 2022 |

---

## Additional Core References

### Temporal upscaling fundamentals

- **AMD FSR 2.0 GDC deck**  
  "FIDELITYFX SUPER RESOLUTION 2.0" – motion vectors, jitter, reactive masks, disocclusion logic.  
  https://gpuopen.com/download/GDC_FidelityFX_Super_Resolution_2_0.pdf

- **FSR 2.x SDK/manual**  
  Official integration docs with required inputs and best practices.  
  https://gpuopen.com/manuals/fidelityfx_sdk/techniques/super-resolution-temporal/

- **NVIDIA DLSS / Streamline docs**  
  DLSS-G Programming Guide (motion vectors, depth, HUD-less color):  
  https://github.com/NVIDIA-RTX/Streamline/blob/main/docs/ProgrammingGuideDLSS_G.md

### Game loop / software patterns

- **Gaffer On Games – "Fix Your Timestep!"**  
  https://gafferongames.com/post/fix_your_timestep/

- **Game Programming Patterns – "Game Loop"**  
  https://gameprogrammingpatterns.com/game-loop.html

- **Unity blog – "Level up your code with game programming patterns"**  
  https://unity.com/blog/games/level-up-your-code-with-game-programming-patterns

---

**Use this doc to quickly find connections and citations when writing your research paper sections on how patterns improve upscaler performance.**