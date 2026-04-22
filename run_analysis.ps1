# Run motion capture analysis on the latest CSV from a UE Play session.
# Usage: .\run_analysis.ps1
#        .\run_analysis.ps1 path\to\specific.csv

$py  = "$PSScriptRoot\.venv\Scripts\python.exe"
$script = "$PSScriptRoot\analysis\analyze_motion.py"

if (-not (Test-Path $py)) {
    Write-Error "Python venv not found at $py. Run: python -m venv .venv && .venv\Scripts\pip install pandas matplotlib scikit-learn scipy"
    exit 1
}

if ($args.Count -gt 0) {
    & $py $script $args[0]
} else {
    & $py $script
}
