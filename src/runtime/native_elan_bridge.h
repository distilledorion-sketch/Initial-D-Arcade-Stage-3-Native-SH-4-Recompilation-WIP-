/*
 * Native NAOMI 2 ELAN command core for the Initial D static recompiler.
 *
 * The command traversal in NativeElanControlWalker is adapted from Flycast
 * v2.6 core/hw/pvr/elan.cpp and core/hw/pvr/elan_struct.h, pinned at commit
 * 392a429e8b040b3e5bf6696cb4f984274fc44123. Flycast is Copyright 2022
 * flyinghead and is licensed under GNU GPL version 2 or later.
 *
 * This file is therefore distributed under the GNU General Public License;
 * you may redistribute it and/or modify it under the terms of version 2 of
 * that License, or (at your option) any later version. It is provided without
 * warranty; without even the implied warranty of merchantability or fitness
 * for a particular purpose. See LICENSE and docs/FLYCAST_ELAN_ATTRIBUTION.md.
 *
 * Only the ELAN command grammar and traversal are retained here. Flycast's
 * SH-4 CPU, address space, renderer, scheduler, networking, and save-state
 * systems are not linked into the recompiled product.
 */
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

// Native-recomp seam for NAOMI2 graphics.
// The SH-4 game remains responsible for building its 32-byte ELAN commands.
// At the exact hardware submission boundary, the stripped native core
// validates and consumes the supported command stream. Unsupported input is
// rejected; there is no interpreter or emulator fallback.

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

// Small native control-stream walker derived from the observable command
// behavior in Flycast's ELAN executeCommand loop, without importing Flycast's
// emulator scheduler, memory bus, TA renderer, save-state, or networking
// dependencies.  It does not render geometry.  Its only output is a validated,
// ordered list of hardware-side-effect commands for SH4Memory to execute.
enum class NativeElanWalkFailure : uint8_t {
    None = 0,
    InvalidRange,
    TruncatedRecord,
    InvalidVertexLayout,
    DepthLimit,
    StreamCycle,
    RecordBudget,
    EventBudget,
    InvalidTaRecord,
    UnknownNaomi2Command,
    InvalidRegisterWait,
};

struct NativeElanControlEvent {
    NativeElanCommand command{};
    uint32_t recordOffset = 0;
    uint32_t depth = 0;
};

struct NativeElanControlWalkResult {
    bool ok = false;
    NativeElanWalkFailure failure = NativeElanWalkFailure::None;
    uint32_t streams = 0;
    uint32_t records = 0;
    uint32_t recursiveLinks = 0;
    uint32_t recursiveModels = 0;
    uint32_t maxDepth = 0;
    uint32_t cycles = 0;
    std::vector<NativeElanControlEvent> events;
};

class NativeElanControlWalker {
public:
    static constexpr uint32_t kMaxDepth = 32u;
    static constexpr uint32_t kMaxRecords = 262144u;
    static constexpr uint32_t kMaxEvents = 65536u;

    static NativeElanControlWalkResult walk(
            const uint8_t* ram, size_t ramSize,
            uint32_t rootOffset, uint32_t rootSize) {
        NativeElanControlWalkResult result{};
        if (!ram || ramSize == 0u) {
            result.failure = NativeElanWalkFailure::InvalidRange;
            return result;
        }
        std::vector<uint64_t> activeStreams;
        result.ok = walkStream(
            ram, ramSize, rootOffset, rootSize, 0u, activeStreams, result);
        if (result.ok) result.failure = NativeElanWalkFailure::None;
        return result;
    }

private:
    static bool hasRange(size_t ramSize, uint32_t offset, uint64_t size) {
        return static_cast<uint64_t>(offset) + size <=
            static_cast<uint64_t>(ramSize);
    }

    static void fail(NativeElanControlWalkResult& result,
                     NativeElanWalkFailure failure) {
        if (result.failure == NativeElanWalkFailure::None)
            result.failure = failure;
    }

    static NativeElanCommand readCommand(const uint8_t* ram, uint32_t offset) {
        NativeElanCommand command{};
        std::memcpy(command.w, ram + offset, sizeof(command.w));
        return command;
    }

    static uint32_t vertexSize(uint32_t flags) {
        switch (flags) {
        case 0x002u: return 16u;
        case 0x00Au: return 24u;
        case 0x00Eu: return 40u;
        case 0x042u: return 24u;
        case 0x04Au: return 32u;
        case 0x10Au: return 40u;
        default: return 0u;
        }
    }

    // Stripped form of Flycast's TaTypeLut sizing rules used by ELAN's
    // executeCommand<false> path. Only record width is needed here; the
    // native framebuffer owns geometry conversion.
    static bool rawPolygonLayout(uint32_t pcw, uint32_t& headerSize,
                                 uint32_t& rawVertexSize) {
        const uint32_t objectControl = pcw & 0xFFu;
        const bool uv16 = (objectControl & 0x01u) != 0u;
        const bool offset = (objectControl & 0x04u) != 0u;
        const bool texture = (objectControl & 0x08u) != 0u;
        const uint32_t colorType = (objectControl >> 4u) & 3u;
        const bool volume = (objectControl & 0x40u) != 0u;

        if (volume && colorType == 1u) return false;
        headerSize = (!volume && colorType == 2u && texture && offset) ||
                     (volume && colorType == 2u) ? 64u : 32u;

        uint32_t vertexType = 0u;
        if (texture) {
            if (!volume) {
                if (colorType == 0u) vertexType = uv16 ? 4u : 3u;
                else if (colorType == 1u) vertexType = uv16 ? 6u : 5u;
                else vertexType = uv16 ? 8u : 7u;
            } else {
                if (colorType == 0u) vertexType = uv16 ? 12u : 11u;
                else vertexType = uv16 ? 14u : 13u;
            }
        } else if (!volume) {
            vertexType = colorType == 0u ? 0u : colorType == 1u ? 1u : 2u;
        } else {
            vertexType = colorType == 0u ? 9u : 10u;
        }
        rawVertexSize = vertexType == 5u || vertexType == 6u ||
                        (vertexType >= 11u && vertexType <= 14u) ? 64u : 32u;
        return true;
    }

    static bool appendEvent(NativeElanControlWalkResult& result,
                            const NativeElanCommand& command,
                            uint32_t offset, uint32_t depth) {
        if (result.events.size() >= kMaxEvents) {
            fail(result, NativeElanWalkFailure::EventBudget);
            return false;
        }
        NativeElanControlEvent event{};
        event.command = command;
        event.recordOffset = offset;
        event.depth = depth;
        result.events.push_back(event);
        return true;
    }

    static bool walkStream(
            const uint8_t* ram, size_t ramSize,
            uint32_t start, uint32_t size, uint32_t depth,
            std::vector<uint64_t>& activeStreams,
            NativeElanControlWalkResult& result) {
        if (!ram || ramSize == 0u || !hasRange(ramSize, start, size)) {
            fail(result, NativeElanWalkFailure::InvalidRange);
            return false;
        }
        if (depth > kMaxDepth) {
            fail(result, NativeElanWalkFailure::DepthLimit);
            return false;
        }

        const uint64_t key = (static_cast<uint64_t>(start) << 32u) | size;
        if (std::find(activeStreams.begin(), activeStreams.end(), key) !=
            activeStreams.end()) {
            ++result.cycles;
            fail(result, NativeElanWalkFailure::StreamCycle);
            return false;
        }
        activeStreams.push_back(key);
        ++result.streams;
        result.maxDepth = std::max(result.maxDepth, depth);

        const uint64_t end = static_cast<uint64_t>(start) + size;
        uint32_t pos = start;
        int32_t rawListType = -1;
        uint32_t rawVertexSize = 32u;
        bool valid = true;
        while (static_cast<uint64_t>(pos) + 32u <= end) {
            if (result.records >= kMaxRecords) {
                fail(result, NativeElanWalkFailure::RecordBudget);
                valid = false;
                break;
            }

            const NativeElanCommand command = readCommand(ram, pos);
            ++result.records;
            uint64_t recordSize = 32u;

            if (command.isNaomi2()) {
                const uint32_t cid = command.n2Command();
                switch (cid) {
                case 0x0u: // Null
                case 0x3u: // ProjMatrix
                case 0xEu: // RegisterWait
                case 0xFu: // Link
                    recordSize = 32u;
                    break;
                case 0x4u: // InstanceMatrix, LightModel, or Light
                    recordSize = command.w[1] == 0xFu && command.w[2] == 0x7Fu
                        ? 160u : 32u;
                    break;
                case 0x5u: // GMP
                    recordSize = 64u;
                    break;
                case 0x7u: { // ICH header plus packed vertices
                    const uint32_t stride = vertexSize(command.w[6]);
                    const uint32_t count = command.w[7];
                    if (!stride || count > 1000000u) {
                        fail(result, NativeElanWalkFailure::InvalidVertexLayout);
                        valid = false;
                        break;
                    }
                    recordSize = 32ull + static_cast<uint64_t>(stride) * count;
                    break;
                }
                case 0x8u: // Model; its referenced stream is walked below
                    recordSize = 32u;
                    break;
                default:
                    fail(result, NativeElanWalkFailure::UnknownNaomi2Command);
                    valid = false;
                    break;
                }
            } else {
                // Exact record-width portion of Flycast's inactive ELAN/TA
                // parser. It preserves alignment without importing the TA
                // renderer or accepting malformed data as 32-byte commands.
                const uint32_t paraType = (command.w[0] >> 29u) & 7u;
                switch (paraType) {
                case 0u: // End Of List
                    rawListType = -1;
                    recordSize = 32u;
                    break;
                case 1u: // Object List Set
                case 2u: // User Tile Clip
                    recordSize = 32u;
                    break;
                case 4u: { // Polygon or Modifier Volume
                    if (rawListType == -1)
                        rawListType = static_cast<int32_t>((command.w[0] >> 24u) & 7u);
                    if ((rawListType & 1) != 0) {
                        rawVertexSize = 64u;
                        recordSize = 32u;
                    } else {
                        uint32_t headerSize = 0u;
                        if (!rawPolygonLayout(command.w[0], headerSize,
                                              rawVertexSize)) {
                            fail(result, NativeElanWalkFailure::InvalidTaRecord);
                            valid = false;
                        } else {
                            recordSize = headerSize;
                        }
                    }
                    break;
                }
                case 5u: // Sprite
                    if (rawListType == -1)
                        rawListType = static_cast<int32_t>((command.w[0] >> 24u) & 7u);
                    rawVertexSize = 64u;
                    recordSize = 32u;
                    break;
                case 7u: // Vertex Parameter
                    recordSize = rawVertexSize;
                    break;
                default:
                    fail(result, NativeElanWalkFailure::InvalidTaRecord);
                    valid = false;
                    break;
                }
            }

            if (!valid) break;

            if (static_cast<uint64_t>(pos) + recordSize > end ||
                !hasRange(ramSize, pos, recordSize)) {
                fail(result, NativeElanWalkFailure::TruncatedRecord);
                valid = false;
                break;
            }

            // At this point command alignment has been established by the
            // Flycast-derived grammar, so side effects do not rely on scanning
            // payload bytes for command-looking bit patterns.
            if (command.isNaomi2()) {
                const uint32_t cid = command.n2Command();
                if (cid == 0x8u) {
                    ++result.recursiveModels;
                    const uint32_t modelSize = command.w[6];
                    if (modelSize != 0u) {
                        const uint32_t target = command.w[4] &
                            static_cast<uint32_t>(ramSize - 1u);
                        if (!walkStream(ram, ramSize, target, modelSize,
                                        depth + 1u, activeStreams, result))
                            valid = false;
                    }
                } else if (cid == 0xFu) {
                    switch (command.kind()) {
                    case NativeElanKind::LinkRecursive:
                    ++result.recursiveLinks;
                    if (command.size() != 0u) {
                        const uint32_t target = command.offset() &
                            static_cast<uint32_t>(ramSize - 1u);
                        if (!walkStream(ram, ramSize, target, command.size(),
                                        depth + 1u, activeStreams, result)) {
                            valid = false;
                        }
                    }
                    break;
                    case NativeElanKind::LinkTextureDmaSh4:
                    case NativeElanKind::LinkTextureDmaEram:
                    if (!appendEvent(result, command, pos, depth))
                        valid = false;
                    break;
                    default:
                        fail(result, NativeElanWalkFailure::UnknownNaomi2Command);
                        valid = false;
                        break;
                    }
                } else if (cid == 0xEu) {
                    if (command.offset() != 0xFFFFFFFFu && command.waitMask() != 0u) {
                        if (command.kind() != NativeElanKind::RegisterWaitPunchthrough) {
                            fail(result, NativeElanWalkFailure::InvalidRegisterWait);
                            valid = false;
                        } else if (!appendEvent(result, command, pos, depth)) {
                            valid = false;
                        }
                    }
                }
            }

            if (!valid) break;
            pos = static_cast<uint32_t>(
                static_cast<uint64_t>(pos) + recordSize);
        }

        activeStreams.pop_back();
        return valid;
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
    uint64_t rejected = 0;
    uint64_t n2Command[16]{};
    uint64_t linkRecursive = 0;
    uint64_t linkTextureDmaSh4 = 0;
    uint64_t linkTextureDmaEram = 0;
    uint64_t registerWaitPunchthrough = 0;
    uint64_t streamWalks = 0;
    uint64_t streamWalkStreams = 0;
    uint64_t streamWalkRecords = 0;
    uint64_t streamWalkEvents = 0;
    uint64_t streamWalkFailures = 0;
    uint64_t streamWalkCycles = 0;
    uint64_t streamWalkMaxDepth = 0;
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
