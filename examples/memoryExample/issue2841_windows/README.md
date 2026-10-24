# HELICS issue 2841 Windows reproducer

This is a Windows-buildable copy of the downloaded 100,000-second C++ case. It
keeps the original 4,000-publication and 4,000-subscription JSON configurations,
and the publisher/subscriber exchange. Process working set and system memory are
reported through Windows APIs on Windows and procfs on Linux.

Build the two executables from the HELICS repository root:

```powershell
cmake --build build --config Debug --target helicsIssue2841Publisher helicsIssue2841Subscriber --parallel 4
```

Run a bounded case from PowerShell:

```powershell
.\examples\memoryExample\issue2841_windows\run_issue2841_windows.ps1 -Duration 10000 -SubscriberInitialDelayMs 1000
```

`-Duration` defaults to the original 100,000 steps. `-SubscriberInitialDelayMs`
defaults to zero; use it to let the publisher advance before the subscriber
starts requesting time. By default, the runner stops both federates after the
first queued-value warning. Pass `-StopAfterWarning $false` to let the chosen
horizon finish. The runner writes separate broker, publisher, and subscriber
logs under `build\bin\Debug\issue2841-run`.
