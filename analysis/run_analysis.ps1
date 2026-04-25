# Run the full motion-capture analysis pipeline on UE Play session output.
#
# Usage:
#   .\run_analysis.ps1                    # analyze latest CSV + regenerate summary + report
#   .\run_analysis.ps1 path\to\file.csv   # analyze a specific CSV (summary/report still cover all)

$py          = "$PSScriptRoot\.venv\Scripts\python.exe"
$motion      = "$PSScriptRoot\analysis\analyze_motion.py"
$aggregate   = "$PSScriptRoot\analysis\aggregate_metrics.py"
$report      = "$PSScriptRoot\analysis\generate_report.py"

if (-not (Test-Path $py)) {
    Write-Error "Python venv not found at $py. Run: python -m venv .venv && .venv\Scripts\pip install pandas matplotlib scikit-learn scipy"
    exit 1
}

Push-Location $PSScriptRoot
try {
    Write-Host "== analyze_motion.py ==" -ForegroundColor Cyan
    if ($args.Count -gt 0) {
        & $py $motion $args[0]
    } else {
        & $py $motion
    }

    Write-Host "`n== aggregate_metrics.py ==" -ForegroundColor Cyan
    & $py $aggregate

    Write-Host "`n== generate_report.py ==" -ForegroundColor Cyan
    & $py $report

    Write-Host "`nDone. See analysis\output\ for summary_metrics.csv and performance_report.md." -ForegroundColor Green
}
finally {
    Pop-Location
}
