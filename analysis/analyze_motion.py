"""
Motion Capture Analysis — ASE Capstone
======================================
Loads a MotionCapture_*.csv produced by FMotionLogger during a UE5 Play session
and outputs:
  1. Position over time per actor (colored by mode)
  2. Frame delta (jitter) over time per actor
  3. Per-mode summary statistics table
  4. Anomaly detection (IsolationForest) — flags jitter outlier frames
  5. Delta distribution comparison (Direct vs Authority)

Usage:
  python analyze_motion.py                        # auto-picks latest CSV
  python analyze_motion.py path/to/capture.csv    # specific file

Output:
  analysis/output/<stem>_positions.png
  analysis/output/<stem>_jitter.png
  analysis/output/<stem>_distribution.png
  analysis/output/<stem>_anomalies.csv
"""

import sys
import glob
import pathlib
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from scipy import stats
from sklearn.ensemble import IsolationForest

# ── Config ────────────────────────────────────────────────────────────────────
SAVED_LOGS_DIR = pathlib.Path(__file__).parent.parent / "demos" / "Saved" / "Logs"
OUTPUT_DIR     = pathlib.Path(__file__).parent / "output"
ANOMALY_CONTAMINATION = 0.05   # expected fraction of anomalous frames
COLORS = {"Authority": "#2ecc71", "Direct": "#e74c3c"}

# ── Helpers ───────────────────────────────────────────────────────────────────
def find_latest_csv() -> pathlib.Path:
    pattern = str(SAVED_LOGS_DIR / "MotionCapture_*.csv")
    files = sorted(glob.glob(pattern))
    if not files:
        sys.exit(f"No MotionCapture_*.csv found in {SAVED_LOGS_DIR}\n"
                 "Run the demo in the UE editor first.")
    return pathlib.Path(files[-1])


def load(path: pathlib.Path) -> pd.DataFrame:
    df = pd.read_csv(path)
    df.columns = df.columns.str.strip()
    df["Mode"] = df["Mode"].str.strip()
    return df


def print_stats(df: pd.DataFrame):
    print("\n── Per-Mode Summary ─────────────────────────────────────────────")
    grouped = df.groupby("Mode")["FrameDeltaCm"]
    summary = grouped.agg(
        Count="count",
        Mean="mean",
        Std="std",
        Median="median",
        P95=lambda x: x.quantile(0.95),
        Max="max"
    ).round(4)
    print(summary.to_string())

    # t-test between modes
    modes = df["Mode"].unique()
    if len(modes) == 2:
        a = df[df["Mode"] == modes[0]]["FrameDeltaCm"]
        b = df[df["Mode"] == modes[1]]["FrameDeltaCm"]
        t, p = stats.ttest_ind(a, b, equal_var=False)
        print(f"\nWelch t-test ({modes[0]} vs {modes[1]}): t={t:.4f}, p={p:.6f}")
        if p < 0.05:
            print("  → Statistically significant difference (p < 0.05)")
        else:
            print("  → No statistically significant difference")
    print()


# ── Plots ──────────────────────────────────────────────────────────────────────
def plot_positions(df: pd.DataFrame, stem: str):
    fig, ax = plt.subplots(figsize=(12, 5))
    for actor, group in df.groupby("ActorName"):
        mode  = group["Mode"].iloc[0]
        color = COLORS.get(mode, "gray")
        ax.plot(group["TimeSeconds"], group["PosX"],
                color=color, alpha=0.8, linewidth=1,
                label=f"{actor} ({mode})")

    # Legend deduplicated by mode
    handles = [plt.Line2D([0], [0], color=c, linewidth=2, label=m)
               for m, c in COLORS.items() if m in df["Mode"].values]
    ax.legend(handles=handles, loc="upper left")
    ax.set_title("Position X over Time — Authority vs Direct")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Position X (cm)")
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    out = OUTPUT_DIR / f"{stem}_positions.png"
    fig.savefig(out, dpi=150)
    print(f"Saved: {out}")
    plt.close(fig)


def plot_jitter(df: pd.DataFrame, stem: str):
    fig, ax = plt.subplots(figsize=(12, 5))
    for actor, group in df.groupby("ActorName"):
        mode  = group["Mode"].iloc[0]
        color = COLORS.get(mode, "gray")
        ax.plot(group["TimeSeconds"], group["FrameDeltaCm"],
                color=color, alpha=0.7, linewidth=0.8)

    handles = [plt.Line2D([0], [0], color=c, linewidth=2, label=m)
               for m, c in COLORS.items() if m in df["Mode"].values]
    ax.legend(handles=handles, loc="upper right")
    ax.set_title("Frame Delta (Jitter) over Time — Authority vs Direct")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Frame Delta (cm)")
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    out = OUTPUT_DIR / f"{stem}_jitter.png"
    fig.savefig(out, dpi=150)
    print(f"Saved: {out}")
    plt.close(fig)


def plot_distribution(df: pd.DataFrame, stem: str):
    fig, ax = plt.subplots(figsize=(9, 5))
    for mode, color in COLORS.items():
        subset = df[df["Mode"] == mode]["FrameDeltaCm"]
        if subset.empty:
            continue
        ax.hist(subset, bins=60, color=color, alpha=0.6, label=mode, density=True)

        # KDE overlay
        kde_x = np.linspace(subset.min(), subset.max(), 300)
        kde   = stats.gaussian_kde(subset)
        ax.plot(kde_x, kde(kde_x), color=color, linewidth=2)

    ax.set_title("Frame Delta Distribution — Authority vs Direct")
    ax.set_xlabel("Frame Delta (cm)")
    ax.set_ylabel("Density")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    out = OUTPUT_DIR / f"{stem}_distribution.png"
    fig.savefig(out, dpi=150)
    print(f"Saved: {out}")
    plt.close(fig)


# ── Anomaly Detection ─────────────────────────────────────────────────────────
def detect_anomalies(df: pd.DataFrame, stem: str) -> pd.DataFrame:
    results = []
    for actor, group in df.groupby("ActorName"):
        features = group[["FrameDeltaCm"]].values
        clf = IsolationForest(contamination=ANOMALY_CONTAMINATION, random_state=42)
        preds = clf.fit_predict(features)
        anomaly_rows = group.copy()
        anomaly_rows["Anomaly"] = (preds == -1)
        results.append(anomaly_rows)

    out_df = pd.concat(results)
    anomalies = out_df[out_df["Anomaly"]]
    out_path = OUTPUT_DIR / f"{stem}_anomalies.csv"
    anomalies.to_csv(out_path, index=False)

    print(f"\n── Anomaly Detection (IsolationForest, contamination={ANOMALY_CONTAMINATION}) ─")
    for mode in out_df["Mode"].unique():
        subset = out_df[out_df["Mode"] == mode]
        n_total    = len(subset)
        n_anomalies = subset["Anomaly"].sum()
        print(f"  {mode:12s}: {n_anomalies:4d} / {n_total:6d} frames flagged "
              f"({100 * n_anomalies / n_total:.1f}%)")
    print(f"Anomalous rows saved: {out_path}")
    return out_df


# ── Main ──────────────────────────────────────────────────────────────────────
def main():
    csv_path = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else find_latest_csv()
    print(f"Loading: {csv_path}")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    stem = csv_path.stem

    df = load(csv_path)
    print(f"Rows: {len(df):,} | Actors: {df['ActorName'].nunique()} | "
          f"Modes: {df['Mode'].unique().tolist()}")

    print_stats(df)
    plot_positions(df, stem)
    plot_jitter(df, stem)
    plot_distribution(df, stem)
    detect_anomalies(df, stem)

    print("\nDone.")


if __name__ == "__main__":
    main()
