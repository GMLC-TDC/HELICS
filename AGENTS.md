# Repository Guidelines

## Project Context

HELICS is a multi-language, cross-platform C++ framework for connecting separate
simulators into a time-synchronized co-simulation. It coordinates federates; it does not
simulate the physical system itself.

Core concepts: federates participate in a federation through brokers and cores. Value
federates exchange physical values through publications, subscriptions, and named inputs.
Message federates exchange directed messages through endpoints and filters. Combination
federates use both value and message interfaces.

## HELICS Philosophy

The five HELICS design principles from the README are:

- Make it as easy as possible for federates of all kinds to work together.
- Federates cannot impose restrictions or requirements on other federates.
- Federates should maintain control and autonomy.
- The design should be layered and modular to be adaptable to a wide variety of
  circumstances.
- Centralized control should be minimized.

Use these as design checks. Prefer local, opt-in behavior; preserve federate autonomy;
and keep core, application API, shared library, and app behavior layered.

## Engineering Guardrails

- Keep fixes narrowly targeted. Avoid drive-by refactors, especially in core timing,
  message routing, serialization, public API, and application API layers.
- HELICS behavior often depends on distributed timing, grants, dependencies, and
  asynchronous federate progress. Tests for these changes should cover timing
  interactions, not just local state.
- Be careful with Windows/Linux differences, file paths, dynamic libraries, process
  memory APIs, integer widths, and static initialization.

## Build And Test Notes

- Common Windows build: `cmake --build build --config Debug --parallel 4`.
- Single target: `cmake --build build --config Debug --target <target>`.
- Test binaries are usually under `build/bin/<Config>/` on Windows and `build/bin/` in
  Linux coverage scripts.
- Normal CI skips GoogleTest names containing `ci_skip`.
- Coverage scripts skip names containing `nocov`; they do not automatically skip
  `ci_skip`.
- Sanitizer scripts skip `ci_skip` and `nosan`.
- Temporary diagnostics that should not run in CI or coverage should include both
  markers in the full GoogleTest name, for example `some_reproducer_ci_skip_nocov`, or
  one marker in the fixture name and the other in the test name.

Run focused tests when possible before broad suites. For C++ changes, also run
`git diff --check`.

## Public API And Versioning

Before changing public headers, C API functions, enums, properties, flags, configuration
names, query formats, network behavior, or app-facing CLI/config behavior, check
`docs/Public_API.md` and `CHANGELOG.md`.

HELICS follows Semantic Versioning. Within a major version, public APIs should remain
code compatible except for explicitly experimental interfaces. Patch releases should not
impact public API or network compatibility. Network compatibility is guaranteed only
within a single minor release.

Versioning-sensitive surfaces include:

- Application API headers, public core headers, app public headers, and C++98 headers
  listed in `docs/Public_API.md`.
- C shared library headers, especially `helics.h` and `helics_api.h`; this API is the
  primary driver of versioning decisions.
- Public numeric `HELICS_*` enum values, including properties, flags, options, errors,
  data types, and return codes.
- App-visible command-line/configuration behavior, query JSON, serialization, and network
  coordination behavior.

Guardrails: do not remove or rename public symbols, config keys, CLI flags, enum values,
or documented behavior without explicit direction and a deprecation plan. Do not renumber
public enums. Keep aliases small, documented, and tested. Keep mirrored/generated public
API files consistent. Update docs and `CHANGELOG.md` for public or user-visible changes.

## Windows PATH Fix

If MSBuild fails before compilation with `MSB6001` and a duplicate environment key
mentioning both `Path` and `PATH`, rerun from `cmd` with the mixed-case `Path` cleared
for the child process:

```bat
cmd /v:on /c "set Path=& cmake --build build --config Debug --parallel 4"
```

Use the same pattern for specific targets:

```bat
cmd /v:on /c "set Path=& cmake --build build --config Debug --target applicationApiTests --parallel 4"
```

Use normal build commands unless this exact `Path`/`PATH` collision appears.

## Documentation Pointers

- `README.md`: project overview and philosophy.
- `CHANGELOG.md`: notable changes and Semantic Versioning expectations.
- `docs/Public_API.md`: stable public API surfaces for versioning decisions.
- `docs/user-guide/`: concepts, installation, examples, and advanced topics.
- `docs/references/configuration_options_reference.md`: configuration options.
- `docs/references/api-reference/`: API reference material.
- `tests/helics/`: component tests by application API, core, system, apps, network,
  shared library, and webserver.
