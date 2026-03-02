---
marp: true
theme: default
paginate: true
---

# How AI is Reshaping Video Game Development
## Using Software Design Patterns for Temporal Upscaling Compatibility

**Project Plan Presentation**

Owen Newberry | ASE Capstone

---

# Project Overview - The Problem

**Modern temporal upscaling (DLSS, FSR, TSR) requires clean software architecture to work effectively**

- Temporal upscalers need stable motion vectors, predictable timing, coherent frame-to-frame data
- Poor game-side code causes ghosting, smearing, artifacts, performance issues
- Student/indie developers lack guidance on software patterns that support these techniques
- Existing resources focus on GPU/graphics side, not software engineering side

---

# Sprint 1 Goal

**Establish research foundation and produce comprehensive literature-backed design patterns**

**Target deliverable:** Complete research paper draft identifying:
1. Temporal upscaler requirements from vendor docs (AMD, NVIDIA, Apple, Epic)
2. Four concrete software design patterns addressing these requirements
3. Connections to classic Gang of Four design patterns
4. Methodology for prototype implementation and evaluation (Sprint 2)

---

# What Was Built - Research Artifacts

### 1. **Quick Reference Document**
- Software patterns → upscaler performance connections
- 40+ citations from vendor docs, GDC talks, engine documentation
- Organized by pattern with key quotes and sources

### 2. **Research Paper Draft**
- ~9,200 words
- Integrated 16 Gang of Four patterns prominently throughout Section 4
- Each of 4 patterns includes "Related GoF Patterns" subsection with concrete examples

---

# Four Design Patterns Proposed

| Pattern | Upscaler Benefit | GoF Patterns Leveraged |
|---------|------------------|------------------------|
| **1. Fixed-Timestep Simulation** | Stable motion vectors, deterministic simulation | Template Method, State, Memento, Iterator |
| **2. Single-Writer Motion Authority** | Coherent motion data, reduced conflicts | Mediator, Facade, Composite, Strategy, Proxy |
| **3. Time-Based Effects** | Consistent visual motion under variable frame rates | Strategy, State, Observer, Flyweight, Template Method |
| **4. Bounded Workload Budgeting** | Stable frame times, better temporal coherence | Command, Chain of Responsibility, Observer, Proxy, Decorator, Iterator |

---

# Weekly Progress Summary

**Week 1:** Project definition, scope refinement
- Identified research question
- Created project plan focusing on software engineering (not graphics internals)

**Week 2:** Literature gathering phase
- Surveyed AMD FSR 2/3 docs, NVIDIA DLSS guides, Unreal TSR docs, Apple MetalFX
- Collected GDC presentations, technical reports

**Week 3:** Pattern extraction and design
- Translated upscaler requirements into software constraints
- Drafted four core patterns using standard pattern format

**Week 4:** Paper writing and GoF integration
- Completed draft with prominent GoF pattern connections throughout

---

# Sprint 1 Retrospective - What Went Well

**Clear research question established early**
- Focused on software engineering, not GPU internals
- Bridged gap between graphics constraints and software architecture

**Strong source base built**
- 40+ citations from authoritative sources (vendor docs, engine docs, established game-programming literature)
- Quick reference doc enables fast citation lookup

**Patterns grounded in real constraints**
- Every pattern directly addresses documented upscaler requirements
- Connections to GoF patterns

**Paper structure solid**
- Academic format with proper methodology section
- Evaluation plan ready for Sprint 2 prototype implementation

---

# Sprint 1 Retrospective - What Went Wrong

**Methodology section is theoretical**
- Test scenarios designed but not yet validated
- Quantitative metrics (frame time, motion vector error) are proposed but not measured

**Pattern format verbose**
- Each pattern ~1.5 pages—may be too detailed for some audiences
- Need summarized version for poster/presentation

**Time estimation was off**
- Underestimated time to properly document design pattern connections
- Paper took longer than expected to integrate patterns throughout

---

# Learning with AI - Topic 1: 

**Used Claude/ChatGPT to deeply understand:**
- 

**Approach:**
- 

**Key insight:** 

---

# Learning with AI - Topic 2: 

**Used Claude/ChatGPT to deeply understand:**
- 

**Approach:**
- 

**Key insight:** 

---

# Sprint 2 Plan - Goals

**Primary goal:** Implement and evaluate a working prototype

### Features to implement:
1. **Fixed-timestep game loop** with interpolated rendering (R1.1-R1.3)
2. **Motion authority system** with centralized transform management (R2.1-R2.4)
3. **Time-based effects system** for particles and animations (R3.1-R3.3)
4. **Workload budgeting system** with task queues and per-frame budgets (R4.1-R4.4)

**Integration:** Connect prototype to temporal upscaler (FSR2, DLSS, or engine TSR) and measure impact

**Total requirements planned:** 14 requirements across 4 features

---

# Sprint 2 Plan - Timeline & Milestones

| Week | Milestone |
|------|-----------|
| **1** | Prototype environment setup (engine, upscaler integration, baseline implementation) |
| **2** | Fixed-timestep loop + Motion Authority implemented |
| **3** | Time-based effects + Workload budgeting implemented |
| **4** | Test scenarios executed, metrics collected (frame time, motion vector error) |
| **5** | Qualitative evaluation (video captures), pattern refinement based on findings |
| **6** | Paper revision with empirical results, demo and presentation prep |

---

# Sprint 2 Plan - Evaluation Strategy

**Quantitative metrics:**
- Frame-time mean and standard deviation (baseline vs pattern-aware)
- Motion-vector reprojection error (measure temporal coherence)
- Workload budget adherence (spike frequency and magnitude)

**Qualitative assessment:**
- Side-by-side video captures showing ghosting, smearing, temporal stability
- Stress test scenarios: fast-moving objects, thin geometry, variable workload

**Comparison:**
- Baseline: Variable timestep, multiple transform writers, frame-based effects, no budgeting
- Pattern-aware: Fixed timestep, single motion authority, time-based effects, budgeted workload

**Success criteria:** Measurable improvement in frame-time stability and reduction in visual artifacts

---

**Questions?**

---

# Thank You

Owen Newberry  
newberryo1@mymail.nku.edu

ASE 485 Capstone Project  
Northern Kentucky University  
Spring 2026
