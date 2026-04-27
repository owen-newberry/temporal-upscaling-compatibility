# Mac LLM: paper data & repo check

Paste this into Cursor/Claude on your Mac after `git pull` in the repo (`ase-capstone` or **`temporal-upscaling-compatibility`** — GitHub may redirect to the latter URL).

## Tracked vs gitignored (so you are not confused)

| Path | Tracked? |
|------|----------|
| `docs/ResearchPaper/paper_data/*` | Yes — tables + CSV copy for the paper |
| `docs/ResearchPaper/figures/*.png` | Yes — final figures |
| `analysis/output/*` | **No** — scratch; regenerate into `paper_data/` if needed |
| `demos/Saved/**` | **No** — Unreal logs stay on the capture machine unless you copy them |
| `docs/FP/*.mp4`, `docs/FP/*.pdf` | **No** — large local exports; use `finalpresentation.md` in git |

## Goals

1. Confirm **`docs/ResearchPaper/paper_data/`** and **`docs/ResearchPaper/figures/*.png`** exist and are current (or regenerate from `demos/Saved/Logs` if you copied the Windows `MotionCapture_*.csv` files over).
2. Install Python deps if needed: `pip install pandas numpy matplotlib scipy scikit-learn` (or `python3 -m venv .venv` from repo root and install into it).
3. If you re-capture or replace CSVs under `demos/Saved/Logs/`, re-run from repo root:
   - `python3 analysis/aggregate_metrics.py`
   - `python3 analysis/generate_report.py`
   - `python3 analysis/make_paper_figures.py` (after any filename edits in `analysis/make_paper_figures.py` → `CANONICAL` dict)
   - Copy `analysis/output/summary_metrics.csv` and `performance_report.md` into `docs/ResearchPaper/paper_data/`.
4. For Overleaf: upload **`docs/ResearchPaper/figures/*.png`**, use **`docs/ResearchPaper/paper_data/paper_tables.md`** and **`summary_metrics.csv`** for numeric tables, and read **`paper_data/performance_report.md`** for the narrative.
5. Do **not** expect `analysis/output/` to be in git; scratch outputs stay gitignored. The **source of truth in git** is `paper_data/` + `figures/`.
6. If `DEMOS_STANDALONE_DIAGNOSTICS` in `DemosDebug.h` is `0`, debug spew is off in C++; set to `1` only when diagnosing PIE/Standalone.

## Quick verify

```bash
cd ase-capstone
ls docs/ResearchPaper/figures
ls docs/ResearchPaper/paper_data
python3 -c "import pandas; import pathlib; p=pathlib.Path('docs/ResearchPaper/paper_data/summary_metrics.csv'); print('rows', len(p.read_text().splitlines())-1 if p.exists() else 'missing')"
```

## Raw CSVs (optional on Mac)

`demos/Saved/` is gitignored. If you need raw logs on the Mac, copy from Windows/OneDrive: `demos/Saved/Logs/MotionCapture_20260427_*.csv` (see `paper_data/README.md` for the list).
