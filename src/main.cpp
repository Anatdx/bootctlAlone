/*
 * bootctl - A/B slot info and compatibility with Android boot HAL CLI.
 * Uses /proc/cmdline for slot detection when HAL is not available.
 *
 * Copyright (C) 2016 The Android Open Source Project (original AOSP bootctl)
 * Copyright (C) YukiSU - property/cmdline fallback implementation
 *
 * Licensed under the Apache License, Version 2.0.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

constexpr int EX_OK = 0;
constexpr int EX_USAGE = 64;
constexpr int EX_SOFTWARE = 70;
constexpr int EX_UNAVAILABLE = 69;

constexpr std::string_view SLOT_SUFFIX_A("_a");
constexpr std::string_view SLOT_SUFFIX_B("_b");
constexpr unsigned NUM_SLOTS = 2;

// Read /proc/cmdline and find androidboot.slot_suffix= or slot_suffix=.
// Returns "_a", "_b", or nullopt on failure.
std::optional<std::string> getSlotSuffixFromCmdline() {
    std::ifstream f("/proc/cmdline");
    if (!f)
        return std::nullopt;
    std::string line;
    if (!std::getline(f, line))
        return std::nullopt;

    const char* prefix = nullptr;
    if (line.find("androidboot.slot_suffix=") != std::string::npos) {
        prefix = "androidboot.slot_suffix=";
    }
    if (!prefix && line.find("slot_suffix=") != std::string::npos) {
        prefix = "slot_suffix=";
    }
    if (!prefix)
        return std::nullopt;

    size_t pos = line.find(prefix);
    if (pos == std::string::npos)
        return std::nullopt;
    pos += std::strlen(prefix);
    size_t end = line.find_first_of(" \n\t", pos);
    if (end == std::string::npos)
        end = line.size();
    if (end <= pos)
        return std::nullopt;

    std::string suffix = line.substr(pos, end - pos);
    if (suffix.size() == 1) {
        if (suffix[0] == 'a') return std::string(SLOT_SUFFIX_A);
        if (suffix[0] == 'b') return std::string(SLOT_SUFFIX_B);
    }
    if (suffix == SLOT_SUFFIX_A || suffix == SLOT_SUFFIX_B)
        return suffix;
    return std::nullopt;
}

// Get current slot number (0 or 1). Returns nullopt on error.
std::optional<int> getCurrentSlotNumber() {
    auto suffix = getSlotSuffixFromCmdline();
    if (!suffix)
        return std::nullopt;
    if (*suffix == SLOT_SUFFIX_A) return 0;
    if (*suffix == SLOT_SUFFIX_B) return 1;
    return std::nullopt;
}

void usage(std::ostream& out, const char* prog) {
    out << prog << " - command-line wrapper for boot slot info (property/cmdline fallback).\n"
        << "\nUsage:\n  " << prog << " COMMAND [SLOT]\n"
        << "\nCommands:\n"
        << "  hal-info               - Show info (fallback mode, no HAL).\n"
        << "  get-number-slots       - Prints number of slots (2).\n"
        << "  get-current-slot       - Prints currently running slot (0 or 1).\n"
        << "  get-active-boot-slot    - Prints slot for next boot (same as current here).\n"
        << "  get-suffix SLOT        - Prints suffix for SLOT (_a or _b).\n"
        << "  mark-boot-successful   - Not supported (requires HAL).\n"
        << "  set-active-boot-slot   - Not supported (requires HAL).\n"
        << "  set-slot-as-unbootable - Not supported (requires HAL).\n"
        << "  is-slot-bootable       - Not supported (requires HAL).\n"
        << "  is-slot-marked-successful - Not supported (requires HAL).\n"
        << "\nSLOT is 0 or 1.\n";
}

uint32_t parseSlot(int pos, int argc, char* argv[], const char* prog) {
    if (pos >= argc) {
        usage(std::cerr, prog);
        std::exit(EX_USAGE);
    }
    errno = 0;
    char* end = nullptr;
    unsigned long val = std::strtoul(argv[pos], &end, 10);
    if (errno != 0 || (end && *end != '\0') || val > 1) {
        usage(std::cerr, prog);
        std::exit(EX_USAGE);
    }
    return static_cast<uint32_t>(val);
}

}  // namespace

extern "C" {

int bootctl_main(int argc, char** argv) {
    const char* prog = (argv && argv[0]) ? argv[0] : "bootctl";

    if (argc < 2) {
        usage(std::cerr, prog);
        return EX_USAGE;
    }

    std::string_view cmd(argv[1]);

    if (cmd == "hal-info") {
        std::cout << "HAL Version: (fallback) property/cmdline, no boot_control HAL\n";
        return EX_OK;
    }

    if (cmd == "get-number-slots") {
        std::cout << NUM_SLOTS << '\n';
        return EX_OK;
    }

    if (cmd == "get-current-slot") {
        auto slot = getCurrentSlotNumber();
        if (!slot) {
            std::cerr << "Could not determine current slot (no slot_suffix in /proc/cmdline)\n";
            return EX_SOFTWARE;
        }
        std::cout << *slot << '\n';
        return EX_OK;
    }

    if (cmd == "get-active-boot-slot") {
        auto slot = getCurrentSlotNumber();
        if (!slot) {
            std::cerr << "Could not determine active boot slot\n";
            return EX_SOFTWARE;
        }
        std::cout << *slot << '\n';
        return EX_OK;
    }

    if (cmd == "get-suffix") {
        uint32_t n = parseSlot(2, argc, argv, prog);
        std::cout << (n == 0 ? SLOT_SUFFIX_A : SLOT_SUFFIX_B) << '\n';
        return EX_OK;
    }

    if (cmd == "mark-boot-successful" || cmd == "set-active-boot-slot" ||
        cmd == "set-slot-as-unbootable" || cmd == "is-slot-bootable" ||
        cmd == "is-slot-marked-successful") {
        std::cerr << "Command '" << cmd << "' requires boot_control HAL (not available in fallback mode)\n";
        return EX_UNAVAILABLE;
    }

    usage(std::cerr, prog);
    return EX_USAGE;
}

}  // extern "C"

#if defined(BOOTCTL_STANDALONE)
int main(int argc, char** argv) {
    return bootctl_main(argc, argv);
}
#endif
