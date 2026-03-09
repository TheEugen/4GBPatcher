# 🔧 4GBPatcher

A lightweight Windows GUI utility written in C++ that patches 32-bit executables to use up to 4 GB of RAM by setting the Large Address Aware (LAA) flag in the PE header. Supports status detection, patching, unpatching, and automatic backup creation.

---

## Background

By default, 32-bit Windows applications can only address 2 GB of virtual memory, even on 64-bit systems with far more RAM available. The PE (Portable Executable) format includes a flag in the header — the **Large Address Aware (LAA) flag** (bit 5 of the characteristics byte) — which, when set, signals to Windows that the process can handle addresses above 2 GB, raising the limit to 4 GB. This is useful for memory-hungry applications such as older games or large-scale data processing tools that were never recompiled with the flag enabled.

---

## How It Works

The patch is a precise single-bit operation on the executable's PE header:

1. The tool reads the PE offset from the file at `OFFSET_POSITION`
2. It seeks to `LAA_BYTE_POSITION` and reads the characteristics byte into a `std::bitset<8>`
3. It flips bit 5 (the LAA flag)
4. It writes the modified byte back to the same position in the file

Unpatching performs the exact same operation in reverse, flipping bit 5 back off.

---

## Features

- **Win32 GUI** — Native Windows application, centred on screen at launch
- **File picker** — Opens a file dialog filtered to `.exe` files only
- **Patch status detection** — Immediately reads and displays whether the selected executable is already patched or not
- **Patch / Unpatch** — Buttons are enabled/disabled contextually based on current patch status (can't patch an already-patched file, can't unpatch an unpatched one)
- **Automatic backup** — Before modifying the file, a backup copy is created (`.exe_unpatched` or `.exe_patched`); if a backup already exists the user is asked whether to overwrite
- **Error handling** — Catches and displays specific file open errors: access denied (e.g. the `.exe` is currently running), file too large, path too long, and unknown errors

---

## Project Structure

```
4GBPatcher/
├── 4GBPatcher.cpp    # Entry point, Win32 window, message loop, button dispatch
├── 4GBPatcher.h      # Includes and forward declarations
├── Utils.h           # Core logic: patchFile, detectPatchStatus, toggleButtons, error handling
├── PathHandler.h     # Simple wrapper holding the selected file path
└── Resource.h        # String constants, control IDs, window dimensions
```

---

## Getting Started

### Prerequisites

- Windows
- Visual Studio (MSVC)

### Build

```bash
git clone https://github.com/TheEugen/4GBPatcher.git
```

Open in Visual Studio and build. No external dependencies — uses only the Windows SDK.

A prebuilt release binary is available on the [Releases page](https://github.com/TheEugen/4GBPatcher/releases/tag/v1.0.0).

---

## Usage

1. Launch `4GBPatcher.exe`
2. Click **Choose File** and select a 32-bit `.exe`
3. The status label shows whether the file is already patched or not
4. Click **Patch** to enable the LAA flag, or **Unpatch** to remove it
5. A backup of the original file is created automatically before any modification

> ⚠️ Do not select an executable that is currently running — the file will be locked and the tool will report an access denied error.

---

## License

This project is licensed under the [MIT License](LICENSE).