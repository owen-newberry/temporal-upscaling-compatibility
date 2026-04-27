"""
make_paper_figures.py — Final figure generator for the honors thesis paper.

Produces exactly four PNGs in docs/ResearchPaper/figures/, named to match the
paper's figure placeholders:

    demo1_jitter.png         FrameDeltaCm vs time (Direct vs Authority)
    demo2_positions.png      PosX vs time (Variable vs Fixed) — hitches visible
    demo3_schedule_error.png PositionErrorCm vs time (FrameBased vs TimeBased)
    demo4_frametime.png      FrameWorkMs vs time (Unbudgeted vs Budgeted)

Each demo reads from its canonical capture(s). Demo 4 merges two split-capture
CSVs (one per mode) because the Workload-Budget runs were captured separately
to keep each scenario's per-actor cost clean.

The styling targets a ~3.5 in column width, 2400 px wide, 300 dpi, with
consistent colors across figures: red = pattern violated, green = pattern
applied. Fonts and grid styling are unified.

Run after aggregate_metrics.py from the repo root:

    python analysis/make_paper_figures.py
"""
from __future__ import annotations

import pathlib
import sys

import matplotlib.pyplot as plt
import pandas as pd


REPO_ROOT  = pathlib.Path(__file__).resolve().parent.parent
LOGS_DIR   = REPO_ROOT / "demos" / "Saved" / "Logs"
# Final paper figures live under docs/ResearchPaper/figures so they are tracked
# in git (analysis/output is gitignored as a generated scratch dir).
OUTPUT_DIR = REPO_ROOT / "docs" / "ResearchPaper" / "figures"

COLOR_BAD  = "#e74c3c"
COLOR_GOOD = "#2ecc71"

MODE_COLOR = {
    "Direct":     COLOR_BAD,  "Authority": COLOR_GOOD,
    "Variable":   COLOR_BAD,  "Fixed":     COLOR_GOOD,
    "FrameBased": COLOR_BAD,  "TimeBased": COLOR_GOOD,
    "Unbudgeted": COLOR_BAD,  "Budgeted":  COLOR_GOOD,
}

# Use the current clean Standalone/PIE session captures (ase-capstone, Apr 2026).
# If you re-capture, update these filenames to match MotionCapture_*.csv in
# demos/Saved/Logs/ (Demo 4 is two split files: unbudgeted + budgeted levels).
CANONICAL = {
    "demo1": {"csv": "MotionCapture_20260427_135151.csv", "modes": ("Direct", "Authority")},
    "demo2": {"csv": "MotionCapture_20260427_134621.csv", "modes": ("Variable", "Fixed")},
    "demo3": {"csv": "MotionCapture_20260427_135413.csv", "modes": ("FrameBased", "TimeBased")},
    "demo4": {
        "csvs": (
            "MotionCapture_20260427_142806.csv",  # Unbudgeted
            "MotionCapture_20260427_142355.csv",  # Budgeted
        ),
        "modes": ("Unbudgeted", "Budgeted"),
    },
}


def apply_style() -> None:
    plt.rcParams.update({
        "figure.dpi":        300,
        "savefig.dpi":       300,
        "font.family":       "DejaVu Sans",
        "font.size":         9,
        "axes.titlesize":    10,
        "axes.labelsize":    9,
        "legend.fontsize":   8,
        "xtick.labelsize":   8,
        "ytick.labelsize":   8,
        "axes.grid":         True,
        "grid.alpha":        0.25,
        "grid.linewidth":    0.5,
        "lines.linewidth":   0.9,
        "axes.spines.top":   False,
        "axes.spines.right": False,
    })


def load_csv(name: str) -> pd.DataFrame:
    path = LOGS_DIR / name
    if not path.exists():
        sys.exit(f"Missing capture: {path}")
    df = pd.read_csv(path)
    df.columns = df.columns.str.strip()
    df["Mode"] = df["Mode"].astype(str).str.strip()
    return df


WARMUP_SECONDS = 1.0  # discard the first second of capture to skip spawn-frame
                      # artifacts (e.g. FrameDeltaCm computed against an
                      # uninitialized previous position) and shader-compile
                      # hitches that are not part of the experimental signal.


def normalize_time(df: pd.DataFrame, warmup: float = WARMUP_SECONDS) -> pd.DataFrame:
    """Zero TimeSeconds at the first sample and discard a warm-up window."""
    if df.empty:
        return df
    t0 = df["TimeSeconds"].min()
    df = df.copy()
    df["TimeSeconds"] = df["TimeSeconds"] - t0
    if warmup > 0:
        df = df[df["TimeSeconds"] >= warmup].copy()
        df["TimeSeconds"] = df["TimeSeconds"] - warmup
    return df


def plot_series(ax, df: pd.DataFrame, mode: str, y_col: str, label: str | None = None) -> None:
    sub = df[df["Mode"] == mode]
    if sub.empty:
        return
    color = MODE_COLOR.get(mode, "#7f8c8d")
    for _, group in sub.groupby("ActorName"):
        ax.plot(
            group["TimeSeconds"], group[y_col],
            color=color, alpha=0.85,
            label=label if label is not None else None,
        )
        label = None  # only one legend entry per mode


def fig_demo1_jitter() -> None:
    spec = CANONICAL["demo1"]
    df = normalize_time(load_csv(spec["csv"]))
    bad, good = spec["modes"]

    fig, ax = plt.subplots(figsize=(8, 3.2))
    plot_series(ax, df, bad,  "FrameDeltaCm", label=bad)
    plot_series(ax, df, good, "FrameDeltaCm", label=good)

    ax.set_title("Demo 1 — Single-Writer Motion Authority: per-frame jitter")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Frame delta (cm)")
    ax.legend(loc="upper right", frameon=False)
    fig.tight_layout()
    save(fig, "demo1_jitter.png")


def fig_demo2_positions() -> None:
    spec = CANONICAL["demo2"]
    df = normalize_time(load_csv(spec["csv"]))
    bad, good = spec["modes"]

    fig, ax = plt.subplots(figsize=(8, 3.2))
    plot_series(ax, df, bad,  "PosX", label=bad)
    plot_series(ax, df, good, "PosX", label=good)

    ax.set_title("Demo 2 — Fixed-Timestep Simulation: position under hitches")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Position X (cm)")
    ax.legend(loc="upper right", frameon=False)
    fig.tight_layout()
    save(fig, "demo2_positions.png")


def fig_demo3_schedule_error() -> None:
    spec = CANONICAL["demo3"]
    df = normalize_time(load_csv(spec["csv"]))
    df["PositionErrorCm"] = pd.to_numeric(df["PositionErrorCm"], errors="coerce")
    df = df[df["PositionErrorCm"].notna()]
    bad, good = spec["modes"]

    fig, ax = plt.subplots(figsize=(8, 3.2))
    plot_series(ax, df, bad,  "PositionErrorCm", label=bad)
    plot_series(ax, df, good, "PositionErrorCm", label=good)

    ax.set_title("Demo 3 — Time-Based Animation: schedule error vs ground truth")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Position error (cm)")
    ax.legend(loc="upper right", frameon=False)
    fig.tight_layout()
    save(fig, "demo3_schedule_error.png")


def fig_demo4_frametime() -> None:
    spec = CANONICAL["demo4"]
    bad_csv, good_csv = spec["csvs"]
    bad, good = spec["modes"]

    df_bad  = normalize_time(load_csv(bad_csv))
    df_good = normalize_time(load_csv(good_csv))

    fig, ax = plt.subplots(figsize=(8, 3.2))
    plot_series(ax, df_bad,  bad,  "FrameWorkMs", label=bad)
    plot_series(ax, df_good, good, "FrameWorkMs", label=good)

    ax.set_title("Demo 4 — Bounded Workload Budgeting: per-frame work cost")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Work per frame (ms)")
    ax.legend(loc="upper right", frameon=False)
    fig.tight_layout()
    save(fig, "demo4_frametime.png")


def save(fig, name: str) -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out = OUTPUT_DIR / name
    fig.savefig(out, bbox_inches="tight")
    plt.close(fig)
    print(f"Wrote {out}")


def main() -> None:
    apply_style()
    fig_demo1_jitter()
    fig_demo2_positions()
    fig_demo3_schedule_error()
    fig_demo4_frametime()


if __name__ == "__main__":
    main()
