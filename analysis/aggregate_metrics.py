"""
Aggregate all MotionCapture CSVs and compute summary metrics for each demo/mode.
Outputs a CSV suitable for paper tables and further analysis.

Usage:
  python aggregate_metrics.py

Output:
  analysis/output/summary_metrics.csv
"""
import pathlib
import pandas as pd
import numpy as np

csv_dir = pathlib.Path('demos/Saved/Logs')
csvs = sorted(csv_dir.glob('MotionCapture_*.csv'))

summary_rows = []
for csv in csvs:
    try:
        df = pd.read_csv(csv)
    except Exception:
        continue
    df.columns = df.columns.str.strip()
    df['Mode'] = df['Mode'].str.strip()
    demo = None
    modes = df['Mode'].values
    if 'Authority' in modes or 'Direct' in modes:
        demo = 'Demo 1'
    elif 'Fixed' in modes or 'Variable' in modes:
        demo = 'Demo 2'
    elif 'TimeBased' in modes or 'FrameBased' in modes:
        demo = 'Demo 3'
    elif 'Budgeted' in modes or 'Unbudgeted' in modes:
        demo = 'Demo 4'
    else:
        continue
    for mode in df['Mode'].unique():
        sub = df[df['Mode'] == mode]
        row = {
            'Demo': demo,
            'CSV': csv.name,
            'Mode': mode,
            'Count': len(sub),
            'MeanDelta': sub['FrameDeltaCm'].mean(),
            'StdDelta': sub['FrameDeltaCm'].std(),
            'MedianDelta': sub['FrameDeltaCm'].median(),
            'P95Delta': sub['FrameDeltaCm'].quantile(0.95),
            'MaxDelta': sub['FrameDeltaCm'].max(),
        }
        if 'PositionErrorCm' in sub.columns and sub['PositionErrorCm'].notna().any():
            row['MeanError'] = sub['PositionErrorCm'].mean()
            row['MaxError'] = sub['PositionErrorCm'].max()
        else:
            row['MeanError'] = np.nan
            row['MaxError'] = np.nan
        summary_rows.append(row)

summary = pd.DataFrame(summary_rows)
pathlib.Path('analysis/output').mkdir(parents=True, exist_ok=True)
summary.to_csv('analysis/output/summary_metrics.csv', index=False)
print(summary)
