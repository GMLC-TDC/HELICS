# CI/CD Infrastructure

This page describes the services that support HELICS development and releases. The active build matrix is documented in [Continuous Integration](continuous-integration.md); repository workflow files remain the source of truth for job definitions and triggers.

## Continuous Integration

HELICS uses Azure Pipelines for its primary Linux, macOS, and Windows build matrix. CircleCI adds compiler, install/package, ARM64, and scheduled Linux jobs. GitHub Actions runs static analysis, MSYS2, C++26, coverage, sanitizer, benchmark, release, Docker, and interface-generation workflows. AppVeyor provides the Cygwin build, and Cirrus CI provides the FreeBSD build.

The Linux jobs that use container images are defined by those images and the repository scripts. Windows and macOS jobs use their hosted runner images and install their dependencies in the pipeline. Scheduled jobs run from `main`; pull-request and push triggers are defined individually by each pipeline or workflow.

## Documentation and automation

Read the Docs builds and hosts the documentation. GitHub Actions also automates generation of SWIG interface files, release checklists, Docker images, benchmark artifacts, and release artifacts. A published GitHub release triggers the release-artifact workflow and downstream version-update notification.

## Additional checks

Codacy and pre-commit checks may report formatting, spelling, and static-analysis findings. They are useful maintenance signals but are not part of the required CI build matrix.
