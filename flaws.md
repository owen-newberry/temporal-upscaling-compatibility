# Demo 1: L_MotionAuthority — Design Flaws and Reliability Notes

**Strengths:**
- All actions (including dropped) are logged, so the data is complete.
- Jitter detection helps flag simulation artifacts.
- Color and debug messages make in-editor diagnosis easy.

**Potential Flaws:**
- Unrealistic input loss: "Dropped" path logs every discarded input, but real network loss is more bursty/correlated with network events, not just mode toggling.
- Mode switching: If toggled mid-session, CSV will have mixed modes, making analysis harder and less realistic.
- No network simulation: No artificial latency, packet loss, or reordering. Test is idealized, not matching real-world network conditions.
- Single authority: Only one authority manager is assumed. Distributed/peer-to-peer authority is not tested.
- No input delay: Inputs are applied immediately in authority mode, not queued/delayed as in real server-client setups.
- Logger flush: Flushing every tick is safe for data, but could be a performance issue in a real system (not a flaw for this test, but worth noting).

**Reliability:**
- High for local, idealized authority/direct comparison.
- Lower for real-world networked scenarios (no latency, loss, or distributed authority).

---

# Demo 2: L_FixedTimestep — Design Flaws and Reliability Notes

**Strengths:**
- Demonstrates the classic instability of variable timestep Euler integration under spikes.
- Fixed timestep mode uses accumulator and sub-stepping, which is robust and industry standard.
- Spike injection is explicit and parameterized (interval, duration).
- All steps and positions are logged for both modes.

**Potential Flaws:**
- Spike injection is artificial: real-world spikes may be less regular, more bursty, or correlated with system load, not periodic.
- Damping is set to zero by default, which exaggerates instability for demonstration but is not realistic for most physical systems.
- Only a single spring system is tested; no multi-body or coupled systems.
- No external forces or noise: only a pure spring, so results may not generalize to more complex simulations.
- No measurement of energy error or long-term drift, only displacement.
- No real-world frame pacing or OS-level hitches simulated (e.g., background tasks, GPU stalls).

**Reliability:**
- High for demonstrating the instability of variable timestep integration.
- Lower for predicting real-world simulation stability, since real spikes and system complexity are not modeled.

---

# Demo 3: L_TimeBasedAnim — Design Flaws and Reliability Notes

**Strengths:**
- Directly compares frame-based (wrong) and time-based (correct) animation.
- Spike injection is parameterized and visible in both code and on-screen.
- Position error is logged and analyzed, providing a quantitative metric.
- Both actors run in the same world, so FPS and conditions are matched.

**Potential Flaws:**
- Spike injection is artificial: real-world hitches may be less regular, more bursty, or correlated with system load, not periodic.
- Only a single sine wave motion is tested; no multi-axis or more complex animation.
- No simulation of input or network delay; only local time and frame rate effects.
- The error metric is only meaningful for this specific motion; may not generalize to other systems.
- No noise, drift, or external disturbances are modeled.
- Both actors are always visible and running; no test of actor spawn/despawn or world streaming effects.

**Reliability:**
- High for demonstrating the divergence between frame-based and time-based motion under hitches.
- Lower for generalizing to all animation systems or real-world game scenarios.

---

# Demo 4: L_WorkloadBudget — Design Flaws and Reliability Notes

**Strengths:**
- Models a real-world scenario: per-frame CPU task queue with budgeted and unbudgeted processing.
- Budgeted mode enforces a hard time cap, simulating a frame time budget.
- Unbudgeted mode exposes the impact of overrunning the frame budget (visible spikes).
- All task processing and motion is logged; queue overflow is detected and reported.

**Potential Flaws:**
- Task cost is simulated with `FPlatformProcess::Sleep`, which does not model real CPU-bound work (e.g., cache, branch prediction, or multi-threading effects).
- Spike and queue overflow are artificial; real games may have more complex, interdependent workloads.
- No simulation of GPU or rendering cost, only CPU.
- Budget is fixed per frame; no dynamic adjustment or adaptation to system load.
- No measurement of actual gameplay impact (e.g., input lag, missed deadlines).
- Only a single actor is tested; no system-level contention or multi-actor effects.

**Reliability:**
- High for demonstrating the effect of CPU budget enforcement on frame time and motion smoothness.
- Lower for predicting real-world performance, since actual game workloads are more complex and variable.
