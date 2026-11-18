# Getting started

## Installation and removal

Use `EasyKnack-Setup-<version>-x64.exe` to install; `EasyKnack.exe` starts the application. Windows x64 and an external x64 JDK 25 including `javac` are required. To remove EasyKnack, close it and choose **Settings → Apps → Installed apps → EasyKnack → Uninstall**, or reopen the setup and choose removal. No separate uninstall EXE is needed. Your documents and separately installed JDK remain.

Setup checks Java before copying application files. If no compatible JDK is found, you can select its folder or explicitly start an external JDK installation. Cancelling stops setup; Java remains separately installed.

Linux is currently a developer-only port, not an official release. There is no
Linux installer or package; see the developer documentation for the source build.
System/application capture, ASIO, routing, device volume, hardware MIDI input,
MP3 recording and plugin builds are not available there yet.

Choose **File → New**, select your device and a low volume on **Audio Output**, then start audio. The initial graph connects an Oscillator to the output. Use Audio Input for a microphone or external signal.

Add nodes from the Add menu. Drag an output to a compatible input; warnings explain rejected connections. The Inspector shows the selection's settings. **N** toggles it; **Ctrl+Shift+L** arranges the current graph. Menus show other shortcuts. Hiding reduces clutter but does not stop processing.

Save as `.knack`. Examples open a new document; save your variations under a new name. Undo/Redo belongs to the open graph; saving covers the whole document. Camera movement alone does not mark it modified. After a crash, startup offers available recovery snapshots; they do not replace regular saving.

## Audio and files

WASAPI uses Windows devices; ASIO requires a suitable installed driver. Device volume on a WASAPI output changes the Windows endpoint; Gain changes the graph signal. Copy capture of system/application audio preserves the original playback route. For processed-only sound, route the source through a separately installed virtual cable into EasyKnack, then to the physical output.

Audio files and plugins are not embedded in `.knack`. Check them after sharing or moving a document. For glitches, inspect device status and Render diagnostics. A useful bug report includes **Help → About**, the message and a small example document.

## Settings and help

Application preferences control language and appearance. Document settings include control interpolation, unused groups and experimental features. Legacy documents using experimental nodes enable the option automatically unless explicitly saved off. Disabling it while such nodes exist requires warning confirmation; existing nodes remain intact.

The documentation window remembers **Same as application / Deutsch / English** independently of application language. Node context menus and Help open the relevant node reference; Back/Forward follows local links. **Find / Ctrl+F** searches the current page with the same options as the text/code editor; **F3 / Shift+F3** selects the next/previous highlighted match.

[Manual](index.md) · [Node index](../../nodes/en/index.md) · [Deutsch](../de/start.md)

The default project folder is `Documents/EasyKnack/Projects`; recordings use `Documents/EasyKnack/Recordings`. Built-in examples are installed with EasyKnack. Save your own variations in Projects. Previously created example copies in Documents are left untouched.

Arrange (**Ctrl+Shift+L**) uses rough placement and branch order while compacting connected nodes and resolving overlaps. Place related nodes beside or above each other first; signal flow and group boundaries take priority. Undo restores the previous positions.
