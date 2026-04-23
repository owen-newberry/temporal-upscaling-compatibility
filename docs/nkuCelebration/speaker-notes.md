# Speaker Notes (Plain-English Edition)

One-to-two paragraph explanations for each slide, written for a **non-technical
audience** (friends, parents, humanities majors, the person at the reception
line who asked "what's your thing?"). Use these to rehearse, memorize the
cadence, and field questions.

Rules of thumb for speaking:

- If someone looks lost, say "sorry — let me back up" and use the analogy.
- If someone looks bored, say "here's the punchline" and jump to the result.
- Never use the words **temporal**, **motion vector**, **reprojection**,
  **sub-step**, or **Euler integration** without immediately explaining them.

---

## Slide 1 — Title

**Say:** "I built a game engine prototype that shows how the way you *write*
game code affects how well a modern upscaler — DLSS, FSR, the stuff that
makes games run faster — actually works. Then I measured it."

**Don't say:** "Software design patterns for temporal upscaling compatibility
and performance in real-time games." (That's the slide title. You already
said it. The audience heard it.)

---

## Slide 2 — What is Upscaling in Video Games?

**Core idea (plain English):**

Your graphics card has too much work to do. Modern games render at high
resolutions with fancy lighting and thousands of objects. To keep the frame
rate playable, almost every modern game uses a trick: **render at lower
resolution, then guess what the rest of the image would look like by
studying previous frames.**

That trick is called *temporal upscaling*. DLSS (NVIDIA), FSR (AMD),
TSR (Unreal Engine), and XeSS (Intel) are the four big implementations.
Almost every AAA game made in the last three years ships with one turned
on by default.

**The analogy that works for everyone:**

> "Imagine you're trying to write down every other word of a song as
> someone sings it. If the singer keeps a steady rhythm, you can guess the
> missing words. If they keep stuttering or skipping ahead, you can't.
>
> Upscalers are like that. They can fill in the missing half of the picture
> only if the game is behaving predictably. Most games are *not* predictable —
> and nobody really writes down what 'predictable' means."

**The punchline:** I wrote it down, measured it, and built a tool that lets
anyone measure it the same way.

---

## Slide 3 — The Problem

**Say:** "The catch is that these upscalers only work if the game is behaving
predictably. Objects have to move smoothly. Physics can't teleport when the
frame rate spikes. Animations can't stutter. When games break those rules —
and they do, all the time — the upscaler's guesses are wrong, and you get
artifacts: smearing, ghosting, flicker, jitter."

**The hook:** "So the question becomes — are there known ways of writing game
code that *keep* the game predictable? And can we prove it with numbers?"

**Keep this short.** The audience already cares from slide 2; this is just
naming the specific failure modes.

---

## Slide 4 — The Solution

**Say:** "I identified four software design patterns — all well-known in game
development, none invented here — that each fix one specific way real games
violate temporal predictability."

**One breath, then advance.** This slide exists to set up the table on the
next slide. Don't try to list the patterns yet.

---

## Slide 5 — Four Patterns, One Goal: Temporal Predictability

**Say:** "Here they are. Four patterns, each addressing one specific failure
mode that breaks the upscaler's ability to reuse information from previous
frames."

**If someone asks for specifics:**

- **Pattern 1 (fixed-timestep simulation):** "Your game's physics runs on a
  steady internal heartbeat, even when the screen can't keep up. Like a
  metronome."
- **Pattern 2 (time-based animation):** "The position of moving things is
  computed from the clock, not from how many frames have gone by. A
  window blind that falls at a steady 30 cm/s — not 'move one centimeter
  every frame.'"
- **Pattern 3 (single-writer authority):** "One system is in charge of
  moving each object. If two different systems try to move the same cube,
  they fight and the cube jitters."
- **Pattern 4 (workload budgeting):** "The game does a bit of expensive
  work each frame instead of all of it at once. Like doing ten minutes of
  dishes after every meal instead of two hours of dishes on Sunday."

**Sources to name-drop if pressed:** Gaffer On Games, Valve's GDC networking
talks, NVIDIA developer docs, console performance guidelines.

---

## Slide 6 — The Paper

**Say:** "The paper itself is a literature-review-and-framework. It surveys
these four patterns across prior engineering literature, organizes them
under a single framing — *upscaler compatibility* — that the source material
never explicitly used, and argues that each pattern fixes a specific failure
mode of the math behind DLSS, FSR, and TSR."

**The novel-contribution framing:** "Nobody had synthesized these under a
single umbrella before. Programmers knew the patterns. Researchers knew the
upscalers. Connecting them is what's new."

---

## Slide 7 — The Methodology

**Say:** "To turn that literature review into actual *evidence*, I built
three things. Four mini-demos in Unreal Engine 5.7, each with a good-design
cube and a bad-design cube side-by-side. A logging harness that captures
per-frame motion and timing to CSV. And a Python analysis pipeline that
spits out a self-updating report at the end of every session."

**Why the dual-cube setup:** "If both cubes are in the same scene, same
camera, same everything — the *only* thing that could differ is the pattern.
Any difference you see is the pattern doing its job."

**The harness is the real contribution:** "The cubes are teaching aids; the
actual deliverable is the measurement pipeline. Anyone can run it on their
own game to see where their patterns are weak. That's the part that outlives
this paper."

---

## Slide 8 — Live Demo

**If you do it:** Walk through the recorded clips in order — Fixed Timestep,
Time-Based Animation, Motion Authority, then the Workload Budgeting pair
(unbudgeted first, then budgeted for the dramatic reveal).

**As each clip plays, narrate once:** "Green cube is well-behaved; red cube
isn't. Watch the frame rate number in the corner." For the workload pair:
"Same scene. Same work. Frame rate goes from 22 to 112 just from capping
CPU work at 2 ms per frame."

**If a clip breaks / doesn't load:** Don't panic. Say "video's having a
moment — I've got the numbers on the next slide." Move on.

---

## Slide 9 — Fixed-Timestep Results

**Say:** "This is a spring simulation. Same oscillator, same initial
conditions, same 300 millisecond hitch injected every few seconds — the
only difference is whether the simulation steps once per rendered frame
or chops each frame into 16ms sub-steps. **The variable version overshoots
the spring's physical maximum by 50% during hitches** — it's producing
nonsense, and those nonsense positions are what the upscaler would be
reprojecting."

**The number to lead with:** "**Max position delta is 306 cm for variable,
200 cm for fixed. The spring can only physically reach 200 cm of amplitude —
variable is giving us positions outside the physically possible range.**"

**Backup number (same direction):** "**Standard deviation of per-frame
motion drops by 44%** when you switch to fixed-stepping. Lower variance =
cleaner motion vectors = happier upscaler."

**The cost story:** "**Mean FPS is identical — 119.9 on both.** The pattern
is essentially free; you're just reshaping when the math runs."

**Which column NOT to cite:** "P95 delta" — this one is slightly higher
for Fixed because sub-stepping collapses multiple 16ms simulation steps
into one rendered frame. That reads as a bigger "per-rendered-frame" delta
but it's correct catch-up behavior, not instability. If someone asks,
that's the explanation; don't lead with it.

**If asked "why not use RK4 or Verlet?":** "Fair — real engines do. Euler
is the pedagogical baseline here. The **pattern** (sub-stepping with a
max-catch-up clamp) is what generalizes; it works with any integrator.
RK4 without sub-stepping would still fail under a 300ms hitch, just less
spectacularly."

---

## Slide 10 — Time-Based Animation Results

**⚠️ This is the slide with the subtlest framing risk — read twice.**

**Say:** "Two cubes moving on the same intended trajectory. The
frame-based version advances a counter by a fixed amount every frame,
which works great at 60 FPS but drifts when the frame rate changes.
The time-based version reads the wall clock. **During a 400 millisecond
hitch, the frame-based cube falls 4 meters behind where it's supposed
to be.** The time-based cube catches up on the next frame — its motion
vector gets big for one frame, but it's an **honest** motion vector, and
the upscaler reprojects it cleanly."

**The number to lead with:** "**Frame-based drifts up to 400 cm —
4 meters — from its own design target during hitches. Time-based
hits the target exactly.**"

**⚠️ DANGER PHRASE — do not say:** "Time-based has zero error." That's
technically what the CSV says, but it's a tautology: the ground truth
for error is the time-based formula itself. Saying "zero error" invites
a smart audience member to point out the circularity.

**Safer phrasing:** "The claim isn't that time-based is perfectly
accurate in some abstract sense — it's that **frame-based fails to hit
its own stated design target** by meters when the frame rate deviates
from assumed. The target is `A·sin(2π·f·t)`. Time-based code
computes exactly that. Frame-based code approximates it with a
counter and drifts."

**If someone asks "so time-based has bigger motion vectors during
hitches — isn't that bad for the upscaler?":** "It looks worse in the
σ-delta column (3.99 vs 3.74) but it's actually fine — temporal
upscalers reproject based on motion vectors. A big but **correct**
motion vector is much easier to reproject than a small but **wrong**
one. The problem with frame-based isn't motion vector magnitude, it's
that the object is in the **wrong place** — that's animation stutter,
and no amount of reprojection fixes it."

---

## Slide 11 — Single-Writer Motion Authority Results

**Say:** "This is the authority pattern. Two different systems both want
to move the same cube every frame — a common situation, happens any
time you have animation and physics, or networking and local prediction,
trying to control the same object. In the 'direct' version, whichever
system writes the transform last wins, and the cube jitters. In the
'authority' version, both systems submit their intent to a single
arbitrator, and only the arbitrator touches the transform. The
frame-to-frame variation — that's what gives upscalers fits — **drops
by almost half**."

**The number to lead with:** "**P95 frame delta goes from 9.9 cm to
5.2 cm. Same motion, same cost — 47% less variance.**"

**If someone asks about cost:** "Mean FPS is identical — 119.8 on both.
The authority isn't doing real work — it's just deciding who gets to
write. One extra function call per input. The pattern is almost free."

**If someone asks about the rival writer:** "It's deterministic — a
second sine wave at a higher frequency layered on top of the primary
motion. Represents any second subsystem that legitimately wants to move
the object. Scripted animation. Physics contact. Network correction.
AI pathfinding. In a real game there are usually three or four of these
fighting for every transform."

**If someone asks 'isn't the rival writer just two function calls in the
same Tick?' (the honest methodologist question):** "Yes — in the
implementation. From the upscaler's perspective, what matters is the
final transform the renderer sees; whether that transform was chosen by
one function or two is invisible to reprojection. The demo models the
**failure mode** (motion vector contaminated by a second writer), not
the **organizational case** (two independent subsystems in different
tick groups). Upgrading to a separate component is on the roadmap,
but it would not change what the numbers measure."

---

## Slide 12 — Workload Budgeting Results

**This is the headline result. Land it clean.**

**Say:** "Same CPU work done in both actors over the session — same
tight math loop, same number of iterations, totals out to the same
number of CPU cycles. One of them caps itself at 2 milliseconds per
frame; the other just does all the work whenever it's asked. **5×
frame-rate difference** just from managing *when* the work happens."

**Numbers to memorize:** "112 FPS budgeted, 22 FPS unbudgeted. Mean
work per frame lands at 2.00 milliseconds — the budget was set to
2 milliseconds. Pattern does exactly what it advertises."

**The real insight to drop in:** "It's not that the budgeted version
is doing less work — both actors do exactly the same amount of work
over the session. It's that the budgeted one spreads it evenly.
That's what the upscaler needs: predictable."

**Max frame time callout (if you have time):** "Even max frame time
drops — 116 ms unbudgeted down to 98 ms budgeted. The worst frame is
better, not just the average."

---

## Slide 13 — The Catch: Budgeting Trades Jitter for Latency (part 1)

**Say:** "Now — these patterns are not free. They defer work; they
don't delete it. Budgeting is the clearest example: a 20-millisecond
CPU spike becomes a smooth 2-millisecond-per-frame cost spread across
ten frames. Great for the upscaler — variance drops, frame rate climbs
5×. But deferred work is **delayed** work."

**Transition line to the next slide:** "And if you apply this pattern
to the *wrong* system, you trade a visible problem for an invisible
one…" *(advance)*

---

## Slide 14 — The Catch: Budgeting Trades Jitter for Latency (part 2)

**Say:** "Player physics deferred means your character can clip through
a wall. Input handling deferred means your clicks feel laggy. Far-field
AI deferred means NPCs freeze for 300 milliseconds and then snap into
position. The jitter went away, but something worse might have taken
its place."

**Key phrase:** "**The design question isn't 'should I budget?' —
it's 'which systems can tolerate deferral?'**"

**The overall framing:** "**Predictable, not faster.** That's the whole
paper in two words. The patterns don't make games faster in some
absolute sense — they make them *predictable*, which is what the
upscaler needs, and what the player *feels* as smoothness."

**If someone pushes back ('then isn't this just a hack?'):** "It's a
trade. Visible jitter goes away; invisible latency shows up. The value
is that visible jitter is what upscalers amplify into on-screen
artifacts, and latency is often something a player can tolerate on
background systems. The skill is knowing *which* systems."

---

## Slide 15 — Why This Matters

**Say:** "Two audiences benefit directly. Engine programmers get four
concrete, code-level changes with measured impact, plus the tooling to
validate their own codebase against the same metrics. And players get
fewer smearing and ghosting and stutter artifacts in games built on
these principles — which is the whole point."

**If someone asks about the research angle (it used to be on the slide):**
"The research contribution is the synthesis and the harness — gathering
four scattered game-dev patterns under a single framing and building a
reproducible measurement pipeline. The follow-up paper compares TSR,
FSR, and DLSS head-to-head across all four patterns. The code is
already upscaler-agnostic; only the plugin installs and capture runs
remain."

**If someone asks "so what's next?":** "Three-way TSR / FSR / DLSS
comparison. That's the real upscaler-literature contribution — this
paper is the framework, the next one is the evaluation."

---

## Slide 16 — Thanks / Q&A

Memorize two opening moves:

1. **"Great question — let me think for a second."** Buys you 3-5 seconds
   to compose.
2. **"That's actually in the paper — the short version is…"** Lets you
   respond without going deep.

Don't bluff. If you don't know, say "I didn't test that" or "I don't
know" and offer "happy to follow up over email."

---

## Deep-dive explanations (for tougher questions)

### "Why does the unbudgeted version run slower if it's doing the same work?"

Think of the CPU like an oven. If you try to bake six trays of cookies
at once, the oven overheats and everything takes longer because it keeps
shutting off to cool down. If you bake one tray at a time on a schedule,
everything comes out faster overall and none of them burn.

The unbudgeted cube is cramming 20ms of CPU work into every single frame.
That blows past the 16ms-per-frame budget needed to hit 60 FPS, so the
frame runs long, everything else queues up, and the game runs at 22 FPS.

The budgeted cube caps itself at 2ms. The remaining 18ms of work gets
distributed across future frames. Total work is identical — the *shape* of
that work across time is what changes. The game feels smooth because
every frame fits in its budget.

### "Why does fixing the physics timestep matter for graphics?"

Upscalers need to know: "that pixel I drew last frame — where did it move
to this frame?" If the physics is jittery or jumpy, every object gets a
wildly different motion vector each frame. The upscaler's guesses for the
in-between pixels are based on those motion vectors, and when the vectors
are wrong, the guesses are wrong, and you see smearing or ghosting
artifacts on screen.

Fixed-timestep physics runs on a steady internal beat regardless of how
long a frame took. Motion between frames becomes predictable. Upscaler
guesses are accurate. No smearing.

### "Why does the Variable-timestep spring 'fly off' during a hitch?"

(This is the tough one. Most audiences won't ask it, but if you're
presenting to programmers, someone will.)

The spring simulation uses a math shortcut called **Euler integration**.
It says: "assume the spring's acceleration is constant for this whole
step, then compute the new position." That shortcut is accurate when the
step is small (like 1/60th of a second). It's very inaccurate when the
step is big — like during a 250-millisecond hitch.

During a big step, the shortcut treats a huge, changing force as if it
were fixed, multiplies it by the big timestep, and produces a position
that's completely wrong. The spring doesn't physically "fly off" — the
*math* gave us a nonsense answer that says it flew off.

The fixed-timestep version chops that 250ms hitch into 15 tiny steps of
16ms each, so Euler's math never goes wrong. Same simulation, same
spring, different way of feeding the math — and the difference is
"spring explodes" vs. "spring keeps oscillating normally."

### "Why not just use a better integrator than Euler?"

Fair — and yes, real engines use Runge-Kutta, Verlet, etc. But Euler is
the pedagogical baseline and most game engines still use it for cheap
per-frame stuff. The *pattern* of chopping big timesteps into small ones
is what generalizes — it's what lets you use *any* integrator with
confidence.

### "Is this actually new research?"

The four patterns aren't new — they're from game-development folklore
and engineering blogs (Gaffer On Games, Valve GDC talks, NVIDIA
dev docs). What's new is:

1. Gathering them under a single framing ("upscaler compatibility")
2. Producing a measurement harness that quantifies each one
3. Showing they hold up under the same empirical methodology

No one had done that synthesis before, which is what the paper argues.

### "Why Unreal 5 specifically?"

Because TSR is built in, because FSR and DLSS plugins exist, because
it's C++ (so the patterns can be demonstrated at the engine level, not
gameplay-script level), and because it's the engine most likely to be
used for the games this research would inform. The methodology works in
any engine — Unity, Godot, custom — but the demos needed a specific
test-bed.

---

## Cheat-sheet: numbers to have memorized

| Thing | Number |
|---|---|
| FPS, budgeted | 112 |
| FPS, unbudgeted | 22 |
| Frame-rate ratio | 5× |
| Mean work per frame, budgeted | 2.00 ms |
| Configured budget | 2 ms |
| Max frame time (unbudgeted → budgeted) | 116 → 98 ms |
| Fixed-timestep Max Δ (variable vs fixed) | 306 cm vs 200 cm |
| Fixed-timestep σ Δ reduction | 44% |
| Time-based: Frame-based schedule drift | 400 cm (4 m) |
| Authority σ Δ reduction | 43% (3.4 → 1.9 cm) |
| Authority P95 Δ reduction | 47% (9.9 → 5.2 cm) |
| Mean FPS (all non-budgeting demos) | ~119.9 (pattern is free) |
| Number of patterns studied | 4 |
| Number of upscalers compared (currently) | 1 (TSR); FSR + DLSS in follow-up |

If you get one of these wrong on stage, it's fine — nobody fact-checks
during a research celebration. What matters is the *direction* (budgeted
is 5× faster, not 50× or 50% faster).

---

## Backup: sentence-long versions of everything

If your slot gets cut to 90 seconds:

> "Modern games use upscalers — DLSS, FSR — to render at lower
> resolution and guess the rest. Those upscalers only work when the
> game behaves predictably. I identified four known patterns from game
> development that keep the game predictable, built small test cases
> for each one, and measured the result. The biggest showed a 5×
> frame-rate improvement just from enforcing a per-frame CPU budget.
> The pipeline lets anyone else measure the same thing on their own
> code. Full paper comparing TSR, FSR, and DLSS is the next step."

Seven sentences. Sixty seconds at a normal speaking pace. Memorize this.
