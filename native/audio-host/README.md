# EasyKnack Audiohost (v9)

C++17 host for Windows and, as a developer port, Linux. The Windows backend uses
miniaudio 0.11.21 for WASAPI hardware devices and ActivateAudioInterfaceAsync for
process loopback. The Linux backend uses miniaudio's ALSA backend. The Java UI
communicates over authenticated loopback TCP. No GLSwing sources or Java runtime
are linked into the host.

## Direct sample access

The calculation files are `src/nodes/OscillatorNode.hpp`, `MathNode.hpp`, `GainNode.hpp` and
`OutputNode.hpp`. They inherit `NodeProcessor` and receive a `ProcessContext`:

```cpp
void process(ProcessContext& context) noexcept override {
    const float* input = context.inputOrSilence(0);
    float* output = context.output;
    for (std::size_t i = 0; i < context.bufferSize; ++i) {
        output[i] = input[i] * 0.5f;
    }
}
```

Samples are normally in [-1, 1]. Buffer pointers remain valid during the call;
do not retain them. State such as oscillator phase lives in the node instance.
`ProcessContext` only carries generic block data: input buffers, output buffers,
MIDI event blocks, actual frame count, sample rate and a shared silence buffer. `input(index)` returns
null for an unconnected input; a processor can then use its own parameter default.
Different processors keep different parameter/state fields in their own class.
`configure(std::istream&)` parses that node's protocol payload on the control thread;
`sources` lists its upstream indices. `inheritState(...)` copies only DSP state when
the graph is swapped, leaving newly configured parameters intact. The host calls it
only for matching instance IDs and processor IDs.
Do not allocate memory, use locks, log, access files or perform networking in
`process()`. The graph owns separate processor state for each stereo channel and
calls the existing sample-loop interface once per channel (`channelIndex`). Input
nodes read prepared endpoint buffers via the abstract `AudioIO`; output nodes sum
into their selected destination. Each destination applies the existing 0.2 master
level and start ramp and clips to [-1, 1]. Stop closes capture activity and stops
device streams immediately. Generators with identical inputs produce identical L/R.
The initial graph uses 48 kHz and blocks of at most 256 frames. Device callbacks
of other sizes are split; a node must honor the supplied count, including smaller
last blocks. Gain changes are smoothed; existing oscillator phase is preserved
when a new graph has the same node ID. Topology changes are not crossfaded yet.

## Build

Place each new processor header under `src/nodes` (subfolders work too), derive its
class from `NodeProcessor`, and declare `EASYKNACK_NODE(MyNode, "my-node")` inside
namespace `easyknack` at the end of the header. No host switch, enum or manual include
list is needed. `scripts/generate-native-nodes.ps1` (Windows) or `scripts/generate-native-nodes.py`
(POSIX) generates the factory and a `nodes.json` manifest. Duplicate IDs and
malformed declarations fail the build. Optional `.cpp` files in the same node
directory are discovered and compiled too. The PowerShell build and CMake invoke
the PowerShell generator; the Linux build script uses the Python one. Keep helpers
outside the registry by omitting the macro. A Java definition links its native ID and header
through `NativeProcessor`; Maven tests verify that link.

With LLVM-MinGW's `clang++` or MinGW `g++` on PATH, from the project root:

```powershell
./scripts/build-audio-host.ps1 -Cxx clang++
mvn -Pnative-audio verify
```

Set `-Daudio.cxx=C:/path/to/clang++.exe` for Maven. The script statically links the
C++ runtime and writes `target/native/easyknack-audio-host.exe`. A CMake project
is also provided for installed CMake/MSVC or MinGW toolchains:

```powershell
cmake -S native/audio-host -B target/audio-cmake
cmake --build target/audio-cmake --config Release
ctest --test-dir target/audio-cmake -C Release --output-on-failure
```

For a CMake build point `-Deasyknack.audio.host=...` at the resulting executable.
The portable compiler used during development is under `target/tools` only; it
is not a source dependency or included in the distribution.

### Linux (developer port)

Linux is not packaged or officially supported yet. The minimal port targets ALSA
playback and capture, normally directly or through `pipewire-alsa`. A system
`libasound.so.2`, a C++17 compiler, Python 3 and Maven are required.

```bash
mvn process-classes
./scripts/build-audio-host.sh --cxx g++ --test
# or let Maven build the host after generating the parameter contracts:
mvn -Pnative-audio-linux process-classes
```

The script writes `target/native/easyknack-audio-host` without an `.exe` suffix.
Use `-Daudio.cxx=clang++` (or `--cxx`) to select a compiler. The build is not
statically linked; use the distribution's runtime packages. The CMake project is
Windows-oriented and is not part of the Linux path yet.

`--null` selects miniaudio's null device and needs no audio hardware:

```bash
./target/native/easyknack-audio-host 0123456789abcdef --null
```

Accepted limitations of this milestone:

- no system or application loopback capture,
- no ASIO, application routing or virtual endpoints,
- no device volume control or volume-key hook (`VOLUME` returns unavailable),
- no hardware MIDI input; the on-screen keyboard still works,
- WAV recording only (MP3 import works, MP3 recording is Windows-only),
- no plugin or Bake source builds, no Linux installer or package.

Device identity is the ALSA device name, for example `out:default` or
`in:default`; the stable logical endpoints remain `default`. Saved explicit
hardware IDs can need reselection after a device rename. Duplicate names receive
`#2`, `#3`, ... suffixes. The protocol remains `READY 13` / `HELLO 13`.

## Execution and protocol

The application starts the host only on Audio → Start. The host opens an ephemeral
IPv4 loopback port, reports `READY 13 <port>` on stdout and accepts one client.
The session token is generated for each child process, passed as its first
argument, and required in `HELLO 13 <token>`; the reply is `OK 13`.
Version 3 adds explicit processing-rate metadata to every node record.
Rebuild the host together with Java; the handshake rejects old executables.

One UTF-8/ASCII line per request, maximum 65536 bytes:

- `GRAPH <count> <id> <osc|gain|out> <input-index|-1> <parameter> @ <a|c> <input-rates> ...`
  replaces the plan. IDs are unique, at most 64 bytes. Inputs refer to zero-based
  node indices; order is arbitrary. Oscillator parameter: Hz (20–20000); gain:
  dB (-80–6); output: 0. Maximum 256 nodes. Missing gain/output inputs mean silence.
  For oscillators the input supplies frequency in Hz per sample; without a wire
  the parameter is used. Incoming frequency is bounded to 0–20000 Hz (nonfinite
  values become 0), while the manual frequency slider accepts 20–20000 Hz.
- A math node extends the record to
  `<id> math <input-a|-1> <a> <input-b|-1> <b> <operation> <clamp:0|1>`.
  Operations: `multiply`, `add`, `subtract`, `divide`, `minimum`, `maximum`,
  `absolute`, `negate`. Unary operations require input B to be `-1`; its stored
  constant remains in the record for compatibility but does not affect the result.
  Disconnected inputs use the saved slider value (−1000000 to +1000000).
  Clamp limits each result to [-1, 1]. Division by |B| < 1e-12 returns 0;
  nonfinite math results return 0 even when Clamp is off.
- `ENDPOINTS`: `OK <five capability bits> <count> <i|o> <type> <hex-id> <hex-name> <available:0|1> <route:0|1> ...`.
  Capabilities are system capture, application capture, routing, virtual inputs, virtual outputs.
  Types are SystemMix=0, Application=1, HardwareInput=2, HardwareOutput=3, VirtualInput=4, VirtualOutput=5.
  Text uses lowercase UTF-8 hex; an empty string is `-`.
- Input: `<id> input <hex-endpoint-id> copy <mono|stereo> @ a -`.
- Output: `<id> deviceout <source-index|-1> <hex-endpoint-id> @ a a`.
  Old `out` records still target `default`. Unknown endpoints remain valid and produce silence.
- `ROUTING_STATUS`: `OK <hex-text>` describing unavailable selected endpoints, or `OK -`.
- `IOSTATS`: `OK <captured-frames> <played-frames> <ring-underruns> <ring-overruns>`.
  These diagnostics cover the currently prepared streams; replacing streams resets
  their counters. Stop/start resets frame positions. They are not hardware XRuns.
- `IO_CLOCK`: `OK timer`, `OK external` (ASIO), or
  `OK output <hex-endpoint-id> <native-period-frames> <native-sample-rate> <minimum-input-reserve-frames>`.
  The selected WASAPI output runs DSP directly without an output ring. Period is
  the negotiated device period, not measured end-to-end latency. Direct playback
  frames are included in `IOSTATS`; hardware dropouts are not ring underruns.
- `IO_BUFFERS`: `OK <hex-input-id> <largest-capture-packet> <queued-frames> <target-frames> ...`,
  or `OK -` without WASAPI inputs. Packet size is observed for device capture;
  process loopback reports 0. These are snapshots, not a latency measurement.
- `PLAY`, `STOP`: start/stop capture and output streams, enable/disable playback envelope.
- `VOLUMES`: `OK <keys-active:0|1> <hex-id> <scalar:0..1> <windows-muted:0|1> <signal-peak:0..1> ...`.
  Live WASAPI endpoint volumes and peak since the previous query; no document state.
- `VOLUME <hex-id> <scalar>`: set the Windows endpoint level. Positive levels also
  clear external endpoint mute. Missing devices return an error without stopping DSP.
- `VOLUME_LINKS [<hex-id> ...]`: replace the set of volume-key targets (max 32).
  Empty set releases the keys. Linking does not write levels. Hook queues up/down;
  control-thread polling changes the loudest target by two percentage points per
  step and scales the others proportionally. Ratios survive a joint reduction to
  zero until the target selection changes or the host exits.
  No OS changes occur in the null-device test backend.
- `VOLUME_KEY_EVENT`: `OK <sequence> <scalar> <target-count>` for the latest
  successfully applied volume-key change (scalar is the loudest target).
- `PING`: `OK <callback-count>`; used as a heartbeat and to check the device loop.
- `QUIT`: acknowledge, close the device and exit.

Successful commands return `OK`; invalid graphs/commands return `ERR <reason>`
and stop playback. Graph parsing and topological sorting run on the control
thread. Three ownership slots transfer prepared plans to the audio thread at
block boundaries; graph storage is reclaimed on the control thread. Cycles and
invalid references are rejected. Socket loss or a five-second read timeout ends
the host, so an abandoned process does not keep playing indefinitely.

`--null` as the second argument selects a silent device for integration tests.
Java coalesces graph updates every 100 ms. It resolves Core mute bypasses and
flattens composites before transfer; Hide does not alter sound. Audio is always
compiled from the root graph while the editor may navigate inside a composite.

## Current limits

One stereo signal port per node, at most 32 selected endpoints / 64 capture and output streams;
no ROUTE mode, virtual driver, ASIO, binary plugin loading, sample files or live C++ recompilation. C++ node
changes require rebuilding/restarting the host. Network transport is Windows
specific; the DSP classes are platform independent. The backend is a prototype,
not a measured low-latency engine; device latency depends on the driver.

Device API reference: https://miniaud.io/docs/manual/index.html
Vendored source and complete license: `third_party/miniaudio.h`.


## Processing rates (protocol v3)

Each node record ends with `@ <output rate> <input rates>`: `a` means audio,
`c` means control; use `-` for zero inputs. For example:
`GRAPH 2 m math -1 2 -1 3 multiply 0 @ c cc out out 0 0 @ a a`.
Control processors must override `supportsControlRate()` to return true.
They receive `context.bufferSize == 1`; their output sample is held for the block.
`context.blockFrames` is the actual elapsed frame count, and `sampleRate` remains
48000. Time-dependent control processors should advance by blockFrames/sampleRate.
An audio processor receiving a control input sees a held copy of its first sample.
All intermediate buffers are allocated when compiling the graph, outside the callback.



## Callback load statistics (protocol v4)

`STATS` returns `OK <average> <peak> <over-budget-count>`. The first two fields use
hundredths of a percent: 1250 means 12.50%. The average is smoothed over approximately
0.5 seconds; peak and count cover the lifetime of the host process. The denominator
is the actual callback frame count divided by the engine sample rate. Values can
exceed 100%. Time is measured using steady_clock around AudioEngine.render, including
mixing/output work. Statistics use lock-free atomic counters. They measure elapsed
DSP-clock callback execution time, not whole-machine CPU use or driver-reported XRuns.
Independent WASAPI callback work is not included. With WASAPI only, a silent
miniaudio clock drives the graph; bounded adaptive SPSC stereo rings connect streams.
With ASIO selected, one full-duplex driver callback clocks the graph directly.
See [external routing](../../docs/development/features/external-audio-routing.md) for setup and limits.

## ASIO endpoints (protocol v6)

Endpoint type ordinals 6/7 are ASIO input/output channel pairs. Their stable IDs
are `asio:{driver-clsid}:i:<first-channel>` and `asio:{driver-clsid}:o:<first-channel>`.
Channels are zero-based in IDs and one-based in display names. There is no implicit
fallback driver or automatic system-route change. Select at most one ASIO driver
per graph. Drivers are loaded through COM; no vendor driver is shipped here.
GPLv3 SDK notices: `third_party/asio/PROVENANCE.md`, `LICENSE.txt`, `COPYING`.


## MIDI and multiple outputs (protocol 7)

`MIDI_INPUTS` returns `OK <count> <unsigned-device-key> <hex-name> ...`.
`MIDI_PANIC` clears held MIDI state. WinMM devices are discovered on the control
thread; only reachable MIDI input nodes open a device. Duplicate references share
one capture queue. Callback arrival timestamps are translated to block-relative
sample positions; overflow/disconnection resets converter state. SysEx, MIDI 2.0
are not supported yet.

Source references pack `nodeIndex + 256 * outputIndex` (up to eight outputs).
`ProcessContext.outputAt(0)` aliases the existing output pointer; additional outputs
have preallocated buffers. Event ports use a separate bounded `MidiBlock`, not float
encoding. Processor metadata validates event-to-event connections. Converters are
mono-shareable, output at audio rate, and preserve held notes/controllers on graph
updates. MIDI device/generation or channel changes clear stale held state.

Processors: `filter input cutoffSource cutoffHz resonanceSource Q lowpass|highpass|bandpass`,
`midiin deviceKey`, `midinote midiSource channel bendRange`, `midicc midiSource channel cc`.
MIDI Note output slots: 0 frequency, 1 gate, 2 trigger, 3 velocity. Channel 0 means all.

## Polyphony and keyboard (protocol 8)

`midinote source channel bendRange [poly voices]` accepts 1–32 voices (default 1).
The graph propagates voice domains in topological order, allocates independent
processors/buffers per voice, and broadcasts mono inputs. Independent domains cannot
be zipped together. `voicemerge source` sums a domain into a mono/stereo signal;
audio sinks automatically sum their incoming domain. Allocation occurs on the control thread, with
at most 2048 active voice instances before stereo duplication. Each processor opts
into spawning a domain via `voices()` and receives `selectVoice(index)`; ordinary
processors require no polyphony-specific buffer API. Each call still processes one
voice/channel using the existing ProcessContext. Release tails persist until reuse.

`miditranspose source semitones` carries fractional semitone offsets separately in
MidiEvent.transpose. MIDI note identity is unchanged; offsets are sampled on NoteOn.
`mididelay source milliseconds` uses a bounded 4096-event queue, absolute sample due
times, and monotonic event ordering when reducing delay. Queue overflow/panic clears
pending events and releases downstream notes. Node state survives graph replacement.

Virtual device key 1 is reserved for the computer keyboard (0 means no device).
It is available even with --null and is added to the device picker by Java.
`MIDI_KEYBOARD count packedMessage ...` sends at most 256 standard packed MIDI
messages; message 255 flushes the keyboard queue. Java forwards events through the
existing single control executor immediately, without waiting for the periodic poll.
The capture queue timestamps arrival and preserves the same block placement rules
as WinMM. Handshake version 9 prevents an older host accepting incompatible graphs.

## Numeric value events (protocol 9)

Rate metadata now accepts `a`, `c`, and `e` for outputs and inputs. MIDI transport
uses `e` without scalar decimation. Numeric MIDI converters run with their full
block context and publish per-output change masks. Masks preserve both edges of
one-sample trigger pulses, including a falling edge in the next block.

`supportsEventRate()` opts in to Event output. MIDI producers opt in automatically.
`evaluatesOnEvents()` identifies pure numeric operations (Math/MapRange): the graph
unions the source change masks, samples all inputs at each shared timestamp, and
calls process with bufferSize=1. Changes of control-rate sources generate a boundary
event; audio-rate sources never drive the event schedule. Initial state and changed
node configuration trigger one evaluation. Unchanged graph publications preserve
held outputs and control snapshots. Equal results do not emit another value event.

Every voice/channel keeps separate held values. Fixed-size masks and pointer arrays
are allocated before publication. Materialized held sample buffers provide ordinary
audio compatibility and still incur buffer work. Unsupported event-rate processors
are rejected explicitly; wire compatibility continues to distinguish raw MIDI from
numeric signals. Source packing and .knack documents remain unchanged.


Graph validation errors return ERR and stop the backend without exiting the process.
The Java service treats GRAPH rejection as PAUSED and resumes after a valid replacement.
`mix a aValue b bValue factor factorValue` is the legacy linear format. The typed
parameter contract also carries `mode` (`linear` or `equalpower`). Both modes support
a/c/e rates. Factor is clamped to 0–1. Equal Power uses cosine/sine weights, preserving
expected power for uncorrelated sources; identical sources gain about 3 dB at the
midpoint. Mode changes are live parameter updates. Baked/fused exports retain the mode.
`unitconvert source value fromUnit toUnit` supports a/c/e rates and same-dimension units.
`voicemerge source` remains backward compatible. Extended payload:
`voicemerge source mix sum|mean|spread|pitch pitchSource width` (two rate metadata inputs).
Merge input buffers are arranged by source then voice; mono auxiliary sources broadcast.
Polyphonic auxiliary sources must have the same voice domain as the main signal.
No allocation occurs in the merge process callback. Sum is the output-node default.

`voicemerge source combine sum|mean` is the current one-input merge payload.
`pan audioSource panSource panValue` is an independent stereo processor; pan is −1..1.
It preserves voice domains until an explicit merge or sink, with channel-local gains.
Legacy `voicemerge ... mix ...` packets remain supported only for existing documents.


## Parameter contracts and analysis (protocol 13)

The application prefixes native payloads with `!<schema fingerprint>`. Java's `ParameterContract`
is the single declaration for encoding, input ordering and generated C++ parameter structs.
Run Maven through `process-classes` before a standalone native build, or use `-Pnative-audio`.
The handshake is `HELLO 13`; incompatible parameter schemas are rejected without terminating
host control. The existing unversioned parser is retained for diagnostic/legacy graph fixtures.

`ANALYSIS` returns `OK 2` followed by zero or more records:
`nodeId frequency confidence span symmetric valueCount <valueCount float values>`.
Frames contain up to 8192 values. The span is seconds for time/lag views and Hz for
spectral resource views; `symmetric` marks the positive-lag correlation representation.
Only fresh frames are returned. Scope/FFT processors publish into bounded SPSC queues; serialization
runs on the control thread. The callback never waits for the consumer and drops display frames if
necessary. An attached analysis node is a graph consumer even without an Audio Output connection.
The scope is an observer without an audio output. Autocorrelation uses an 8192-sample
mean-removed window and zero-padded FFT autocorrelation. The shared history/analysis
pipeline, worker latency and resource contracts are described in
[the DSP foundation notes](../../docs/development/FOUNDATION-REFORM.md).
# Block Delay / Feedback

`Effects > Block Delay` retains one prepared DSP block (currently 256 samples,
5.33 ms at 48 kHz). Its initial output is zero. Audio and Control sources are
accepted; output is Audio rate. Each stereo channel and polyphonic voice keeps
independent state. Partial callback blocks preserve the delay in samples.

Feedback needs at least one Block Delay on every cycle. An ordinary time-delay
node does not break graph scheduling dependencies. The host first publishes the
retained block, calculates its consumers, then captures the new input after the
whole graph has finished. Muting silences this node instead of bypassing it.
Use Math or Mix in the return path to set the feedback gain.

Live groups work; Bake/plugin export of groups containing Block Delay is currently
rejected explicitly until generated plans support the commit phase as well.
Sample-by-sample feedback uses **Effects > Sample Delay** instead. Its one-sample
delay and supported feedback islands are described in
[Sample Feedback](../../docs/development/features/SAMPLE-FEEDBACK.md).

## Manual parameter interpolation (protocol 13)

Optional metadata after a node's rate fields has this form:

```text
~ count [port enabled minMs maxMs target device channel cc low high reversed logarithmic pickup externalEcho B buffered bufferMs smoothing smoothingMs mapping]... ~
```

Only unconnected numeric defaults are interpolated. Physical sources retain their
actual rates and values. Ports 32 + 3 * input + axis are reserved for vector-editor
components; Vector Input uses input zero. Configuration-only and explicitly
discrete sliders disable interpolation. Device zero means no direct MIDI input.
The host retains per-lane timing and current values through parameter updates.
External GUI echoes of a directly handled MIDI CC do not restart the native ramp.
Control-rate processors sample the transition once per block. Settled defaults
retain zero-stride scalar access; active ramps use preallocated sample buffers.
No additional graph nodes, render-thread allocations or GUI animation are needed.

`mapping` is `-` for the ordinary MIDI range mapping, or `LUT` followed by 128
values when parameter/unit conversion must follow MIDI mapping. Tables are
prepared in Java and copied into preallocated lane storage on live updates.

Protocol 13 adds the `B` timing extension before `mapping`; unextended diagnostic
entries retain adaptive behavior. Continuous controls default to a 50 ms buffered
timeline and 5 ms boundary rounding in Java. Points use a preallocated 512-slot
queue per lane. Overflow coarsens the future tail without replacing the playback
head. Segments latch their endpoint treatment once rounding begins, so a late
successor never changes an already rendered boundary retrospectively. In adaptive
mode, rounding applies to initial/restart ramps only; ongoing ramps stay linear.
