# Week-by-Week Capstone Plan Explained

Based on your research plan and the README, here's what you should be doing each week in plain terms:

---

## **Sprint 1: Research Paper (4 weeks)**

### **Week 1 (Feb 2-8): Understanding the Foundation**
**What you're doing:** Building your knowledge base

**Practical tasks:**
- Read academic papers about GPU evolution (start with your bibliography sources)
- Watch Nvidia's Blackwell announcement and technical deep-dives
- Learn the difference between ray tracing and rasterization
- Understand what DLSS, FSR, and frame generation actually do
- Read about how game engines worked before vs. after GPU AI features

**What you're learning:**
- How GPUs evolved from "just graphics" to AI processors
- Why modern games need different programming approaches than older ones
- The technical vocabulary you'll need for the rest of the paper

---

### **Week 2 (Feb 9-15): Studying Real Examples**
**What you're doing:** Analyzing how engines actually use new GPU tech

**Practical tasks:**
- Study Unreal Engine 5 documentation (Lumen, Nanite, how they leverage GPUs)
- Explore Unity's ECS/DOTS system documentation
- Read Nvidia DLSS developer guides and AMD FSR documentation
- Watch GDC talks about modern engine architecture
- Download and experiment with UE5 or Unity if you haven't already

**What you're learning:**
- How Unreal Engine 5's features (Lumen for lighting, Nanite for geometry) rely on GPU power
- Why Unity moved to ECS (Entity Component System) for better parallel processing
- How DLSS/FSR work from a developer's perspective, not just a gamer's

---

### **Week 3 (Feb 16-22): Deep Dive into Blackwell**
**What you're doing:** Understanding the cutting edge

**Practical tasks:**
- Read all available Nvidia Blackwell technical documentation
- Compare Blackwell to Ada Lovelace (previous generation) specs
- Research neural rendering techniques
- Study how frame generation works at a technical level
- Look at benchmarks and performance comparisons

**What you're learning:**
- What makes Blackwell different (dedicated AI cores, memory bandwidth, etc.)
- How neural networks are being used for rendering tasks
- The tradeoffs between traditional rendering and AI-assisted rendering
- Why some games now *require* ray tracing (like Indiana Jones)

---

### **Week 4 (Feb 23-29): Connecting Everything**
**What you're doing:** Building your argument

**Practical tasks:**
- Identify the software engineering patterns that emerged because of GPU changes
- Write out the connections between hardware capabilities and software design
- Create an outline for your research paper
- Start drafting your introduction and methodology sections
- Map out how your findings will inform the code project

**What you're learning:**
- How to synthesize all your research into a cohesive argument
- The relationship between GPU hardware and software architecture patterns
- What specific patterns you'll validate in the code phase

**Concurrent (Weeks 1-4):** 
- Choose whether you'll use Unreal or Unity for the code project
- Install your chosen engine and get familiar with it
- Set up performance profiling tools
- Sketch out your game prototype architecture

---

## **Sprint 2: Code Implementation (6 weeks)**

### **Weeks 1-2 (Mar 2-15): Building the "Wrong" Way**
**What you're doing:** Creating a deliberately bad baseline

**Practical tasks:**
- Build a simple horror game prototype with frame-rate-coupled logic
- Make AI update every frame (intentionally inefficient)
- Use multiple systems modifying the same objects
- Run performance tests and save the terrible results

**What you're learning:**
- What NOT to do when building for modern GPUs
- How to measure frame times, GPU usage, CPU usage
- What breaks when you ignore temporal coherence
- How to use profiling tools in your engine

---

### **Weeks 3-4 (Mar 16-29): Fixing It Properly**
**What you're doing:** Applying good architectural patterns

**Practical tasks:**
- Rewrite simulation to run at fixed timestep (independent of frame rate)
- Implement workload budgeting (limit AI cost per frame)
- Apply single-writer pattern (one system controls each object's position)
- Make effects time-based instead of frame-based
- Measure performance at each step

**What you're learning:**
- How fixed-timestep simulation prevents temporal issues
- Why spreading AI work across frames maintains stability
- How single ownership of transforms improves motion vectors
- The actual performance impact of each pattern (with numbers!)

---

### **Weeks 5-6 (Mar 30-Apr 12): Proving It Works**
**What you're doing:** Testing with real upscaling tech

**Practical tasks:**
- Turn on DLSS or FSR in all your builds
- Compare visual quality: baseline vs. optimized versions
- Take screenshots of artifacts (ghosting, smearing) in the bad version
- Show how the good version looks clean with upscaling
- Create charts comparing frame times, FPS stability, GPU usage
- Write up your findings

**What you're learning:**
- Whether your patterns actually improved upscaler compatibility
- How to present technical data visually (graphs, comparison images)
- How to write performance analysis documentation
- Whether your research paper predictions matched reality

**Final Week Tasks:**
- Polish the prototype
- Create architecture diagrams
- Write the implementation guide
- Prepare presentation slides

---

## **The Big Picture**

**Sprint 1 = Understanding WHY** (research paper)
- Why do GPUs have AI cores now?
- Why do engines need new architecture patterns?
- Why does DLSS/FSR need specific coding approaches?

**Sprint 2 = Proving HOW** (code implementation)
- How do you actually implement these patterns?
- How much difference do they make?
- How can other developers learn from this?

Each week builds on the last, and your research directly informs what you build in the code phase.
