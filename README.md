# bootctlAlone

Minimal **bootctl** — A/B slot info via Android properties.  
No HIDL/Boot Control HAL dependency; uses `ro.boot.slot_suffix` and fixed A/B semantics.  
CMake-only, build with Android NDK for device.

## Features

- **get-number-slots** — prints `2` (A/B)
- **get-current-slot** — 0 or 1 from `ro.boot.slot_suffix` (`_a` → 0, `_b` → 1)
- **get-active-boot-slot** — same as get-current-slot
- **get-suffix SLOT** — prints `_a` for slot 0, `_b` for slot 1

Commands that require the Boot Control HAL (e.g. **set-active-boot-slot**, **mark-boot-successful**) are not implemented; this build is for read-only slot reporting (e.g. for [YukiSU](https://github.com/YukiSU) or scripts that only need current slot/suffix).

## Requirements

- CMake ≥ 3.20
- C++17
- **Android NDK** for device binary

## Build

### Android (NDK)

```bash
export ANDROID_NDK=/path/to/ndk
cmake -S . -B build_android \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21
cmake --build build_android
```

Binary: `build_android/bootctl`. Other ABIs: use `build_android.sh armeabi-v7a`, etc.

### Host (stub)

```bash
cmake -S . -B build && cmake --build build
```

Prints that bootctl is for Android only.

## Usage

```bash
bootctl get-number-slots      # 2
bootctl get-current-slot      # 0 or 1
bootctl get-suffix 0           # _a
bootctl get-suffix 1           # _b
```

## Project layout

```
.
├── CMakeLists.txt
├── README.md
├── LICENSE
└── src/
    └── bootctl_main.cpp   # property-based slot commands
```

## Origin and license

- CLI and command names follow **AOSP** `system/extras/bootctl`. This repo is a minimal, property-only implementation without HIDL for projects that only need slot read-out.
- **License**: Apache-2.0 (same as AOSP extras). See [LICENSE](LICENSE).
