# Developer documentation

- [Code map](CODE-MAP.md): owning implementation and regression tests.
- [Build identity](BUILD-IDENTITY.md) and [Windows packaging](WINDOWS-PACKAGE.md).
- [Build storage](BUILD-STORAGE.md): disposable output, local tools and examples.
- [Core ownership](CORE-BOUNDARY.md): reusable mechanism versus audio policy.
- [Cleanup worklist](CLEANUP-PLAN.md): completed changes and open follow-ups.
- [Node documentation](NODE-DOCUMENTATION.md): bilingual reference maintenance.

`features/` retains technical feature references; current public behavior is
specified in the bilingual node and user references. `history/` preserves dated
plans and investigations and is not an implementation contract.

## Build languages

PowerShell orchestrates Windows/native builds, packaging and JDK selection. Shell
scripts provide the Linux developer build. C# implements the small Windows launcher
and packaging file helper. Python renders and validates documentation, generates
the portable native node registry and runs source/reference audits. These are
active build tools, separate from the Java application and C++ audio engine.
Assembly files under `native/audio-host/benchmarks/archive` retain compiler-output
evidence; they are not additional application implementations or build inputs.

## Linux developer build

Linux is a developer-only port in this milestone; there is no Linux installer or
official package. A C++17 compiler, Python 3 and Maven are required, plus ALSA
(usually directly or through `pipewire-alsa`).

```bash
mvn process-classes
./scripts/build-audio-host.sh --cxx g++ --test
```

`mvn -Pnative-audio-linux process-classes` runs the same shell build after the
generated parameter contracts exist. The host is written to
`target/native/easyknack-audio-host`. Use `--null` for hardware-free runs. Native
tests that assume Windows/PowerShell or static `.exe` binaries remain disabled in
that Maven profile; the shell `--test` mode runs `AudioGraphTest` and
`LinuxAudioBackendTest`. See the [audio-host README](../../native/audio-host/README.md)
for accepted limitations.
