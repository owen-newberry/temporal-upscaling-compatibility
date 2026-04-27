# Paper-ready numbers (from `summary_metrics.csv`, 2026-04-27)

Values below are from `summary_metrics.csv` in this folder. Rounded for paste into LaTeX; verify in the CSV if you need more precision.

---

## Demo 1 — Motion Authority (Table)

| Metric | Direct | Authority | % change (Authority vs Direct) |
|--------|--------|-----------|----------------------------------|
| Mean Δ (cm) | 2.08 | 2.08 | 0% |
| Std Δ (cm) | 1.09 | 1.09 | 0% |
| P95 Δ (cm) | 3.50 | 3.50 | 0% |
| Max Δ (cm) | 53.34 | 53.34 | 0% |
| Mean FPS | 195.18 | 195.18 | 0% |

`% change` for variance/tail = `(Authority - Direct) / Direct * 100` for Std, P95, Max.

**Note:** This session’s **Direct** and **Authority** rows are numerically almost identical in the aggregate (same mean Δ, σ, P95, max). The demo’s intended story is a cleaner Authority signal; if your paper text assumes a clear σΔ separation, re-check the level (phantom writer / authority wiring) or capture again and confirm `summary_metrics` before publishing.

---

## Demo 4 — Workload budget (Table)

| Metric | Unbudgeted | Budgeted |
|--------|------------|----------|
| Mean work / frame (ms) | 3.01 | 1.50 |
| Max work / frame (ms) | 5.24 | 10.38 |
| Max frame time (ms) | 43.30 | 26.70 |
| Mean FPS | 38.58 | 72.44 |

**Speed-up (Budgeted / Unbudgeted Mean FPS):** 72.44 / 38.58 ≈ **1.88×**

**Note on Max work:** The aggregate **max** of `FrameWorkMs` in the Budgeted run can **exceed** a nominal budget for rare frames (e.g. first-frame / spike); the **mean** and **P95** work (see `summary_metrics`) are usually the clearest “capped work” story. Use `summary_metrics.csv` columns `MeanWorkMs`, `P95WorkMs`, `MaxWorkMs` as needed.

---

## One-line session metadata (fill in for paper)

- **UE:** 5.7  
- **Capture date:** 2026-04-27  
- **Upscaling:** project default (TSR / engine settings in `demos/Config/DefaultEngine.ini`)  
- **Hardware:** _(add CPU, GPU, RAM, OS from your machine)_  
