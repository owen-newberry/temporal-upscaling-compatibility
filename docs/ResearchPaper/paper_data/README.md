# Paper data bundle (sync to Mac / Overleaf)

This folder and `../figures/` hold **everything you need to drop into the paper** without the Unreal `demos/Saved/` tree. Regenerated on **2026-04-27** from clean captures in `demos/Saved/Logs/`.

## What to copy to your Mac

After `git pull` on the Mac in the `ase-capstone` repo:

| Path | Contents |
|------|----------|
| `docs/ResearchPaper/paper_data/summary_metrics.csv` | All demo/mode aggregates (numbers for tables) |
| `docs/ResearchPaper/paper_data/performance_report.md` | Human-readable tables + interpretation text |
| `docs/ResearchPaper/paper_data/paper_tables.md` | Pre-formatted Demo 1 / Demo 4 numbers + speed-up |
| `docs/ResearchPaper/figures/demo1_jitter.png` | Final paper figure |
| `docs/ResearchPaper/figures/demo2_positions.png` | Final paper figure |
| `docs/ResearchPaper/figures/demo3_schedule_error.png` | Final paper figure |
| `docs/ResearchPaper/figures/demo4_frametime.png` | Final paper figure |

**Raw MotionCapture CSVs** are large and live under `demos/Saved/Logs/` (gitignored). If the Mac has the same OneDrive path, they may already be there. Otherwise copy these files manually once:

- `MotionCapture_20260427_134621.csv` — Demo 2 (Fixed Timestep)  
- `MotionCapture_20260427_135151.csv` — Demo 1 (Motion Authority)  
- `MotionCapture_20260427_135413.csv` — Demo 3 (Time-based motion)  
- `MotionCapture_20260427_142355.csv` — Demo 4 Budgeted  
- `MotionCapture_20260427_142806.csv` — Demo 4 Unbudgeted  

## Regenerate (Windows or Mac)

From the **repo root** (with the five CSVs in `demos/Saved/Logs/`):

```bash
python analysis/aggregate_metrics.py
python analysis/generate_report.py
python analysis/make_paper_figures.py
```

Then copy `analysis/output/summary_metrics.csv` and `performance_report.md` into this folder if you want the bundle updated in git.

Optional per-capture plots (slower on huge files):

```bash
python analysis/analyze_motion.py demos/Saved/Logs/MotionCapture_20260427_134621.csv
# ...repeat for each CSV
```

Python deps: `pandas`, `numpy`, `matplotlib`, `scipy`, `scikit-learn` (see project README).

## `make_paper_figures.py` source filenames

`analysis/make_paper_figures.py` lists the canonical `MotionCapture_*.csv` names in a `CANONICAL` dict. If you re-capture, update that block and re-run the script.
