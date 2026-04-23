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

**Don't say:** "temporal upscaler compatibility through software design patterns."
(That's the slide title. You already said it. The audience heard it.)

---

## Slide 2 — Why should anyone care?

**Core idea (plain English):**

Your graphics card has too much work to do. Modern games render at high
resolutions with fancy lighting and thousands of objects. To keep the frame
rate playable, almost every modern game uses a trick: **render at half
resolution, then guess what the other half would look like by studying
previous frames.**

That trick is called *temporal upscaling*. DLSS (by NVIDIA), FSR (by AMD),
and TSR (built into Unreal Engine) are the three big implementations. Almost
every AAA game made in the last three years ships with one turned on by
default.

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

## Slide 3 — The research question

**Say:** "Specifically: are there known ways of writing game code that make
this trick work better? And can we prove it with numbers?"

**Keep it short.** Don't belabor the question — the audience already gets
it from slide 2.

---

## Slide 4 — The four patterns

**Say:** "I picked four of them, all well-known in game dev, none of them
invented here. They each fix one specific thing that breaks the upscaler's
ability to guess the next frame."

**If someone asks for specifics:**

- **Pattern 1 (fixed-timestep simulation):** "Your game's physics runs on a
  steady internal heartbeat, even when the screen can't keep up. Like a
  metronome." *(You can point at Demo 2 live if you want.)*
- **Pattern 2 (time-based motion):** "The position of moving things is
  computed from the clock, not from how many frames have gone by. A
  window blind that falls at a steady 30 cm/s — not 'move one centimeter
  every frame.'"
- **Pattern 3 (single-writer authority):** "One system is in charge of
  moving each object. If two different systems try to move the same cube,
  they fight and the cube jitters."
- **Pattern 4 (workload budgeting):** "The game does a bit of expensive
  work each frame instead of all of it at once. Like doing ten minutes of
  dishes after every meal instead of two hours of dishes on Sunday."

---

## Slide 5 — What I built

**Say:** "Four mini-demos in Unreal Engine, each one has a cube that does
the thing *right* and a cube that does the thing *wrong*, side-by-side. And
I built a separate tool to log how they behave and spit out a report."

**Why the dual-cube setup:** if both cubes are in the same scene, same
camera, same everything — the *only* thing that could differ is the pattern.
Any difference you see is the pattern doing its job.

**The harness is the real contribution:** "The cubes are teaching aids;
the actual deliverable is the measurement pipeline. Anyone can run it on
their own game to see where their patterns are weak."

---

## Slide 6 — Live demo

**If you do it:** Open `L_WorkloadBudget`. Hit Play. "Green cube is
well-behaved; red cube isn't. Watch the frame rate number when I…"
Click the red cube, toggle `bBudgeted` off. "…turn off the budget on the
red one. Now it's a bully." *(Frame rate drops to ~22.)*

**If you skip it:** "I have a live version but for time I'll show you the
numbers from last night's capture on the next slide."

**If the live demo breaks:** don't panic. Say "Unreal is having a moment
— I've got the recorded data." Move on.

---

## Slide 7 — Headline result

**Say:** "This is Demo 4 — the budgeting pattern. Identical everything
except whether the CPU work gets capped at 2 milliseconds per frame or
not. The budgeted version runs at 112 FPS. The unbudgeted version runs at
22 FPS. Same total work. Same code. Same actors. **5× frame rate
difference** just from using the pattern."

**The real insight to drop in:** "It's not that the budgeted version is
doing less work — both cubes do exactly the same amount of work over the
session. It's that the budgeted one spreads it evenly. That's what the
upscaler needs: predictable."

**Subtle important point (if someone asks):** The *mean* work per frame
for the budgeted cube lands at 2.00 milliseconds. The budget was set to 2
milliseconds. So the pattern is doing exactly what it advertises — no more,
no less. That's the kind of measurability the paper is arguing for.

---

## Slide — Single-Writer Authority results (Demo 1)

**Say:** "This is the authority pattern. Two different systems both want
to move the same cube every frame — a common situation, happens any
time you have animation and physics, or networking and local prediction,
trying to control the same object. In the 'direct' version, whichever
system writes the transform last wins, and the cube jitters. In the
'authority' version, both systems submit their intent to a single
arbitrator, and only the arbitrator touches the transform. The frame-to-frame
variation — that's what gives upscalers fits — **drops by almost half**."

**The number to lead with:** "**P95 frame delta goes from 9.9 cm to 5.2
cm. Same motion, same cost — 47% less variance.**"

**If someone asks about cost:** "Mean FPS is identical. The authority
isn't doing real work — it's just deciding who gets to write. One extra
function call per input. The pattern is almost free."

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

## Slide — Fixed-Timestep results (Demo 2)

**Say:** "This is a spring simulation. Same oscillator, same initial
conditions, same hitches injected — the only difference is whether the
simulation steps once per rendered frame or chops each frame into 16ms
sub-steps. **The variable version overshoots the spring's physical
maximum by 50% during hitches** — it's producing nonsense, and those
nonsense positions are what the upscaler would be reprojecting."

**The number to lead with:** "**Max position delta is 306 cm for
variable, 200 cm for fixed. The spring can only physically reach
200 cm of amplitude — variable is giving us positions outside the
physically possible range.**"

**Backup number (same direction):** "**Standard deviation of per-frame
motion drops by 44%** when you switch to fixed-stepping. Lower
variance = cleaner motion vectors = happier upscaler."

**Which column NOT to cite:** "P95 delta" — this one is slightly
higher for Fixed (11.4 vs 9.5) because sub-stepping collapses multiple
16ms simulation steps into one rendered frame. That reads as a bigger
"per-rendered-frame" delta but it's correct catch-up behavior, not
instability. If someone asks about it, that's the explanation; don't
lead with it.

**If asked "why not use RK4 or Verlet?":** "Fair — real engines do.
Euler is the pedagogical baseline here. The **pattern** (sub-stepping
with a max-catch-up clamp) is what generalizes; it works with any
integrator. RK4 without sub-stepping would still fail under a 300ms
hitch, just less spectacularly."

---

## Slide — Time-Based Animation results (Demo 3)

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

## Slide — Workload Budgeting results (Demo 4)

**Say:** "This is the headline result. Same CPU work done in both
actors over the session — same tight math loop, same number of
iterations, totals out to the same number of CPU cycles. One of them
caps itself at 2 milliseconds per frame; the other just does all the
work whenever it's asked. **5× frame-rate difference** just from
managing *when* the work happens."

**Numbers to memorize:** "112 FPS budgeted, 22 FPS unbudgeted. Mean
work per frame lands at 2.00 milliseconds — the budget was set to
2 milliseconds. Pattern does exactly what it advertises."

**The swarm variant:** "I also ran it with 8 intensive actors
simultaneously to simulate a real game with multiple expensive
subsystems. The effect shrinks to about 1.7× (56 FPS vs 33 FPS) but
it's still there and still significant. Of course the effect shrinks —
you can only defer work that *can* be deferred, and at some point the
total work exceeds any budget. That's not a failure of the pattern,
it's an honest diminishing return."

---

## Slide 7.5 — Combined stress

**Say:** "Then I ran all four demos in the same scene at once to make
sure the patterns still work when they're stacked on top of each other. They
do. The budget still held at 2.00 ms. The physics still stayed bounded.
Patterns compose."

**Why this matters:** "Real games have all four problems at the same time.
Showing each pattern works in isolation is fine for a paper — showing they
work *together* is what makes it useful to a real team."

---

## Slide 8 — Not a silver bullet

**Say:** "The catch is: all four patterns add cost somewhere else. Budgeting
in particular means you're *delaying* work, not deleting it. If you delay
the wrong thing — player input, physics for something important, network
acks — your game breaks in subtle ways: NPCs freeze for half a second,
your character walks through a wall, etc."

**If someone pushes back ('then isn't this just a hack?'):** "It's a
trade. Visible jitter goes away; invisible latency shows up. The value is
that visible jitter is what upscalers amplify into artifacts you can see,
and latency is often something a player can tolerate on background
systems."

**Key phrase:** "Predictable, not faster."

---

## Slide 9 — Methodology contribution

**Say:** "The long-term value isn't the four demos — it's the pipeline.
I can drop a new upscaler into this thing (FSR, Intel's XeSS, whatever
comes next) and instantly get a report comparing it across all four
patterns. That's the research-reuse story."

**For engineering audiences:** "Capture to CSV, Python does the math,
markdown comes out the other end. Reproducible. Anyone with the repo
can rerun it."

---

## Slide 10 — What's next

**Say:** "FSR integration is the biggest unfinished piece. I've scoped
it at about four hours of work — the scope doc is in the repo. The
bigger paper will compare TSR, FSR, and DLSS head-to-head across all
four patterns, which becomes a real contribution to the upscaler
literature."

---

## Slide 11 — Thanks / Q&A

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
3. Showing they compose under combined load

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
| Demo duration | ~30 s each |
| Number of patterns studied | 4 |
| Number of upscalers compared (currently) | 1 (TSR); FSR planned |
| Combined-stress actors | 8, one level |
| Authority σ Δ reduction | 43% (3.4 → 1.9 cm) |
| Authority P95 Δ reduction | 47% (9.9 → 5.2 cm) |

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
