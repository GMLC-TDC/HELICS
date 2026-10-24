param(
    [double]$Duration = 100000,
    [int]$SubscriberInitialDelayMs = 0,
    [bool]$StopAfterWarning = $true,
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
$binaryDirectory = Join-Path $repoRoot "build\bin\$Configuration"
$runDirectory = Join-Path $binaryDirectory "issue2841-run"
$brokerPath = Join-Path $binaryDirectory "helics_broker.exe"
$publisherPath = Join-Path $binaryDirectory "helicsIssue2841Publisher.exe"
$subscriberPath = Join-Path $binaryDirectory "helicsIssue2841Subscriber.exe"

foreach ($path in @($brokerPath, $publisherPath, $subscriberPath)) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing executable: $path. Build the helicsIssue2841Publisher and helicsIssue2841Subscriber targets first."
    }
}

New-Item -ItemType Directory -Path $runDirectory -Force | Out-Null
$broker = $null
$publisher = $null
$subscriber = $null

try {
    $broker = Start-Process -FilePath $brokerPath `
        -ArgumentList @("-f", "2", "--loglevel=warning") `
        -WorkingDirectory $binaryDirectory -PassThru -WindowStyle Hidden `
        -RedirectStandardOutput (Join-Path $runDirectory "broker.stdout.log") `
        -RedirectStandardError (Join-Path $runDirectory "broker.stderr.log")
    Start-Sleep -Milliseconds 500

    $publisher = Start-Process -FilePath $publisherPath `
        -ArgumentList @($Duration.ToString([Globalization.CultureInfo]::InvariantCulture)) `
        -WorkingDirectory $binaryDirectory -PassThru -WindowStyle Hidden `
        -RedirectStandardOutput (Join-Path $runDirectory "publisher.stdout.log") `
        -RedirectStandardError (Join-Path $runDirectory "publisher.stderr.log")

    $subscriber = Start-Process -FilePath $subscriberPath `
        -ArgumentList @(
            $Duration.ToString([Globalization.CultureInfo]::InvariantCulture),
            "sub_fed.json",
            $SubscriberInitialDelayMs.ToString([Globalization.CultureInfo]::InvariantCulture)
        ) `
        -WorkingDirectory $binaryDirectory -PassThru -WindowStyle Hidden `
        -RedirectStandardOutput (Join-Path $runDirectory "subscriber.stdout.log") `
        -RedirectStandardError (Join-Path $runDirectory "subscriber.stderr.log")

    $warningObserved = $false
    while (-not $publisher.HasExited -or -not $subscriber.HasExited) {
        if ($StopAfterWarning -and (Test-Path -LiteralPath (Join-Path $runDirectory "subscriber.stdout.log"))) {
            $recentOutput = Get-Content -LiteralPath (Join-Path $runDirectory "subscriber.stdout.log") -Tail 100
            if ($recentOutput -match "queued future value buffers contain") {
                $warningObserved = $true
                foreach ($process in @($publisher, $subscriber)) {
                    if (-not $process.HasExited) {
                        Stop-Process -Id $process.Id -Force
                    }
                }
                break
            }
        }
        if (($publisher.HasExited -and $publisher.ExitCode -ne 0) -or
            ($subscriber.HasExited -and $subscriber.ExitCode -ne 0)) {
            break
        }
        Start-Sleep -Milliseconds 500
    }
    $publisher.WaitForExit()
    $subscriber.WaitForExit()
    if (-not $warningObserved -and ($publisher.ExitCode -ne 0 -or $subscriber.ExitCode -ne 0)) {
        throw "A federate failed. See logs in $runDirectory"
    }
} finally {
    foreach ($process in @($publisher, $subscriber, $broker)) {
        if ($null -ne $process -and -not $process.HasExited) {
            Stop-Process -Id $process.Id -Force
        }
    }
}

if ($warningObserved) {
    Write-Output "Warning detected; the run was stopped at the first threshold crossing. Logs: $runDirectory"
} else {
    Write-Output "Run completed. Logs: $runDirectory"
}
