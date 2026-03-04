# WinAPI System Monitor

A lightweight Windows system monitoring application built with the Win32 API. Displays real-time CPU, RAM, disk, network, and GPU usage in a dark-themed grid UI with colored progress bars.

## Features

- **CPU** — per-core usage tracking via `NtQuerySystemInformation`
- **Memory** — system RAM load percentage via `GlobalMemoryStatusEx`
- **Disk** — both storage space used and I/O activity on C:\
- **Network** — real-time upload and download speed in KB/s
- **NVIDIA GPU** — usage, VRAM, temperature and clock speed via NVML (dynamic loading of `nvml.dll`)
- **AMD GPU** — usage percentage via PDH performance counters
- **Alerts** — pop-up warnings when CPU, RAM or disk exceed configurable thresholds (30s cooldown between alerts)

## Building

### Requirements

- Windows 10 or later
- Visual Studio 2022 with the **Desktop development with C++** workload
- Windows SDK 10.0.22621.0 or later
- NVIDIA drivers installed (for GPU monitoring — `nvml.dll` must be present in `C:\Windows\System32`)

### Steps

1. Clone the repository:
```bash
   git clone https://github.com/SoloScriptSage/winapi-system-monitor.git
```
2. Open `winapi.vcxproj` in Visual Studio
3. Set configuration to **Debug x64** or **Release x64**
4. Build with `Ctrl+Shift+B`
5. Run with `F5`

## Roadmap

- [ ] GPU temperature graph over time
- [ ] Per-core CPU usage bars
- [ ] Multiple disk support (C:, D:, etc.)
- [ ] Configurable alert thresholds via settings window
- [ ] Full AMD GPU support via ADL SDK (currently only usage % via PDH)
- [ ] System tray icon with quick stats on hover
- [ ] Export stats to CSV