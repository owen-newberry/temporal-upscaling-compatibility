# ase-capstone

## Project Description

This capstone project investigates **architectural patterns and design decisions that ensure compatibility with modern GPU upscaling and frame generation techniques** through the development of a small-scale survival horror game prototype.

The project addresses a practical gap in game development: while temporal upscaling (DLSS, FSR) and frame generation are widely adopted, there is limited documented guidance on the software architecture patterns that ensure compatibility with these technologies. Many student and indie projects inadvertently break temporal reconstruction through poor frame pacing, inconsistent motion vectors, or unstable simulation behavior.

This project implements and empirically validates specific design patterns—including fixed-timestep simulation, workload budgeting, and temporal coherence constraints—then measures their impact on frame-time stability, motion vector quality, and visual fidelity under upscaling. The survival horror genre provides realistic AI and environmental workloads to stress-test these patterns under variable computational demands.

The end deliverable is both a working game prototype and a validated reference architecture with performance data demonstrating the measurable impact of each pattern.

---

## Problem Domain

Modern games increasingly rely on temporal upscaling and AI-driven frame generation to achieve high visual fidelity on consumer hardware. However, these techniques assume:

- **Stable frame pacing** (consistent frame-to-frame timing)
- **Coherent motion vectors** (predictable object movement)
- **Temporal continuity** (deterministic simulation output)

When game architecture violates these assumptions—through frame-rate-coupled logic, inconsistent workloads, or multi-writer transform systems—upscaling quality degrades, producing visual artifacts, ghosting, and frame-time spikes.

This project addresses:

- **How do specific architectural patterns affect upscaler compatibility?**
- **What is the measurable performance impact of each pattern?**
- **Can these patterns be validated empirically in a realistic game context?**

By implementing patterns in isolation and combination, this project provides quantitative evidence of their effectiveness.

---

## Features & Requirements

### Features

#### **Pattern Implementation:**

1. **Fixed-Timestep Simulation**
   - Simulation logic (AI, physics, gameplay) runs at consistent rate independent of rendering
   - Prevents frame-rate-dependent behavior that breaks motion prediction
   - Supports smooth temporal upscaling

2. **Workload Budgeting**
   - Per-frame computational caps for AI and physics systems
   - Time-sliced updates to prevent frame-time spikes
   - Ensures predictable GPU frame submission

3. **Temporal Coherence Patterns**
   - Single-writer motion authority (one system owns transforms)
   - Time-based (not frame-based) visual effects
   - Deterministic simulation output for consistent motion vectors

4. **Data-Oriented Design** (optional depth)
   - Cache-friendly memory access patterns
   - Batch processing where applicable
   - Reduced CPU overhead for more consistent performance

#### **Validation Framework:**

- Baseline implementation (intentionally naive architecture)
- Incremental pattern application
- Comparative performance measurement across configurations
- DLSS/FSR visual quality analysis

#### **Game Prototype:**

- Small survival horror environment (stress-test context)
- AI enemy systems (variable workload)
- Environmental effects (temporal coherence validation)
- Playable vertical slice demonstrating all patterns

### Requirements

- Built with modern game engine (Unreal Engine or Unity)
- Implements all core patterns in modular, measurable way
- Runs on consumer-grade GPU hardware
- Supports DLSS and/or FSR for validation testing
- Clean, documented codebase following software engineering best practices
- Comprehensive performance profiling and comparative analysis

---

## Tests

### Performance Validation

**Frame-Time Analysis:**
- Measure frame-time consistency across pattern implementations
- Compare 1% low FPS and frame-time variance
- Identify performance impact of each pattern

**Workload Stress Testing:**
- Test under varying AI complexity
- Validate budgeting effectiveness under peak load
- Confirm simulation stability during high-intensity sequences

**Motion Vector Stability:**
- Profile motion vector coherence in engine tools
- Validate single-writer pattern effectiveness
- Measure temporal continuity across frames

### Upscaler Compatibility Testing

**Visual Quality Analysis:**
- Compare DLSS/FSR output with and without patterns
- Document visual artifacts in baseline vs. optimized implementations
- Screenshot comparisons at key stress points

**Temporal Reconstruction:**
- Verify temporal effect stability under upscaling
- Test for ghosting, smearing, or motion artifacts
- Validate that patterns improve reconstruction quality

### Comparative Testing

**Configuration Matrix:**
1. Baseline (no patterns)
2. Fixed-timestep only
3. Fixed-timestep + workload budgeting
4. Full pattern implementation

**Metrics Collected:**
- Frame time (avg, min, max, variance)
- CPU/GPU utilization
- Motion vector stability
- Visual quality scores

---

## Project Documentation

- [Project Plan Presentation](https://github.com/owen-newberry/ase-capstone/blob/main/docs/PPP/pdf/ppp.pdf)

Additional documentation will include:

- **Pattern Catalog** — Detailed description of each architectural pattern
- **Implementation Guide** — How to apply patterns in Unity/Unreal
- **Performance Analysis Report** — Charts, graphs, and comparative data
- **System Architecture Diagrams** — Visual representation of pattern relationships
- **Validation Methodology** — Testing procedures and metrics
- **Known Limitations** — Edge cases and future work

---

## Schedule & Milestones

**Project Structure:** This capstone consists of two deliverables:
1. **Research Paper** — Investigates how AI is transforming game development practices and workflows
2. **Code Implementation** — Demonstrates the findings through practical development of a small-scale game

The research paper serves as the theoretical foundation and can stand alone as a capstone deliverable if needed. The code project validates the research findings through practical implementation.

### Sprint 1 (4 weeks) — Research Paper Completion
**Start Date:** February 2, 2026  
**Primary Deliverable:** Initial research phase including literature review, case study analysis, and theoretical framework development

**Research Focus:**
- How GPU advancements (Blackwell architecture, AI integration) reshape game development
- Software engineering pattern evolution in response to hardware changes
- Impact of AI-driven rendering (DLSS, FSR, frame generation) on development workflows
- Analysis of modern game engines (Unreal Engine 5, Unity) adapting to new GPU capabilities

**Week-by-Week Breakdown:**
- **Week 1:** Historical and technical foundation review
  - Survey academic and industry literature on GPU evolution
  - Establish architectural inflection points in gaming hardware
  - Document traditional rendering pipelines vs. AI-integrated approaches

- **Week 2:** Case study analysis begins
  - Unreal Engine 5 and Lumen/Nanite architecture
  - Unity's ECS/DOTS framework for parallel processing
  - DLSS/FSR technical implementation analysis

- **Week 3:** Deep dive into Blackwell architecture and AI features
  - Nvidia technical documentation and SDK analysis
  - Ray tracing vs. rasterization performance implications
  - Neural rendering and frame generation impact on software design

- **Week 4:** Synthesis and framework development
  - Identify recurring engineering patterns
  - Connect hardware capabilities to software design decisions
  - Establish theoretical foundation for code implementation phase

**Code Preparation (Concurrent):**
- Engine selection and installation
- Initial architecture planning
- Review of DLSS/FSR documentation
- Preliminary performance profiling setup

### Sprint 2 (6 weeks) — Code Implementation & Validation
**Start Date:** March 16, 2026  
**Primary Deliverable:** Working game prototype with validated architectural patterns and comparative performance analysis

**Phase 1: Baseline Implementation (Weeks 1-2)**
- Build naive game implementation (intentionally frame-coupled)
- Establish baseline performance metrics
- Document control group measurements

**Phase 2: Pattern Implementation (Weeks 3-4)**
- Implement fixed-timestep simulation
- Add workload budgeting systems
- Apply temporal coherence patterns
- Incremental performance measurement

**Phase 3: Validation & Analysis (Weeks 5-6)**
- Enable DLSS/FSR across all configurations
- Comparative performance testing
- Visual quality analysis
- Synthesize results and document findings

**Final Deliverable:**
- Working game prototype
- Performance analysis report
- Implementation guide
- Presentation materials

---

## Expected Outcomes

### Deliverables:

1. **Working Game Prototype** — Demonstrates all patterns in realistic context
2. **Reference Architecture** — Documented, modular pattern implementation
3. **Performance Analysis** — Empirical validation with quantitative data
4. **Implementation Guide** — Reproducible instructions for pattern application

### Contribution:

> "An empirically-validated reference architecture for temporal upscaler-compatible game development, with measurable performance analysis demonstrating the impact of specific design patterns on frame stability and reconstruction quality."

### Academic Value:

- Bridges theory (known patterns) with practice (measured impact)
- Provides reproducible validation methodology
- Creates educational resource for student/indie developers
- Demonstrates hardware-aware software engineering principles

---

## Future Work

- Extension to additional engine platforms
- Validation with ray-tracing workloads
- Integration with procedural content generation
- Scalability testing with larger game systems
- Pattern application to other real-time interactive applications