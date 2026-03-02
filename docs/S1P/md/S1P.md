---
marp: true
theme: default
paginate: true
---

# How AI is Reshaping Video Game Development
## Using Software Design Patterns for Temporal Upscaling Compatibility

**Sprint 1 Presentation**

Owen Newberry | ASE Capstone

---

# The Problem

**Modern temporal upscaling (DLSS, FSR, TSR) requires clean software architecture and specific design to work effectively**

- Temporal upscalers need stable motion vectors, predictable timing, coherent frame-to-frame data
- Poor game-side code causes ghosting, smearing, artifacts, performance issues
- Student/indie developers lack guidance on software patterns that support these techniques
- Existing resources focus on GPU/graphics side, not software engineering side

---

# Sprint 1 Goal

**Establish research foundation and produce comprehensive literature-backed design patterns**

**Target deliverable:** Complete research paper draft identifying:
1. Temporal upscaler requirements from vendor docs (AMD, NVIDIA, Apple, Epic)
2. Background and Literature Analysis of available resources
3. Four concrete software design patterns addressing these requirements
4. Connections to software engineering design patterns (Strategy, Observer, etc.)
5. Methodology for prototype implementation and evaluation (Sprint 2)
6. Significance of Work and Future Applications

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
- Created project plan focusing on software engineering of game systems (not graphics internals)

**Week 2:** Literature gathering phase
- Surveyed AMD FSR 2/3 docs, NVIDIA DLSS guides, Unreal TSR docs, Apple MetalFX
- Collected GDC presentations, technical reports
- Wrote background and literature analysis

---

**Week 3:** Pattern extraction and design
- Translated upscaler requirements into software constraints
- Drafted four core patterns using standard pattern format

**Week 4:** Finished paper writing
- Completed paper with prominent GoF pattern connections throughout

---

# Sprint 1 Retrospective - What Went Well

**Strong source base built**
- 40+ citations from authoritative sources (vendor docs, engine docs, established game-programming literature)

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
- Quantitative metrics (frame time, motion vector error) are proposed but not yet measured (will be addressed in Sprint 2)

**Time estimation was off**
- Underestimated time to properly document design pattern connections
- Paper took longer than expected to integrate patterns throughout

---

# Learning with AI - Topic 1: Real-Time Screen Capture

**Used AI to deeply understand:**
- Trade-offs between Windows Desktop Duplication API and BitBlt for low-latency screen capture
- How to structure a multi-threaded capture pipeline (capture → process → render) without blocking or dropping frames
- Region-of-Interest capture strategies to minimize unnecessary GPU/CPU work

---

**Approach:**
- Asked AI to compare capture APIs and explain when each is appropriate
- Used AI to understand thread synchronization patterns and queue-based architectures
- Researched change detection techniques to skip processing on static frames

**Key insight:** Keeping the capture, processing, and rendering stages in separate threads with thread-safe handoffs is critical for maintaining a responsive overlay without impacting game performance

---

# Learning with AI - Topic 2: OCR for Game UI

**Used AI to deeply understand:**
- Why game fonts (outlines, shadows, anti-aliasing) break standard OCR engines and how preprocessing fixes it
- Trade-offs between Tesseract, EasyOCR, and PaddleOCR for real-time game text extraction
- Image preprocessing pipelines — adaptive thresholding, morphological operations, color channel isolation

---

**Approach:**
- Asked AI to explain how each OCR engine handles stylized text and where they fail
- Used AI to brainstorm preprocessing steps for specific UI elements (inventory slots, stack counts)
- Researched confidence scoring and domain-validated autocorrection strategies

**Key insight:** Preprocessing is more impactful than engine choice — isolating the text region and normalizing contrast before OCR dramatically reduces misreads on stylized game UI

---

# Sprint 2 Plan - Goals

**Primary goal:** Implement and evaluate a working prototype to validate upscaler compatibility with design patterns

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

**Questions?**
