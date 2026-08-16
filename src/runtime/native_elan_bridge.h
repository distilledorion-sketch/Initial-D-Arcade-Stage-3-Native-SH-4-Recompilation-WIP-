#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>

// First hybrid-recomp seam for NAOMI2 graphics.
// The SH-4 game remains responsible for building its 32-byte ELAN commands.
// At the exact hardware submission boundary, a native handler may consume a
// command.  Anything not explicitly understood falls back to the existing
// guest-facing ELAN model.

enum class NativeElanKind : uint8_t {
    Unknown = 0,
    LinkRecursive,
    LinkTextureDmaSh4,
    LinkTextureDmaEram,
    RegisterWaitPunchthrough,
};

struct NativeElanCommand {
    uint32_t w[8]{};

    static constexpr uint32_t kRegisterWaitListCompleteMask =
        0x00000080u | 0x00000100u | 0x00000200u |
        0x00000400u | 0x00200000u;

    uint32_t pcw() const { return w[0]; }
    bool isNaomi2() const { return (w[0] & (1u << 27)) != 0; }
    uint32_t n2Command() const { return (w[0] >> 8) & 0xFu; }
    // isNaomi2() is sufficient only after the hardware parser has established
    // a command boundary.  A raw 32-byte store-queue block can begin with
    // vertex float data whose bit 27 happens to be set.  This stricter shape
    // recognizes the canonical control headers used by diagnostics without
    // mistaking those payload words for commands.
    bool isCanonicalControlHeader() const {
        return (w[0] & 0xFFFFF0FFu) == 0x08000000u;
    }
    bool isLink() const { return isNaomi2() && n2Command() == 0xFu; }
    bool isRegisterWait() const { return isNaomi2() && n2Command() == 0xEu; }
    uint32_t offset() const { return w[1]; }
    uint32_t vramAddress() const { return w[2]; }
    uint32_t size() const { return w[3]; }
    uint32_t waitMask() const { return w[3]; }

    NativeElanKind kind() const {
        // GDS-0033 uses RegisterWait for all five list-complete events.
        // Keep the legacy enum name so existing counters/log consumers remain
        // stable, but admit only non-empty combinations of the known bits.
        if (isRegisterWait() &&
            (offset() == 0x005F6903u || offset() == 0x025F6903u) &&
            waitMask() != 0u &&
            (waitMask() & ~kRegisterWaitListCompleteMask) == 0u)
            return NativeElanKind::RegisterWaitPunchthrough;
        if (!isLink()) return NativeElanKind::Unknown;
        if (offset() & 0x80000000u) return NativeElanKind::LinkTextureDmaSh4;
        if (offset() & 0x20000000u) return NativeElanKind::LinkTextureDmaEram;
        return NativeElanKind::LinkRecursive;
    }
};

struct NativeElanRecursiveSnapshot {
    uint32_t offset = 0;
    uint32_t size = 0;
    uint32_t copied = 0;
    uint32_t words[32]{}; // first 128 bytes, captured at submission time
};

struct NativeElanBridgeStats {
    uint64_t observed = 0;
    uint64_t handled = 0;
    uint64_t fallback = 0;
    uint64_t n2Command[16]{};
    uint64_t linkRecursive = 0;
    uint64_t linkTextureDmaSh4 = 0;
    uint64_t linkTextureDmaEram = 0;
    uint64_t registerWaitPunchthrough = 0;
};

// Opt-in provenance for the game's native InstanceMatrix submitter.  The AOT
// body at 0C1D7E20 snapshots the named XF matrix once, then emits the command
// through five store-queue commits.  Keeping this as host-only metadata lets
// the resident capture correlate a decoded matrix with the exact guest caller
// without changing any guest register, RAM byte, or device timing.
struct NativeElanGuestInstanceTraceTag {
    uint64_t sequence = 0;
    uint32_t outerPr = 0;
    uint32_t stack = 0;
    uint32_t fpscr = 0;
    uint32_t sourceMatrix[16]{};
    bool pending = false;
};

inline thread_local NativeElanGuestInstanceTraceTag g_nativeElanGuestInstanceTraceTag{};

inline bool nativeElanGuestInstanceTraceRequested() {
    static const bool requested = [] {
        const char* v = std::getenv("IDAS3_NATIVE_ELAN_INSTANCE_GUEST_TRACE");
        return v && *v && std::strcmp(v, "0") != 0;
    }();
    return requested;
}

inline bool nativeElanBridgeRequested() {
    static const bool requested = [] {
        const char* v = std::getenv("IDAS3_NATIVE_ELAN");
        return v && *v && std::strcmp(v, "0") != 0;
    }();
    return requested;
}

inline bool nativeElanSubmittedCaptureRequested() {
    static const bool requested = [] {
        const char* v = std::getenv("IDAS3_REPLAY_USE_SUBMITTED_SCENE");
        return v && *v && std::strcmp(v, "0") != 0;
    }();
    return requested;
}
