# bootctlAlone

Standalone (or multi-call) **bootctl** compatible with Android boot HAL CLI.

- **Entry**: `bootctl_main(int argc, char **argv)` for use when linked into ksud; define `BOOTCTL_STANDALONE` for a standalone executable with `main()`.
- **Slot detection**: Reads `/proc/cmdline` for `androidboot.slot_suffix=` / `slot_suffix=` (e.g. `_a` / `_b`). No dependency on boot_control HAL or libcutils.
- **Commands**: `hal-info`, `get-number-slots`, `get-current-slot`, `get-active-boot-slot`, `get-suffix SLOT`. Write operations (`set-active-boot-slot`, `mark-boot-successful`, etc.) are not supported in fallback mode and return exit code 69 (EX_UNAVAILABLE).
