/**
 * Minimal bootctl — A/B slot info via properties (no HIDL).
 * Compatible with AOSP bootctl commands: get-number-slots, get-current-slot, get-suffix, get-active-boot-slot.
 * set-active-boot-slot / mark-boot-successful require Boot Control HAL and are not implemented here.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(BOOTCTL_ANDROID)
#include <sys/system_properties.h>
#include <android/log.h>
#define LOG_TAG "bootctl"
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)
#else
#define LOGE(fmt, ...) fprintf(stderr, "bootctl: " fmt "\n", ##__VA_ARGS__)
#endif

#define PROP_VALUE_MAX 92

static void usage(const char* prog) {
    fprintf(stderr,
        "Usage: %s COMMAND [SLOT]\n"
        "Commands (property-based; no HAL):\n"
        "  get-number-slots        Print number of slots (2 for A/B).\n"
        "  get-current-slot        Print current slot index (0 or 1).\n"
        "  get-active-boot-slot    Same as get-current-slot.\n"
        "  get-suffix SLOT        Print suffix for SLOT (0=_a, 1=_b). SLOT is 0-based.\n"
        "  -h, --help              Show this help.\n"
        "Not implemented (need Boot Control HAL): set-active-boot-slot, mark-boot-successful, etc.\n",
        prog);
}

#if defined(BOOTCTL_ANDROID)

static int prop_get(const char* name, char* value) {
    return __system_property_get(name, value);
}

// Parse ro.boot.slot_suffix (e.g. "_a", "_b") -> 0 or 1. Default 0 if unset.
static int get_current_slot_from_prop() {
    char buf[PROP_VALUE_MAX];
    if (prop_get("ro.boot.slot_suffix", buf) <= 0)
        return 0;
    if (strcmp(buf, "_b") == 0)
        return 1;
    return 0; // "_a" or anything else
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
    }

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    const char* cmd = argv[1];

    if (strcmp(cmd, "get-number-slots") == 0) {
        printf("2\n");
        return 0;
    }

    if (strcmp(cmd, "get-current-slot") == 0 || strcmp(cmd, "get-active-boot-slot") == 0) {
        printf("%u\n", (unsigned)get_current_slot_from_prop());
        return 0;
    }

    if (strcmp(cmd, "get-suffix") == 0) {
        if (argc < 3) {
            LOGE("get-suffix requires SLOT (0 or 1)");
            return 1;
        }
        unsigned slot = (unsigned)strtoul(argv[2], nullptr, 10);
        if (slot == 0)
            printf("_a\n");
        else if (slot == 1)
            printf("_b\n");
        else {
            LOGE("invalid slot %u (use 0 or 1)", slot);
            return 1;
        }
        return 0;
    }

    // Unsupported commands
    if (strcmp(cmd, "set-active-boot-slot") == 0 || strcmp(cmd, "mark-boot-successful") == 0 ||
        strcmp(cmd, "set-slot-as-unbootable") == 0 || strcmp(cmd, "is-slot-bootable") == 0 ||
        strcmp(cmd, "is-slot-marked-successful") == 0 || strcmp(cmd, "hal-info") == 0) {
        LOGE("command '%s' requires Boot Control HAL; not implemented in this build", cmd);
        return 1;
    }

    usage(argv[0]);
    return 1;
}

#else

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    LOGE("bootctl is for Android only. Build with Android NDK for device.");
    usage(argv[0]);
    return 1;
}

#endif
