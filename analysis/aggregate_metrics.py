"""
Aggregate all MotionCapture CSVs and compute summary metrics for each demo/mode.

For each (CSV, Demo, Mode) combination this script computes:

    Count, MeanDelta, StdDelta, MedianDelta, P95Delta, MaxDelta,
    MeanError, MaxError,
    MeanFPS, StdFrameTimeMs, MaxFrameTimeMs,
    P1_FPS, P01_FPS, Avg1LowFPS, Avg01LowFPS

FPS is derived from diff(TimeSeconds) within each actor's row-ordering, since
MotionLogger records one row per actor per frame. The slowest frames drive
the 1% low / 0.1% low metrics (F4.4 in the README).

Usage:
  python analysis/aggregate_metrics.py

Output:
  analysis/output/summary_metrics.csv
"""
import pathlib

import numpy as np
import pandas as pd


DEMO_MAP = {
    "Demo 1 - Motion Authority":  {"Authority", "Direct", "Dropped"},
    "Demo 2 - Fixed Timestep":    {"Fixed", "Variable"},
    "Demo 3 - Time-based Motion": {"TimeBased", "FrameBased"},
    "Demo 4 - Workload Budget":   {"Budgeted", "Unbudgeted"},
}


def classify_mode(mode: str) -> str | None:
    """Map an individual Mode value to its demo. Per-mode so a single CSV
    that captured data from multiple levels in one session is still split
    into the right demo buckets."""
    for demo, keys in DEMO_MAP.items():
        if mode in keys:
            return demo
    return None


def compute_frame_times(sub: pd.DataFrame) -> pd.Series:
    """Derive per-frame delta-time (seconds) from TimeSeconds within each actor.

    MotionLogger emits one row per actor per frame, so grouping by ActorName
    and diffing TimeSeconds recovers the frame pacing each actor observed.
    NaNs (first row per actor) are dropped.
    """
    if "TimeSeconds" not in sub.columns or "ActorName" not in sub.columns:
        return pd.Series(dtype=float)

    dt = (
        sub.sort_values(["ActorName", "TimeSeconds"])
           .groupby("ActorName")["TimeSeconds"]
           .diff()
    )
    dt = dt[(dt > 0) & (dt < 1.0)]  # ignore pauses/session gaps > 1s
    return dt


def low_fps_metrics(dt_series: pd.Series) -> dict:
    """Compute FPS aggregates from a series of frame-time deltas (seconds)."""
    if dt_series.empty:
        return {
            "MeanFPS": np.nan,
            "StdFrameTimeMs": np.nan,
            "MaxFrameTimeMs": np.nan,
            "P1_FPS": np.nan,
            "P01_FPS": np.nan,
            "Avg1LowFPS": np.nan,
            "Avg01LowFPS": np.nan,
        }

    fps = 1.0 / dt_series
    frame_ms = dt_series * 1000.0

    # Percentile-based lows: FPS value at the 1st / 0.1st percentile of the
    # sorted FPS distribution. Lower percentile = slower frame.
    p1  = float(np.percentile(fps, 1))
    p01 = float(np.percentile(fps, 0.1))

    # Mean-of-worst-N% lows: more common in game reviews. Take the slowest
    # 1% (or 0.1%) of frames by frame-time and average their FPS.
    def mean_worst(pct: float) -> float:
        n = max(1, int(round(len(dt_series) * pct)))
        worst = dt_series.nlargest(n)
        return float((1.0 / worst).mean())

    return {
        "MeanFPS":         float(fps.mean()),
        "StdFrameTimeMs":  float(frame_ms.std()),
        "MaxFrameTimeMs":  float(frame_ms.max()),
        "P1_FPS":          p1,
        "P01_FPS":         p01,
        "Avg1LowFPS":      mean_worst(0.01),
        "Avg01LowFPS":     mean_worst(0.001),
    }


def main() -> None:
    csv_dir = pathlib.Path("demos/Saved/Logs")
    # Skip anything prefixed `_archive_` — those are retired captures kept for
    # history (e.g. the pre-fix Sleep-based workload data, or mixed-mode
    # Demo 4 sessions that pre-date the split-capture methodology).
    csvs = sorted(
        p for p in csv_dir.glob("MotionCapture_*.csv")
        if not p.name.startswith("_archive_")
    )
    if not csvs:
        print(f"No MotionCapture_*.csv files found in {csv_dir}")
        return

    summary_rows: list[dict] = []
    for csv in csvs:
        try:
            df = pd.read_csv(csv)
        except Exception as exc:
            print(f"  skip {csv.name}: {exc}")
            continue

        df.columns = df.columns.str.strip()
        if "Mode" not in df.columns:
            continue
        df["Mode"] = df["Mode"].astype(str).str.strip()

        for mode in df["Mode"].unique():
            demo = classify_mode(mode)
            if demo is None:
                continue
            sub = df[df["Mode"] == mode]
            if sub.empty:
                continue

            row: dict = {
                "Demo":        demo,
                "CSV":         csv.name,
                "LevelName":   sub["LevelName"].iloc[0] if "LevelName" in sub.columns else "",
                "Mode":        mode,
                "Count":       len(sub),
                "MeanDelta":   float(sub["FrameDeltaCm"].mean()),
                "StdDelta":    float(sub["FrameDeltaCm"].std()),
                "MedianDelta": float(sub["FrameDeltaCm"].median()),
                "P95Delta":    float(sub["FrameDeltaCm"].quantile(0.95)),
                "MaxDelta":    float(sub["FrameDeltaCm"].max()),
            }

            if "PositionErrorCm" in sub.columns and sub["PositionErrorCm"].notna().any():
                row["MeanError"] = float(sub["PositionErrorCm"].mean())
                row["MaxError"]  = float(sub["PositionErrorCm"].max())
            else:
                row["MeanError"] = np.nan
                row["MaxError"]  = np.nan

            if "FrameWorkMs" in sub.columns:
                work = pd.to_numeric(sub["FrameWorkMs"], errors="coerce")
                if work.notna().any():
                    row["MeanWorkMs"] = float(work.mean())
                    row["MaxWorkMs"]  = float(work.max())
                    row["P95WorkMs"]  = float(work.quantile(0.95))
                else:
                    row["MeanWorkMs"] = np.nan
                    row["MaxWorkMs"]  = np.nan
                    row["P95WorkMs"]  = np.nan
            else:
                row["MeanWorkMs"] = np.nan
                row["MaxWorkMs"]  = np.nan
                row["P95WorkMs"]  = np.nan

            row.update(low_fps_metrics(compute_frame_times(sub)))
            summary_rows.append(row)

    summary = pd.DataFrame(summary_rows)
    out_dir = pathlib.Path("analysis/output")
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "summary_metrics.csv"
    summary.to_csv(out_path, index=False)

    if summary.empty:
        print("No rows produced.")
    else:
        pd.set_option("display.float_format", lambda x: f"{x:,.2f}")
        print(summary.to_string(index=False))
    print(f"\nWrote {out_path}")


if __name__ == "__main__":
    main()
