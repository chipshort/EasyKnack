# Release notes

## Unreleased - 0.3.1

- Add a developer-only Linux port of the Java UI and native host. Build the host
  without PowerShell via `scripts/generate-native-nodes.py` and
  `scripts/build-audio-host.sh`, with an optional `native-audio-linux` Maven
  profile. Playback and capture use miniaudio's ALSA backend, the host keeps the
  existing graph/DSP engine, WAV recording and import, and the on-screen MIDI
  keyboard. Linux has no installer or package and is not officially supported.
- Linux limitations: no system/application loopback capture, ASIO, application
  routing, device volume or volume-key hook, hardware MIDI input, MP3 recording or
  plugin/Bake source builds. WAV recording is the supported Linux format.
- Auto-arrange example graphs and inner groups, centering their saved cameras.
- Replace the bundled examples with sixteen self-contained learning projects covering
  basics, buffers, MIDI, Instance/Spectral/Formula Groups and delayed feedback.
- Include German and English listening exercises; retire the earlier examples
  while preserving edited development copies in a hidden backup.

Compatibility:

- The host protocol remains version 13 and the `.knack` format is unchanged.
- Windows keeps the WASAPI/ASIO backends, device volume and plugin builds.

## 0.3.0 - 2026-11-18

- Expand Formula with typed user functions, bounded for loops, final locals and
  parameters, input-independent const expressions, chosen-base log and real root.
- Improve Formula completion, diagnostics, parameter hints, formatting and
  declaration navigation. Show applied/draft/error state and persistent node warnings.
- Add a standalone bilingual Formula language reference and a linked function index.
  Built-in function navigation opens the relevant entry in the selected help language.
- Share Find across text, code and documentation, with marked matches, matching
  options and forward/backward navigation. Remove empty component-name tooltips.
- Speed up eligible Instance Group oscillator Bakes, including Formula weights and
  audio-rate modulation. Reuse unchanged Bakes and show progress and fallback status.
- Give Envelope separate Attack, Decay and Release durations, editable start/end
  values and selectable retrigger behavior. Show curve controls on hover/focus only.
- Hold MIDI Note Frequency and Velocity after note-off for release tails; add Voice.
  Muted Digital Noise forwards Frequency and emits neutral Factor 1.
- Preserve connected port types when grouping; keep graph arrangement compact while
  respecting rough placement. Restore focused/error connection outlines and export geometry.
- Edit recording names directly, number takes only on collision, and use dedicated
  Projects/Recordings folders. Bundled examples stay in the installation.
- Improve Sine Cloud frequency placement, maximum bin spacing and period reporting.

Compatibility:

- Older Envelope timing is migrated; existing duration wires drive Attack and retired
  sustain-position bindings are removed. Review older modulated Envelope patches.
- MIDI patches using Velocity as a gate should use Gate instead.
- Sine Cloud frequency placement, tone count and periods can differ from older
  versions; Gaussian density is removed. Review saved research stimuli.
- Formula identifiers named final, const or root must be renamed because these names
  are now reserved. Existing document formats remain readable.

## 0.2.0 - 2026-11-14

- Make Mint the default; offer exactly Mint, Amber, Light Mint and Light Amber.
  Migrate retired Classic/Blue choices to Mint and old Light to Light Mint. Add
  Light Mint and Light Amber with lighter type headers and dark title text.
  Mint and Amber provide coordinated control accents and
  moderately saturated pastel type colors. Compact controls, Control-rate
  diamonds, wires, editor groups, popup menus and tooltips use borderless styling;
  titles and sliders have flat fills, with titles extending to the node edges.
  Audio-rate socket sizes and checkbox check colors retain their original appearance. Connected port lists align flush with
  adjacent buttons without rounded shared corners through updated GLSwing.
  Round socket fills retain their relative size across zoom levels. Node bodies
  remain neutral; the retired tinted-body study falls back to Mint. Theme geometry may change node
  dimensions; graph content and positions are preserved.

- Use almost square one-pixel corners across all four themes and straight wires
  with zero curvature by default. Node editors gain one pixel of padding on each
  side. Explicitly saved curvature preferences remain unchanged. Toggling the inspector preserves the graph
  position on screen at every zoom.

- Route unhandled keyboard commands through a shared focus/hover/window-default
  manager. The workspace remains reachable without a focused control; text input
  and open menus keep priority. Remove slider side-button insets in Mint and Amber.

- Clarify which compiler components are actually shipped and document recipient
  source access, with a preliminary release-license and interface-origin review.
  Documentation only; existing 0.1.0 downloads are unchanged.

## 0.1.0 - 2026-11-13

First Windows x64 release. Requires a separately installed Windows x64 JDK 25
(including javac); setup checks it and offers assistance. The installer is
unsigned. Build identifiers explicitly identify this as a local, manually built
release, not an official CI run.

- Visual audio graphs with Instance, Spectral and Partial Groups, Formula,
  spectral analysis/synthesis, MIDI and native Bake/source-plugin workflows.
- Bilingual offline manuals and detailed node references, with independent
  documentation language selection.
- Background startup preparation, one setup window, compiler archive installation
  and repair/uninstall support.
- Experimental algorithms remain opt-in per document; Pitch Hold accuracy is
  not guaranteed. Real-device and clean-machine acceptance limits accompany
  the downloadable validation report.

Compatibility: this first 0.x release includes earlier pre-release contract and
Complex Oscillator stretch changes described below. Preserve old documents and
rebuild/re-export older Bakes and source plugins with the matching application.

### Changes included in this first release

- Retry transient Windows sharing/access locks when publishing compiled plugin
  directories. Keep existing destinations intact and fail explicitly if a lock
  persists; verified with a real child-process directory lock.

- Package the installer compiler as one verified archive, with build-time disk
  costing, repair and transactional publication. Prepare extraction before
  removing an older installation; portable images stay extracted and startup
  performs no unpacking. Retain the archive locally for repair. Covered by 0.1.0.

- Bundle a tested Windows x64 LLVM-MinGW runtime compiler profile instead of the
  complete multi-architecture/debugger distribution. Keep shared C/C++ headers,
  x64 libraries, compiler configuration, runtime dependencies and license notices;
  omit ARM/32-bit targets and Python/LLDB. Formula, Bake and source plugins retain
  their native x64 compilation path. Covered by 0.1.0 development.

- Wait for forced audio-host termination before releasing its process reference,
  avoiding a Windows directory-lock race when publishing freshly validated Bakes
  and plugins. Covered by the current 0.1.0 development scope.

- Validate Windows x64 JDK 25 and its compiler before EXE/MSI installation copies
  files. Reuse launcher discovery and explicit selection/installation choices;
  unattended setup fails without a compatible JDK and never downloads one.
  Cancelling stops setup; uninstall does not require Java. Existing launcher
  checks remain. Covered by the current 0.1.0 development scope.

- Unify the Windows EXE installer into one setup window for license, options,
  progress, repair/removal and application launch. Hide the nested MSI wizard and
  its Apps entry; retain standalone MSI UI. Preserve shortcut/folder choices and
  close setup when launching EasyKnack. Compiler archive packaging reduces MSI costing.

- Show a nonblocking loading state when help or Formula is opened before
  background preparation completes, then reveal the requested content automatically.
  Closed editors stay closed; preparation failures end the loading state.

- Shorten the blocking startup path: automatically prepare both help languages,
  Formula/parser and font caches in the background after the workspace is visible;
  examples generate/scan on a separate low-priority worker. Font warming owns an
  offscreen GL context and avoids blocking the shared font cache during rasterization.
  Installed plugins still register before document loading. No document, plugin ABI
  or audio protocol change; covered by 0.1.0 development.

- Name Windows installers `EasyKnack-Setup-<version>-x64` to distinguish setup
  from the application launcher. Clarify the preparation message and document
  Windows uninstall/maintenance access; installation identity is unchanged.

- Establish `0.1.0-SNAPSHOT` as the development version targeting the first
  EasyKnack release `0.1.0`, covering the accumulated features and compatibility
  changes below. GLSwing stays independently versioned at `0.2.0-SNAPSHOT`.
  This version decision does not publish or tag a release; older artifacts retain
  their original identities.

- Show the current Maven product version for IDE launches alongside `IDE / unpackaged`,
  without reusing stale package identities or allocating a build number.

- Accept `buffer.writepos` in Formula Groups as a compatible alias of the absolute
  `buffer.writePosition` sample position; complete both Formula language references.
  Existing documents, buffer semantics and protocol versions are unchanged. Covered
  by the current development version before the first release.

- Fix missing text on repeated graph image exports with context-owned capture fonts in the external GLSwing dependency; verify the repeated capture in the packaged application smoke test.

- Include both offline manuals and full node references beside the Windows application; correct the external quick-start links and check bundled help during package smoke validation.

- Consolidate workflow guides into twelve concise German/English chapters; retain the detailed bilingual node tables and keep old workflow links through forwarding pages. Help now opens the independently selected manual language.
- Standardize visible Multiple Group naming as Instance Group (Instanzgruppe); saved document identifiers and plugin/audio contracts remain unchanged.

- Add a persisted documentation-language selector in the native help viewer (Same as application, Deutsch, English). Local links, reference/index actions and reopened help follow this preference independently of the UI language; untranslated pages use a visible English fallback.
- Consolidate repeated spectral definitions and viewer limits into canonical node references, retaining workflow examples and correcting the stale FFT-window range in the guide. Covered by the current first-development version.


- Store experimental-feature availability per document. Infer it for legacy files containing experimental nodes, retain explicit saved decisions, and confirm disabling while experimental nodes remain. Remove the global settings control. Existing nodes keep their execution.
- Ignore viewport camera-only changes in unsaved-content checks, while retaining cameras in saved files and preserving dirty detection for model edits, node movement and resizing. No document schema or audio protocol change; current first-development version covers this follow-up.


- Preserve stored internal numeric fallbacks at socket-only group boundaries when feedback wires are ignored. Keep explicit public editor defaults.
- Add an opt-in Experimental features setting (off by default), including dependent groups, examples and newly exported plugins. Mark Pitch Hold experimental; existing documents retain their nodes and execution. No document/audio protocol change; covered by the current first-development version.


- Reuse graph rate and diagnostic structure keys across scalar edits; cache execution-area and mute diagnostics until relevant structure changes. Pair with GLSwing connection-gradient reuse and conservative offscreen-node traversal; preserve crossing wires, focused editors, warning updates and custom color callbacks. Covered by the current development versions.

- Reuse reduced Value Viewer snapshots across status, cursor and chart rendering; invalidate on data, size and series changes while keeping full export copies and capture intact. Skip unchanged analysis redraws, duplicate repaint requests and redundant layouts during unchanged node binding updates. Covered by the current development version.

- Size new and loaded nodes synchronously from themed editor dimensions, including groups. Honor wider editor requirements and preserve larger saved sizes; remove deferred repacking and fixed standard-width overrides. Device names remain in independent popups. Covered by the existing development version.

- Derive Sine Cloud frequency editor and parameter limits from the DSP Nyquist frequency (currently 24 kHz), allowing tones above 20 kHz. Existing frequency defaults and saved settings remain unchanged.

- Expose the active Sine Cloud table period in seconds, initially zero and updated when a new table becomes active. Existing Audio/Actual count output positions are preserved.

- Add an optional Randomize phases toggle to Sine Cloud (on by default). Off starts all sine components at phase zero and ignores Seed without rebuilding playback; existing documents and native payloads retain random phases.

- Extend Sine Cloud to ten million tones and optional Gaussian frequency density (linear Hz, +/-3 sigma), retaining equal component amplitudes and exact periodic playback. Larger tables are allocated only when needed; existing documents default to Uniform and old native payloads remain readable. Covered by the current first-development version.

- Add Sine Cloud, a research generator for up to one million equal-amplitude, linearly spaced tones with seeded random phases. Background IFFT tables crossfade on control changes; Actual count exposes grid limits. Existing FFT limits and document/audio protocols are unchanged. Covered by the first development version.

- Treat the first themed viewport as startup initialization, avoiding a false
  unsaved-document prompt. Actual graph edits and recovered documents remain dirty.
- Speed up packaging tree copies and checksum inventories while preserving every
  file, empty directory and checksum; refuse linked or existing copy targets.

- Organize compiler, document, language and measurement code by responsibility.
  Existing public application aliases, saved preferences, node IDs and document
  formats remain compatible. GLSwing remains an external Maven dependency.
- Use shared native build policy in developer, installer and plugin builds;
  retain existing optimization choices. SDK forwarding headers preserve include
  paths while implementations move into runtime/resource/DSP folders.
- Reuse Core Markdown presentation for offline help. Move technical documentation,
  historical benchmarks and deferred driver work to explicit locations.
- Store local developer tools in `.local/tools` and new installer output in `dist`.
  Existing installer and research material is retained. The current first
  development version covers this cleanup; no host protocol or plugin ABI change.

- Keep long arranged node chains approximately horizontal rather than accumulating
  port offsets. Theme clearances, feedback regions and Undo are preserved.
- Add connectable Seed inputs to Bin Random and Partial Noise, including group and
  generated execution. Rebuild Java and the audio host together; regenerate old
  exported plugins that embed these changed processor parameter contracts.
- Document example-folder ownership and provide report-first cleanup of old Windows
  packaging staging. Successful package builds retain logs and remove their own
  staging; finished packages, compiler tools and runtime Bake caches are preserved.

- Use automatic node sizes in the Stochastic Bin Width example instead of fixed oversized rectangles.

- Stochastic Bin Width now uses an octave-based kernel (default +/-0.25 octaves) and installs under Examples > Spectral. Existing user example files are preserved. The previous repository preset used a linear bin radius.

- Improve Arrange all nodes: align connected ports, keep feedback regions together,
  compact narrow branches and pack independent components into free space. Group
  Input/Output remain at the outer edges, including when unconnected. Preserve node
  sizes, graph data, camera/zoom and a single Undo operation. Covered by the current
  first-development version; no document-format, plugin ABI or host protocol change.

- Add an importable Stochastic Bin Width example using existing spectral nodes:
  integer Bin Reader sampling with Box/approximate Gaussian kernels, Seed,
  per-frame variation and static-mapping phase compensation. No existing node
  behavior or document/host contract changes.

- Preserve A/B wire relocation from composite boundary inputs during projection.
  File-browser tree expansion and main-pane navigation are independent.
- Import groups with Select all, a separate whole-graph option and retained menu
  categories. Index digits is now inspector-only. Preserve recorder status across
  unused prepared sessions. Add completed-take Save as and
  optional Windows MP3 recording without bundling an encoder. MP3 has codec delay
  and padding; WAV remains the sample-accurate recording format. No document, host
  transport or plugin ABI version change; current development versions cover this.

- Show all public group input editors in the inspector, including inherited defaults
  and copied Slider, Vector, choice, checkbox and trigger controls. Persist a camera
  center per graph while keeping zoom shared. Improve automatic graph arrangement
  and preserve an occupied A/B input wire on a free compatible alternate input.
- Add Audio Recorder under Outputs: sample-aligned Start/Stop, mono/stereo float32
  WAV, optional overwrite and exclusive next-free numbered filenames. Disk writing
  runs outside the audio callback; bounded-queue/write errors are reported.
- Newly saved documents use Core schema 10 for camera centers. Existing schemas
  remain readable; earlier application versions cannot read new files. Rebuild the
  Java application and host together for the new recorder. The first development
  version, plugin ABI and host transport version remain unchanged.

- Keep independent Undo/Redo stacks for the root graph and each group definition,
  retaining group history across reopening and one document-wide save checkpoint.
  Preserve group-boundary insertion through projection and navigation.
- Restrict Copy Editor sources to standalone controls or existing input wires;
  never add implicit connections. Existing document bindings remain readable.
- Provide compact Step Switch inspector rows with names, numeric values and
  geometric reorder icons, without duplicate per-step controls.
- Import multiple group definitions in one operation, sharing copied dependencies.
  Fix Add-menu icon spacing/weight, stale tooltip ownership and inspector N/Esc
  handling. No document, plugin ABI or audio-host protocol change; the existing
  first-development version remains in use.

- Replace the experimental Partial Analysis classifier and harmonic heuristic with original Loris reassigned analysis, bandwidth estimation and tracking. Add Resolution, Frequency drift and Bandwidth model controls; Context Tonality now means the tonal energy fraction. Retired controls/bindings are removed on load, so existing separation results can change. Rebuild Java, the host and plugin/baked caches together. Loris source and notices ship in the SDK; no Python or FFTW runtime is needed.

- Replace Partial Analysis heuristic gates with adaptive bandwidth/duration/coherence classification and a 5-60 dB Minimum peak SNR control (default 30 dB). Add an inspector classification switch; Context Tonality becomes Classification margin. Old heuristic settings use the new default and copied bindings to retired controls detach with a log warning; signal connections and raw analysis are retained. Rebuild Java and the host together. The parameter fingerprint changes; document schema and transport versions do not.

- Improve Partial Analysis for neighbouring resolved tones and changing frequency by fitting strong components first and comparing track phases at the window centre. Existing settings and document/host contracts are unchanged. Add an optional offline SMS reference comparison; tonality remains a model score, not a calibrated noise probability.

- Expose the primary Partial Analysis selection controls directly in the node; keep detailed tracking and harmonic settings in the inspector. DSP behavior and defaults are unchanged.

- Improve Partial Analysis with sinusoidal-lobe fit, local prominence, time-based confidence, predictive tracking, gap tolerance and optional harmonic selection with automatic or supplied fundamental. Add Context Tonality and a separate stochastic Residual Model. Prepared partial capacities now follow configured graph budgets up to 32768 rather than a fixed 256. Existing documents receive the improved classification defaults; embedded legacy parameter commands remain supported. Rebuild Java and the host together.

- Name the creation action New Partial Group. Add Partial Noise with independent per-track/frame variation and a separate Context Seed output; existing ID outputs and connections retain their meaning. Rebuild Java and the host together.

- Draw Partials resource references dashed in red. Add complex Spectrum Math and turn Spectrum Mix into a Linear/Equal Power crossfade; earlier weighted sums migrate to Math, and embedded legacy plugin commands retain their meaning. Rebuild Java and the audio host together.

- Add tracked Partial Analysis, Partial Group, Partials to Spectrum and complex Spectrum Mix with paired German/English references. Partial programs execute per analysis frame; unchanged partials plus residual reconstruct the source spectrum.

- Add Buffer Context with Size and sample-accurate Write position for recording/file buffers. Remove the writer metadata output; old document connections and exposed group outputs migrate automatically with a load notice. Rebuild Java and the audio host together.

- Keep Copy Editor source selection explicit for unbound group inputs, preventing the first nested group input from hiding alternatives such as Hilbert filter length.

- Allow Hilbert filter length selection directly in the node (61-2049 taps); changing length resets its history and adjusts both output delays. Rebuild Java and the audio host together for the updated Hilbert parameter contract.

- Add a Hilbert quadrature filter with aligned outputs, sample-feedback and Instance Group support, and bilingual documentation.

- Add Manage groups to document settings: list embedded definitions, direct usage
  counts and containing instances. Block deletion of referenced or open groups,
  including references from unused parents. Support Undo/Redo and clear stale
  forward navigation when deleting a definition. No file is removed from disk.
  Compatible feature covered by the current first development version; no
  document-format, plugin ABI or host protocol change.

- Add a sample-accurate normalized Write position output to Recording Buffer.
  Retain the existing Buffer output and reader Distance/Read position semantics.
  Compatible addition covered by the current first development version; no
  document-format, plugin ABI or host protocol change.

- Keep Step Switch visible when changing mode inside groups; retain reentrant graph
  changes and prevent right-click/disabled sliders from blocking Undo.
- Refresh applied custom combobox templates immediately, including Undo/Redo.
- Show group icons in the Add menu, support undoable name reset, include control
  names in tooltips, and reorder Step Switch input rows in the inspector while
  retaining stable step indices and cables.
- Compatible UI fixes/features covered by the current first development version;
  no DSP, document-format, plugin ABI or host protocol change.

- Disable nested group controls while a parent group controls them, and restore
  editing when the coupling is removed. Covered by the current development
  version; no document or host contract change.

- Offer public editors of nested groups as Copy Editor sources. Forward numeric,
  checkbox, vector and choice values through nested group interfaces; retain
  wireable public inputs and exclude targets already controlled by other inputs.
  Covered by the current first development version; no document or host contract
  change.

- When choosing Copy Editor for a new group input, select from free targets
  instead of automatically trying an already controlled parameter. Keep the
  current input's own targets selectable. No document or host contract change.

- Revalidate group editor targets, public boundaries, property mappings and signal
  wires after edits, node removal, grouping and ungrouping. Remove stale bindings
  while preserving valid fan-out targets and public controls; deletion/grouping
  and their repairs undo together. Recheck older documents when loading. Missing
  plugin definitions remain diagnosable rather than discarding their bindings.
  Covered by the current first development version; no document or host contract
  change.

- Keep ordinary groups visible when they expose a nested Instance Group Count
  editor. Scope the private Count role to its owning Instance Group and repair
  existing wrappers on load, preserving their ports, wiring and editor settings.
  No document format or audio-host protocol change.

- Label the existing HISTORY group-port type as Buffer resource (German:
  Buffer-Ressource), including Formula Group inputs and live language refresh.
  Existing Buffer connections and saved type IDs are unchanged.

- Keep Custom Groups executable after replacing a choice editor: ignore retired
  generated selection fields during compilation while preserving document/Undo
  values and declared fields. Recover explicit selections when a stable custom
  entry UUID identifies one current editor; retain current explicit choices and
  avoid ambiguous recovery. Covered by the current first development version;
  no document-format or host contract change.

- Add 64- and 128-sample FFT windows, including legacy FFT Stream, analysis,
  spectral processing and IFFT. Use 16/32-sample minimum synchronous hops for
  these windows while preserving the existing cadence for larger windows.
  Rebuild the host to use the new sizes. Covered by the current first development
  version; no document-format or parameter-contract change.

- Reduce UI work when changing external parameters of Instance Groups: prepare
  only reachable group definitions and batch inferred-rate property updates.
  Preserve unused groups in the document and retain live DSP state. Covered by
  the current first development version; no document or host contract change.

- Accept external Buffer inputs in Instance Groups, including both Buffer Readers,
  shared processing after Instance Merge and Bake/export. Instances share the
  input resource and retain independent reader state. Rebuild the audio host for
  the new internal boundary processors. Covered by the current first development
  version; no document-format or protocol version change.

- Show brief, themed desktop feedback when linked output volume keys are used,
  also while EasyKnack is minimized. The overlay does not take keyboard focus,
  dismisses automatically, and reports the highest linked output level without
  changing proportional device-volume behavior. No document or protocol change.

- Honor MP3 gapless metadata during import to remove encoder delay/end padding
  from File Player and File Buffer loops. WAV import is unchanged; untagged MP3s
  retain their samples without heuristic silence trimming. Older MP3 caches are
  refreshed asynchronously when their file editors are restored; open nested
  groups or reselect the file if needed. Corrected duration/sample counts can
  differ from old imports. Use pinned MIT-0 dr_mp3 0.7.4 for import only and retain
  its upstream notices in Windows packages.

- Keep linked output volume keys active when groups contain Hidden or Mute state.
  Discover targets through the same special-group preparation as audio execution,
  independently of muting, and retain link changes made during host communication.

- Bin Width now has one Width input with Octaves, Cent, Semitones and Bins in its
  Unit context menu. Connected pitch values use the shared unit conversion path.
  Bins is an alternative linear radius, not a conversion from octaves. Remove the
  Distribution selector, obsolete Factor mode and legacy host readers. Earlier
  octave/bin document wiring is migrated; Factor documents need manual rebuilding.
  Rebuild the host and source plugins for the changed parameter contract. This is
  a pre-release breaking change covered by the current first development version.

- Add sinh, cosh, tanh, asinh, acosh, atanh, expm1 and log1p to Formula Groups,
  including preview/native execution, generated kernels and editor help. Existing
  formulas retain their behavior; new functions require an updated audio host.
  Covered by the current first development version; no document or ABI change.

- Prepare documentation and its font atlases during visible startup steps to reduce
  the delay when opening help for the first time. Parsed pages are reused while
  changes to documentation source files remain visible.

- Show the Core hand cursor over documentation links and release it when leaving
  a link, changing pages or closing the view. Avoid re-recording the entire page
  for hover changes that do not affect its appearance. Covered by the current
  development version; document and host contracts are unchanged.

- Resolve help from the verified source checkout in incremental development runs
  when copied resources are missing or stale. Keep installed help bundle-only.
  Node reference now opens the active node; a separate Node index entry always
  opens the directory. Document and host contracts are unchanged; covered by the
  current development version.

- Open node references and the handbook inside a themed EasyKnack dialog, with
  native tables, local links, Back/Forward history, restored scroll positions,
  text copying and Escape to close. Read only the selected page in the background;
  help no longer launches a browser. Existing HTML export remains available to
  tooling. Covered by the current development version; document and host contracts
  are unchanged.

- Bundle bilingual node/group references and shared fundamentals. Open localized
  offline HTML from Help or a node's Documentation action, with English fallback
  and portable links. Prepare pages in the background and invalidate by content.
  Archive six superseded references while preserving forwarding paths and workflow
  guides. Covered by the current development version before the first release;
  document and host contracts are unchanged.

- Add Vector, Wavetable, Buffer, Spectrum and Analysis modes to Step Switch, with typed optional resource inputs and block-aligned, zero-copy selection. Keep per-step buttons, names, numeric Index and Instance/Bake/export support. MIDI remains excluded. The Step Switch parameter fingerprint changes; previous numeric source exports remain readable. Rebuild Java, host and plugin SDK together; document schema and host protocol are unchanged. Covered by the current development version before its first release.

- Support Step Switch inside Instance Groups, nested instances and Bake/export. Keep selection independent per instance; show and save the first instance's selection, with manual selection shared across replicas. Rebuild the host and plugin SDK together. Parameter fingerprints, document schema and host protocol are unchanged; covered by the current development version before its first release.

- Remove fixed 8-input Instance and 16-frame-input Spectral Group limits. Counted Java/C++ source lists drive actual port layouts, rate plans, frame history, native execution and Bake/export. Keep one source Spectrum per Spectral Group and the existing supported boundary types. Previous fixed group contracts remain readable. Rebuild Java, host and plugin SDK together; parameter fingerprints change for compiled groups, while document schema and host protocol remain unchanged. Covered by the current development version before its first release.

- Remove the fixed eight-output host/group/export limit. Size output, event-adapter and spectral/instance work buffers during preparation, with packed per-bin voice storage. Keep the existing source-reference encoding and validate actual source-port counts. Cover 24-port audio/control/event/resource routing without callback allocations, and 12-port Spectrum/numeric groups including persistence and Bake/export. Existing graph/voice budgets and integer addressability still apply. Rebuild the host and plugin SDK together; document schema, parameter fingerprints and protocol remain unchanged. Covered by the current development version before its first release.

- Allow multiple Bin Outputs in one Spectral Group with a shared bin schedule, independent complex streams and matching worker/Bake/export kernels. Instance Groups can expose Spectrum outputs with aligned complex Sum/Mean reduction per channel. Output storage follows the group's declared interface; Bin Discard still affects all outputs. Rebuild the host and plugin SDK together; rebake/re-export to use multiple outputs. Existing single-output programs, document schema and host protocol are unchanged. Covered by the current development version before the first release.

- Validate bin-only processors against their required Spectral Group context before sending graphs to the host. Keep ordinary temporal contracts and FFT-frame trigger execution separate in regression tests. Existing DSP behavior, native contracts and protocol versions are unchanged; covered by the current development version.

- Add an editable Test column after the Measurement Recorder index. Store labels per row in the document, preserve them during capture and column reordering, and include a `test` column after `measurement` in CSV exports. Label Undo retains later automatic captures. Existing tables start with blank labels; host protocol and document schema are unchanged. Covered by the current development version.

- Add independently switchable buffered control interpolation (50 ms default) and finite movement-edge rounding (5 ms default), saved per document with driver overrides. Buffering replaces adaptive ramp latency; rounding preserves endpoints and linear interiors. Existing documents adopt these defaults when the new fields are absent. Host protocol advances to 13: rebuild Java, host and plugin SDK together. Product version remains covered by the current development baseline.

- Correct adaptive control timing: first changes and gaps above Max use Min; all other intervals are used exactly, including intervals below Min. Interrupted ramps continue from the current value. Clarify the timing labels; host protocol and document schema remain unchanged. Covered by the current development version.

- Add document-scoped adaptive linear interpolation for continuous mouse and MIDI controls, with per-driver overrides, bounded timing and explicit discrete-slider policies. Connected signals bypass interpolation; copied group editors and baked/Instance boundaries retain the policy. Audio rate renders per-sample transitions; Control remains once per block. Host protocol advances to 12: rebuild Java, host and plugin SDK together. Existing documents use 5-50 ms defaults; the current development version covers this feature.

- Expand Instance Groups with Spectrum Blur/Shift/Prune, Formula Groups, numeric spectral/correlation analysis and averaging, envelopes, trigger processors, live Buffers and Buffer Readers. Include resource relays for nested execution. Public boundary formats remain unchanged. Rebuild the host and plugin SDK together; existing document and protocol versions are unchanged. Covered by the current development version.

- Apply live parameter changes recursively inside Instance Groups and exported/baked graphs without rebuilding their runtime, resetting oscillator phases or regenerating unchanged Wavetables. Parse one instance template per update, share immutable Wavetables and reuse Java execution contracts. Structural edits still require preparation. Rebuild the host, SDK and source plugins together; document and wire-protocol versions are unchanged. Covered by the current development version.

- Accept large graphs with embedded Wavetables through buffered host command reads (up to 64 MiB). Oversized commands are rejected without terminating the host or losing command framing. The current development version covers this fix; the protocol version is unchanged.

- Support Wavetable nodes and typed external Wavetable inputs in Instance Groups, including shared processing, Bake and source export. Rebuild the audio host and plugin SDK together; existing document and protocol versions are unchanged. Covered by the current development version.

- Unify Value Viewer/Recorder port and display order. Reordering moves ports with their connections while retaining recorded values and frozen Viewer state. Use Core-framed inspector rows, drawn arrow icons and categorized unit submenus; remove Recorder fallback sliders to match Viewer port spacing and simplify the inspector. Requires the updated external Core view-state-transfer callback. Native contracts and document schema are unchanged; covered by the current development version.

- Add shared, compact name/unit inspector rows with up/down ordering to Value Viewer and Measurement Recorder. Display order applies to frozen histories, table columns and exports while input sockets, trace colours and recorded slot associations remain stable. Recorder units are display labels only. Settings persist with the document and support Undo; legacy names/units are retained. Native contracts and document schema are unchanged. Covered by the current development version.

- Correct Digital Noise cycle-duration bias and fractional cycle timing so stationary octave deviations are symmetric over elapsed time. Add a bounded Gaussian distribution alongside Uniform; Strength remains the maximum octave deviation, with Gaussian sigma = Strength / 3. Strength zero immediately passes the base frequency. This intentionally changes existing sounds and seeded sequences. Rebuild Java, host, plugin SDK and affected baked/exported nodes together. Port IDs and document/protocol versions are unchanged. With no release baseline, retain the current development version and include this behaviour change in the first release notes.

- Add an inspector frequency-scale choice to Spectral Features. Logarithmic moments exclude DC, return a geometric Centroid in Hz and Spread in octaves (with semitone/cent choices), and preserve the definitions of Flux, Roll-off, Slope and Flatness. Linear remains the default; changing scale changes Spread connection semantics. Rebuild Java, host and plugin SDK together; document and protocol versions are unchanged. Covered by the current development version.

- Fix right-channel autocorrelation processing when FFT preserves stereo. Correlation descriptors now remain valid on both channels instead of being halved by a downstream channel mix. Rebuild the audio host and plugin SDK; document and protocol versions are unchanged. Covered by the current development version.

- Add Correlation Features with held control outputs for periodicity, period, repetition frequency, estimated HNR and validity; no Updated trigger. Inspector search/threshold settings and source-level metadata support silence gating, including averaged correlations. Rebuild Java, host and plugin SDK together. Document and host protocol versions are unchanged; the current development version covers this addition.

- Fix Measurement Recorder capture and Undo retention with multiple inputs by recognizing dynamically registered node variants. The current development version covers the fix; document and host contracts are unchanged.

- Add Measurement Recorder: trigger-aligned numeric rows, named inputs, persistent document tables, enlarged inspection and CSV/Excel export. Automatic capture marks the document modified without adding Undo entries. Tables retain up to 4096 rows without overwriting; Clear starts a new acquisition. Rebuild Java, host and plugin SDK together. No existing protocol or document-schema changes; the current development version covers this addition.

- Add population standard deviation, minimum and maximum to Value Average, using the same finite acquisition and held results as Mean. Rebuild Java, audio host and plugin SDK together. Existing port IDs and output indices are retained; document and protocol versions are unchanged. Covered by the current development version.

- Preserve direct group input-to-output routes when outer parameters change. Keep custom Value Viewer dimensions when returning from a group, including hidden nodes. Requires the updated external Core artifact; document and audio contracts are unchanged. Covered by the current development version.

- Fix group connection Undo after view replacement and preserve boundary wiring through presentation/property changes. Copied choice editors now allocate independent persistent parameter IDs; duplicate legacy IDs are repaired deterministically, retaining the first parameter value and using each additional editor's own default. Requires the updated external Core artifact; existing valid IDs, document format and audio protocol are unchanged. The current development version covers these fixes.

- Fix Analysis Average dropping delayed frames from asynchronous FFT spectrum streams; each new delivered frame now counts once. Both averaging nodes support insertion into compatible connections and mute bypass; auxiliary Count/Finished outputs are silenced while muted. Existing contracts and versions are unchanged; the current development version covers these fixes.

- Add Value Average and Analysis Average for trigger-started finite arithmetic measurements, count/time limits, held results and a Finished trigger. Spectra average power per bin; correlations average per lag. Spectral Features accepts statistical analysis resources; IFFT still requires a phase-bearing spectrum. Rebuild Java, host and plugin SDK together. The current development version covers these compatible additions; existing protocol and document versions are unchanged.

- Preserve renamed nodes through diagnostic refresh, language changes, copying and group navigation. Custom names are saved in Core graph schema 9; older documents still load, while new saves require the updated Core. The current development version covers this fix.

- Buffer spectral render bursts on a dedicated WASAPI producer, with reserve based on FFT hop cadence. Ordinary audio graphs keep direct rendering; ASIO stays synchronous. This trades extra spectral-path latency for callback deadline isolation. Playback queue underruns are reported independently of cycle-averaged DSP load. The current development version covers the fix.

- Add MIDI editor drivers with learning, per-slider mapping/pickup, note/CC control of choices, toggles and trigger buttons, and MIDI-colored frames. Assignments persist in documents; graph connections take precedence without removing them. MIDI changes do not add Undo entries and retain current values through graph Undo. Requires the updated external GLSwing artifact and rebuilt audio/plugin host; audio protocol remains 11 with additive driver commands. The current development version covers these compatible additions.

- Add Spectral Features (eight descriptors) and a multi-input Value Viewer with bounded history, peak-preserving plots, Freeze, numeric display, zoom/measurement cursors and PNG/JPG/CSV snapshot export. Rebuild Java, host and plugin SDK together for the new node types; existing contracts and documents are unchanged. The current development version covers these compatible additions.

- Read the original project license/notices in verified source checkouts when an incremental IDE build removes generated legal resources. Packaged applications continue to require their bundled documents; plugin SDK licensing uses the same lookup. The current development version covers this fix.

- Keep Help > Licenses responsive by browsing complete license documents individually, starting with an overview. Requires the accompanying GLSwing multiline-layout fix for large texts. The current development version covers this compatible fix.

- Offer EasyKnack, Blue, Dark Blue and Light themes using external GLSwing presets with shared application color/layout fragments. Theme selection previews live; Light adapts workspace overlays for contrast. Existing theme selection IDs remain valid. The current development version covers this compatible feature.

- Add 16k, 32k and 64k FFT windows for synchronous and background processing, including 2x padding, IFFT and analysis readouts. Rebuild the audio host and source plugins with the updated SDK; new window sizes require this version. The current development version covers this compatible feature.

- Keep direct group input-to-output connections when leaving a group and reopening documents. Preserve numeric defaults and units during execution. Groups using direct forwarding require the updated GLSwing dependency; no audio-host protocol change. The current development version covers this fix.

- Add a sample-based Wavetable Viewer, lossless resource copy/paste and freehand sample editing alongside curves. Add an inspector-only minimum interval for asynchronous FFT requests. Existing curves remain readable; sample-mode documents require this version. Rebuild host and plugins together for the updated waveform and FFT parameter contracts. The current development version covers these additions.


- Add dark-blue Waveform resources with a periodic curve editor and band-limited oscillator playback. Add background Spectrum-to-Waveform capture, document snapshots and an explicit 64-point editable-curve copy. Waveform ports now require resources rather than numeric blocks; rebuild custom waveform plugins. The current unreleased development version covers this addition.

- Center unpacked groups on their combined node bounds. Fix live curvature preview with the event-driven application Canvas while Preferences remains open; verified using the actual application host. No document or host-protocol changes.

- Preview theme, language and connection curvature while Preferences is open; cancel restores the previous appearance. Add one-level Ungroup (Alt+G) with undo, preserved boundary values and connections, and retained group definitions. Special execution groups remain intact. No document or host-protocol migration required.

- Make autosave interval and recovery retention configurable in Preferences. Prepare and compare detached document snapshots on a low-priority worker. Add a live connection-curvature slider from straight lines to the current theme geometry; cancelling restores the previous setting.


- Add atomic crash-recovery snapshots every 30 seconds with a startup recovery dialog, original save targets, and seven-day retention. Ignored connections use muted opacity; unused muted-node inputs are included. Group titles show a geometric group marker.


- Add document-local unused-group retention (off by default when saving), fit-all view (Ctrl+Shift+C), and audio start/stop shortcuts (Ctrl+Enter / Ctrl+Shift+Enter). Missing mute bypass sources now mark connections as ignored and expose destination editors. Existing document metadata remains compatible.


- Groups can copy Vector editors with XYZ defaults and unit choices, including
  rotation and scale editors. Standalone vector editors support shared coupling.
  Updated Core capabilities allow explicitly copyable custom editors to be linked.

- Replaced Vector Mapping with Vector Scale and Vector Rotate, each with an
  Audio/Control Origin input. Existing documents/plugins using Vector Mapping
  must replace it with Scale followed by Rotate (both Origins zero) and rebuild.
  Java, the audio host and plugin SDK must be updated together.

- Removed group inputs no longer block execution through stale instance defaults.
  Formula kernels consistently use their declared input units in spectral graphs.
- Unconnected numeric group inputs retain editable defaults without blocking audio,
  including newly added Formula inputs awaiting a valid code update.


- Numeric ports with unit formats now expose a Unit context menu, including connected
  inputs and group outputs. Labels show the selected unit. Output units change the
  emitted numbers; input units interpret incoming numbers. Connecting ports never
  changes units or converts a connection automatically. Settings persist through
  save/load, undo, group baking and source export.


- Vector ports now align with the first XYZ slider and collapse while connected.
  Vector Math and Vector Mapping can be inserted directly into vector connections.
  Vector Input is listed under Inputs; Spatial Receiver is under Routing.
  Receiver yaw is hidden and disconnected from processing in Mono mode.
  Added Vector Mapping (vector scale and Euler rotation) and Context Time output
  in seconds at audio rate. Existing port indices are retained. New nodes and the
  Time output require the updated host and SDK.


- Added Audio/Control vector signals, grouped XYZ editors with length units,
  vector operations and scalar measurements, and a mono/stereo Spatial Receiver
  with channel-specific distances, yaw, spacing and optional propagation.
  Vector group boundaries support bake/export. Existing documents retain their
  behavior; documents using these nodes require this build or newer.

- Release packaging now includes verified GLSwing build inputs and WiX sources/
  notices. SDK and source-plugin exports carry license notices; existing plugins
  may rebuild once because the SDK archive changed. Source review and installation
  validation are reported separately.

- Added product splash artwork and application icons for windows, launcher,
  installer, shortcuts and .knack documents. The splash retains its standard
  text layout directly over the artwork, with narrower text margins and a
  full-width progress bar at the bottom edge.

- Windows packages now require a separately installed x64 JDK 25. The launcher
  detects Java and offers interactive Microsoft OpenJDK installation or folder
  selection. LLVM-MinGW remains included for Bake/plugins; Java is not removed
  when EasyKnack is uninstalled. Bundled Temurin is deferred.


- Replaced bundled FFmpeg/JavaCPP with the existing miniaudio decoder for WAV and
  MP3 import. Other formats must be converted before import; existing PCM caches
  remain readable. Player/Buffer behavior and the audio-host protocol are unchanged.

- Expanded Windows native-source and license coverage, including GNU Make,
  MinGW gendef and FFmpeg dependencies. Recorded official runtime/toolchain
  comparisons and remaining provenance limits; exclude generated compiler Python
  caches from future installers. No DSP, document or plugin-contract change.

- Added Help with an offline English handbook, selectable application/Core build
  details and license notices. Installer builds capture local sources and collect
  checksum-locked upstream materials beside the installer; incomplete provenance
  review is explicitly reported. No document, plugin ABI or audio protocol change.

- Declared EasyKnack GPL-3.0-only; GLSwing and third-party licenses remain unchanged.
  Packages include project and native-component notices. Corresponding-source
  preparation remains a release requirement. No runtime or document-format change.

- Replaced the mixed-language README with an English quick start and linked user
  guide, node reference and known limitations. Windows packages include the guide.
  Developer and historical notes are separate; no runtime or document-format change.

- Windows installers register .knack documents. File launches reuse the running
  application, queue during startup and preserve the unsaved-changes prompt.
  No document, plugin ABI or audio protocol change.

- Windows EXE/MSI installers let users choose desktop and Start Menu shortcuts independently.

- Added a self-contained Windows app-image build with bundled Java/C++ tools,
  native audio host and media support. Installed host/tool discovery no longer
  depends on the working directory. No document, plugin ABI or protocol change.

- Shared UI/export and group-authoring infrastructure now comes from GLSwing.
  Dialogs remember folders by stable purpose across language changes; directory
  selections remember the selected folder. Existing group/template documents remain compatible.

- Image-export geometry now uses GLSwing's shared dimension model; sizing and
  padding behavior are unchanged. No document or audio-protocol migration.

- Unified live node/inspector tooltips, displayed-unit bounds, port terminology
  and English/German rate explanations. Reference ports now describe their
  actual data type instead of numeric control-rate behavior. Inspector refreshes
  no longer emit parameter edits; hidden slider labels follow current units.
  Added builtin inspector coverage and grouped Value Input document checks.

- Value Input supports inspector-configured button/bar sliders and optional bounds;
  narrowing bounds clamps the stored value. Fixed Step Switch input labels during
  live language changes and repaired mixed encoding in the German language file.

- Node-menu categories follow a fixed workflow order in both languages; nodes
  inside each category remain alphabetically sorted.

- Reorganized the node catalog by task, with separate Inputs, Outputs and Trigger
  categories. Envelope and Clock are generators; Pitch Hold remains in Analysis.
  Clarified node names in English and German, including Additive Oscillator,
  Audio File Player, Recording Buffer, Threshold Trigger and Instance Merge.
  Node IDs, DSP behavior and signal colors are unchanged; old plugin category
  constants remain accepted as aliases.

- Node port labels update when their parameters change, including the Unit Converter's output unit.

- Breaking: the five unit-converter variants are replaced by one Unit Converter.
  Old variant node IDs are removed without document migration. Choose the source
  unit first, then a compatible target. Frequency and period convert reciprocally,
  including BPM and samples at the host sample rate. Zero maps to zero.

- Rewiring group boundary connections keeps the active drag alive; interface
  reconciliation waits until release or cancellation. Copied sliders list their
  allowed default units directly, without a redundant family submenu.

- Copy Editor prioritizes the connected parameter and its units. Disconnecting
  its source clears the inherited editor. Linked controls display group defaults
  without overwriting their original values; hidden sockets lock numeric rates
  to Control. Boundary changes now apply the application's presentation policy.

- Copied editors retain a read-only rate and allow validated defaults (slider
  value/unit, combobox entry or checkbox state). Node definitions can opt editor
  rows out of copying; Step Switch custom controls are excluded. Group slider
  units use one grouped default-unit menu; the default unit cannot be disabled.

- Copied group editors inherit their style, bounds and allowed units, while
  exposing their source, coupling targets and permitted defaults. “No editor” removes an editor and releases
  its coupled internal controls without deleting the group port.

- Quick-created group ports snap to the boundary preview and inherit semantic
  type and declared rate, including Auto. Standalone editor selection is grouped
  by node type and parameter, excludes input-backed and generated unit selectors,
  and then exposes individual coupling targets. Coupled sliders also lock their
  unit and automatic-value actions.

- Group interfaces retain exposed ports when an internal node or dynamic port is
  removed. New boundary nodes start beside the inner graph; inserting into boundary
  wires and creating ports by dropping connections onto boundary nodes support undo.
- New group ports default to Value and accept Unicode names. Group sliders offer
  bar/button presentation, unit families, allowed units and a default display unit.
- Group editors can control multiple matching standalone parameters. Original
  controls are read-only in the node and inspector. Couplings are saved with the
  document; older versions do not interpret these new editor options. Bake/export
  supports coupled comboboxes; other standalone coupled editors currently require
  live groups and are explicitly rejected by bake/export instead of silently frozen.
- Requires the external GLSwing 0.2.0-SNAPSHOT library. No native host protocol change.

- Analysis hover readouts update outside rendering, preventing repeated layout
  invalidation that could stall picking and leave tooltips visible after exit.

- Spectral Groups support Formula nodes, with per-bin/channel persistent state,
  frame triggers and portable runtime/bake/export execution. File/delay Buffer
  inputs and dependencies on generated per-sample audio are rejected in this domain.

- Spectral Groups add Bin Memory with a 0–1 per-frame Update input, independent
  state per bin/channel, and runtime/export support.
- Filter adds orders 2/4/6/8 and optional bounded RMS level compensation. Existing
  documents and old native parameter payloads retain order 2 with compensation off.
- Spectral Groups add Bin Sample & Hold with a Capture button and Trigger input,
  separate state per bin/channel, and timestamped external frame triggers.
  Compatible rebuilds retain captures; changed FFT identity/size resets them.
  Stateful groups retain empty held bins and use serial bin evaluation, also in exports.
- Spectrum Blur Noise now limits phase deviation, phase movement and magnitude
  fluctuation below one physical frequency cell. Tiny nonzero widths approach the
  original signal continuously instead of switching immediately to full noise.
  No parameter/document format changes; existing narrow-width Noise settings sound gentler.
- IFFT adds optional Stability (0–1), a bounded continuous RMS correction after
  overlap-add for narrow-band noise. Default zero preserves existing sound.
  Correction adds no latency but can reduce intentional dynamics and broaden spectra.
  Previous compiled IFFT contracts remain accepted with stability disabled.
- Spectrum Blur offers Noise, Continuous and Preserve synthesis. New nodes use
  noise with energy between bin centres; existing phase settings retain their mode.
  Logarithmic noise shaping targets equal power per octave for flat envelopes.
  Zero width remains an exact bypass. Java and host must be rebuilt together;
  previous compiled parameter contracts remain supported.
- Spectrum Blur now gathers neighbouring magnitudes using Gaussian, Box or Maximum
  kernels. New nodes default to octave spacing; frame power normalization is optional.
  Existing documents retain their selected width mode, but use the new Gaussian
  behaviour unless another kernel is chosen. Previously compiled native payloads
  retain their legacy blur behaviour until regenerated.
- Audio diagnostic exports identify the packaged EasyKnack and GLSwing builds,
  including the source revision and whether uncommitted changes were present.
- Local development launches are explicitly distinguished from packaged builds.

The first published release version has not been designated yet.

- Generic vector nodes now offer length, angle, scale and unitless quantities in the inspector, including compatible scalar output units. Existing vectors keep length as their default.
- Instance Groups now support internal FFT / Spectral Group / IFFT chains, with independent frame state per instance, including source export. Public Instance Group boundaries remain numeric.

- Instance Groups accept shared Spectrum inputs, including source export and Bake; per-instance processing retains independent state. Existing numeric instance-input contracts remain unchanged; the host and plugin SDK include the new typed resource boundary.

- Instance Groups support all Vector nodes and Spatial Receiver, including shared Vector inputs, sample-rate/channel preservation and Bake/source export. Group outputs still require numeric Instance Merge results.

- Instance Groups can contain other Instance Groups, including in Bake and source
  export. Counts and context indices are scoped to each level; capacity limits
  account for the entire nested graph. Update the audio host and plugin SDK together.

- Copied integer group sliders retain usable increments and drag sensitivity, including older editor snapshots. Group-authored sliders can select integer values in Group-IO.
- Newly extracted groups inherit their first output signal kind once. All signal categories have matching node kinds and shared highlight colors; trigger-producing, buffer and spectrum nodes use their corresponding palettes. Existing manual group kinds remain unchanged.

- Group extraction preserves the semantic category of connected outputs instead of their shared numeric transport type. Editing group slider limits or defaults adjusts dependent values automatically.

## Backlog stabilization (29 October 2026)

- Reuse local group Bakes across equivalent instances, with independent DSP state,
  stable compilation identity and shared invalidation after internal edits.
- Establish MIDI driver indicators as inspector controls are attached.
- Add Step Switch index mode and named inputs; existing documents default to
  trigger mode. The Step Switch native parameter contract changes; rebuild Java,
  host and source plugins together. No document schema or protocol change.
- Verify identical cent detuning for all Complex Oscillator overtones while
  preserving the fundamental. Archive obsolete backlog and implementation notes.
- The existing 0.0.1-SNAPSHOT development version covers this work; no first
  release baseline has been declared.

- Consolidate and preserve the previously uncommitted group-editor, unit/vector,
  node-catalog and spectral/filter improvements in focused local commits. Machine-
  specific Eclipse configuration is intentionally excluded.

- Rename Step Switch inputs in place from the slider context menu or by clicking
  a connected input label. Persist the selected step and allow manual selection
  while audio is stopped. Revision-checked runtime feedback updates saved state
  without adding Undo entries or replaying stale selections into the DSP.
  Rebuild Java, host and source plugins together (Step Switch parameter contract);
  existing documents start on step zero. Document schema and protocol are unchanged.
  Covered by the existing development version; oscillator spreading is unchanged.

- Replace Complex Oscillator cent spreading with dimensionless power stretching:
  partial n uses f0 * n^p (0-4, neutral 1). Keep the fundamental fixed and cache
  ratios/logarithms. Old scalar cent settings map to clamp(1 + cents/1200, 0, 4);
  this preserves the second partial where possible, not the previous full spectrum.
  Existing modulation, inherited editor bounds/units and generated groups/plugins
  require review and rebaking/re-export. Java and host must be rebuilt together.
  Stable port IDs and document schema are unchanged. This intentional 0.x semantic
  change is covered by the existing development version before its first release.
