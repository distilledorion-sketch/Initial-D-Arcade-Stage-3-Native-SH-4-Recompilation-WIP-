#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include "native_elan_bridge.h"

// Clean-room structural decoder for the NAOMI2 ELAN command formats already
// proven by the old IDAS3Recomp capture tooling.  This is deliberately
// read-only: it observes ELAN RAM at submission time and never mutates guest
// state or fabricates hardware completion.

struct NativeElanVertexSample {
    uint32_t ichOffset = 0;
    uint32_t header = 0;
    uint32_t flags = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 1.0f;
    float u = 0.0f;
    float v = 0.0f;
    uint32_t baseArgb = 0xFFFFFFFFu;
    uint32_t offsetArgb = 0u;
    bool hasVertexColor = false;
    bool hasUv = false;
};

struct NativeElanProjectionState {
    uint32_t offset = 0;
    // ELAN owns a persistent projection state.  A hard reset starts with
    // these coefficients and RegisterWait/frame state resets do not clear
    // them.  Keeping the exact hardware-facing default prevents a scene
    // submitted before its first explicit ProjMatrix command from being
    // discarded as projection-less.
    float fx = 579.411194f;
    float tx = -320.0f;
    float fy = -579.411194f;
    float ty = -240.0f;
    bool valid = true;
};

struct NativeElanMaterialState {
    uint32_t offset = 0;
    std::array<uint32_t, 16> words{};
    bool valid = false;
};

struct NativeElanInstanceState {
    uint32_t offset = 0;
    float nearValue = 0.0f;
    std::array<float, 9> normalTransform{};
    std::array<float, 12> transform{};
    float farValue = 0.0f;
    float inverseNear = 0.0f;
    // Diagnostic-only guest provenance, populated at the resident completion
    // edge.  These fields are never decoded from or written to ELAN RAM.
    uint64_t guestSubmitSequence = 0;
    uint32_t guestOuterPr = 0;
    uint32_t guestStack = 0;
    uint32_t guestFpscr = 0;
    std::array<uint32_t, 16> guestSourceMatrix{};
    bool guestTraceValid = false;
    bool valid = false;
};

struct NativeElanLightModelState {
    uint32_t offset = 0;
    std::array<uint32_t, 8> words{};
    bool valid = false;
};

struct NativeElanLightState {
    uint32_t offset = 0;
    std::array<uint32_t, 8> words{};
    uint32_t lightId = 0;
    bool parallel = false;
    bool valid = false;
};

// ELAN Model command state that is active while the model's sub-stream is
// executed. Flycast applies it at polygon-submission time:
//   pp.tsp.full  ^= modelTSP.full
//   pp.isp.CullMode ^= cullingReversed ^ projFlip
//   pp.pcw.Shadow   ^= shadowedVolume
// Capturing it per batch keeps the translated ICH state faithful instead of
// silently using the raw ICH words.
struct NativeElanModelState {
    uint32_t offset = 0;
    uint32_t pcw = 0;
    uint32_t param = 0;
    uint32_t tsp = 0;
    bool cullingReversed = false;
    bool openVolume = false;
    bool shadowedVolume = false;
    bool valid = false;
};

struct NativeElanDrawBatch {
    uint32_t ichOffset = 0;
    uint32_t pcw = 0;
    uint32_t flags = 0;
    uint32_t vertexCount = 0;
    uint32_t vertexSize = 0;
    uint32_t ispTsp = 0;
    uint32_t tsp0 = 0;
    uint32_t tcw0 = 0;
    uint32_t tsp1 = 0;
    uint32_t tcw1 = 0;
    uint32_t finiteVertices = 0;
    NativeElanProjectionState projection{};
    NativeElanMaterialState material{};
    NativeElanInstanceState instance{};
    NativeElanLightModelState lightModel{};
    NativeElanModelState model{};
    std::vector<NativeElanLightState> lights;
    std::vector<NativeElanVertexSample> vertices;
};

struct NativeElanDecodeResult {
    uint32_t rootOffset = 0;
    uint32_t rootSize = 0;
    uint32_t streams = 0;
    uint32_t commands = 0;
    uint32_t links = 0;
    uint32_t models = 0;
    uint32_t gmps = 0;
    uint32_t ichs = 0;
    uint32_t completeIchs = 0;
    uint32_t instanceMatrices = 0;
    uint32_t projectionMatrices = 0;
    uint32_t unknownRecords = 0;
    uint32_t nonElanRecords = 0;
    uint32_t vertices = 0;
    uint32_t maxDepth = 0;
    // Flycast executes every Link/Model occurrence.  The original diagnostic
    // decoder used an address/size visited set to suppress repeated streams,
    // which can incorrectly drop shared geometry submitted with new render
    // state.  Count that condition in all builds; an opt-in A/B path below
    // re-executes it while retaining the existing depth/payload bounds.
    uint32_t streamRevisits = 0;
    uint32_t reexecutedStreamRevisits = 0;
    uint32_t registerWaits = 0;
    uint32_t activeRegisterWaits = 0;
    uint32_t registerWaitStateResets = 0;
    std::vector<NativeElanModelState> modelStates;
    uint32_t firstIchOffset = 0;
    uint32_t firstIchFlags = 0;
    uint32_t firstIchVertexCount = 0;
    uint32_t payloadVertices = 0;
    uint32_t droppedDrawBatches = 0;
    std::vector<NativeElanVertexSample> samples;
    std::vector<NativeElanProjectionState> projections;
    std::vector<NativeElanMaterialState> materials;
    std::vector<NativeElanInstanceState> instances;
    std::vector<NativeElanDrawBatch> drawBatches;
};

struct NativeElanFrameDecode {
    uint64_t frame = 0;
    uint32_t linkRecordOffset = 0;
    NativeElanDecodeResult decoded;
};

struct NativeElanSceneDraw {
    uint32_t sourceLinkRecord = 0;
    uint32_t sourceRootOffset = 0;
    NativeElanDrawBatch batch;
};

struct NativeElanFrameScene {
    uint64_t frame = 0;
    uint32_t sourceLinks = 0;
    uint32_t decodedLinks = 0;
    uint32_t rawBatches = 0;
    uint32_t duplicateBatches = 0;
    uint32_t rawVertices = 0;
    uint32_t uniqueVertices = 0;
    std::vector<NativeElanProjectionState> projections;
    std::vector<NativeElanMaterialState> materials;
    std::vector<NativeElanInstanceState> instances;
    std::vector<NativeElanSceneDraw> draws;
};

// ELAN render state is global across command-port submissions and recursive
// Link/Model streams.  Keep it outside an individual decode result so the
// frame-final diagnostic path can replay the exact submitted command order
// without resetting projection/material/instance state at every resident
// Link record.
struct NativeElanDecodeState {
    NativeElanProjectionState projection{};
    NativeElanMaterialState material{};
    NativeElanInstanceState instance{};
    NativeElanLightModelState lightModel{};
    // Active Model command context. Scoped to the model's sub-stream exactly
    // as Flycast scopes cullingReversed/openModifierVolume/shadowedVolume/
    // modelTSP around its recursive executeCommand call.
    NativeElanModelState model{};
    std::array<NativeElanLightState, 16> lights{};
};

inline uint32_t nativeElanReadU32(const uint8_t* ram, size_t ramSize, uint32_t off) {
    if (!ram || static_cast<uint64_t>(off) + 4u > ramSize) return 0;
    uint32_t v = 0;
    std::memcpy(&v, ram + off, sizeof(v));
    return v;
}

inline float nativeElanU32ToFloat(uint32_t v) {
    float f = 0.0f;
    std::memcpy(&f, &v, sizeof(f));
    return f;
}

inline int nativeElanCommandId(uint32_t word0) {
    if ((word0 & 0x08000000u) == 0) return -1;
    return static_cast<int>((word0 >> 8) & 0xFu);
}

inline uint32_t nativeElanVertexSize(uint32_t flags) {
    switch (flags) {
        case 0x002u: return 16u; // V
        case 0x00Au: return 24u; // VU
        case 0x00Eu: return 40u; // VNU
        case 0x042u: return 24u; // VR
        case 0x04Au: return 32u; // VUR
        case 0x10Au: return 40u; // VUB
        default: return 0u;
    }
}

class NativeElanDecoder {
public:
    static bool readProjectionState(const uint8_t* ram, size_t ramSize, uint32_t off,
                                    NativeElanProjectionState& projection) {
        if (!ram || static_cast<uint64_t>(off) + 32u > ramSize ||
            nativeElanReadU32(ram, ramSize, off) != 0x08000300u)
            return false;
        projection = {};
        projection.offset = off;
        projection.fx = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, off + 8u));
        projection.tx = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, off + 12u));
        projection.fy = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, off + 16u));
        projection.ty = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, off + 20u));
        projection.valid = std::isfinite(projection.fx) && std::isfinite(projection.tx) &&
            std::isfinite(projection.fy) && std::isfinite(projection.ty);
        return projection.valid;
    }

    static bool readInstanceState(const uint8_t* ram, size_t ramSize, uint32_t off,
                                  NativeElanInstanceState& instance) {
        if (!ram || static_cast<uint64_t>(off) + 160u > ramSize ||
            nativeElanReadU32(ram, ramSize, off) != 0x08000400u ||
            nativeElanReadU32(ram, ramSize, off + 4u) != 0x0000000Fu ||
            nativeElanReadU32(ram, ramSize, off + 8u) != 0x0000007Fu)
            return false;
        instance = {};
        instance.offset = off;
        for (unsigned i = 0; i < 9; ++i)
            instance.normalTransform[i] = nativeElanU32ToFloat(
                nativeElanReadU32(ram, ramSize, off + (10u + i) * 4u));
        instance.nearValue = nativeElanU32ToFloat(
            nativeElanReadU32(ram, ramSize, off + 25u * 4u));
        for (unsigned i = 0; i < 12; ++i)
            instance.transform[i] = nativeElanU32ToFloat(
                nativeElanReadU32(ram, ramSize, off + (26u + i) * 4u));
        instance.farValue = nativeElanU32ToFloat(
            nativeElanReadU32(ram, ramSize, off + 38u * 4u));
        instance.inverseNear = nativeElanU32ToFloat(
            nativeElanReadU32(ram, ramSize, off + 39u * 4u));
        instance.valid = std::isfinite(instance.nearValue) &&
            std::all_of(instance.normalTransform.begin(), instance.normalTransform.end(),
                        [](float value) { return std::isfinite(value); }) &&
            std::all_of(instance.transform.begin(), instance.transform.end(),
                        [](float value) { return std::isfinite(value); }) &&
            std::isfinite(instance.farValue) && std::isfinite(instance.inverseNear);
        return instance.valid;
    }

    // Command 4 also carries standalone light-model and light records. The
    // game commits these 32-byte records to resident ELAN RAM before linking
    // model streams, so the submission-order capture must retain them just as
    // it retains projection and instance state.
    static bool readLightingState(const uint8_t* ram, size_t ramSize, uint32_t off,
                                  NativeElanDecodeState& state) {
        if (!ram || static_cast<uint64_t>(off) + 32u > ramSize ||
            nativeElanCommandId(nativeElanReadU32(ram, ramSize, off)) != 0x4)
            return false;
        uint32_t words[8]{};
        for (unsigned i = 0; i < 8; ++i)
            words[i] = nativeElanReadU32(ram, ramSize, off + i * 4u);
        if (words[1] == 0xFu && words[2] == 0x7Fu) return false;
        decodeLightWords(words, off, state);
        return true;
    }

    static NativeElanDecodeResult decode(const uint8_t* ram, size_t ramSize,
                                         uint32_t start, uint32_t size,
                                         size_t maxSamples = 32u,
                                         size_t maxPayloadVertices = 4096u) {
        NativeElanDecodeResult r{};
        r.rootOffset = start;
        r.rootSize = size;
        std::vector<uint64_t> visited;
        NativeElanDecodeState state{};
        decodeStream(ram, ramSize, start, size, 0u, maxSamples,
                     maxPayloadVertices, visited, state, r);
        return r;
    }

    static NativeElanDecodeResult decodeWithState(
            const uint8_t* ram, size_t ramSize,
            uint32_t start, uint32_t size,
            NativeElanDecodeState& state,
            size_t maxSamples = 32u,
            size_t maxPayloadVertices = 4096u) {
        NativeElanDecodeResult r{};
        r.rootOffset = start;
        r.rootSize = size;
        std::vector<uint64_t> visited;
        decodeStream(ram, ramSize, start, size, 0u, maxSamples,
                     maxPayloadVertices, visited, state, r);
        return r;
    }

    static NativeElanDecodeResult decodeCommandSequence(
            const uint8_t* ram, size_t ramSize,
            const std::vector<NativeElanCommand>& commands,
            NativeElanDecodeState& state,
            size_t maxSamples = 32u,
            size_t maxPayloadVertices = 262144u) {
        NativeElanDecodeResult r{};
        r.rootSize = static_cast<uint32_t>(commands.size() * 32u);
        bool haveRoot = false;
        for (const auto& cmd : commands) {
            if (!cmd.isNaomi2()) {
                ++r.nonElanRecords;
                continue;
            }
            const uint32_t cid = cmd.n2Command();
            if (cid == 0x3u) {
                ++r.commands;
                ++r.projectionMatrices;
                NativeElanProjectionState projection{};
                projection.fx = nativeElanU32ToFloat(cmd.w[2]);
                projection.tx = nativeElanU32ToFloat(cmd.w[3]);
                projection.fy = nativeElanU32ToFloat(cmd.w[4]);
                projection.ty = nativeElanU32ToFloat(cmd.w[5]);
                projection.valid = std::isfinite(projection.fx) &&
                    std::isfinite(projection.tx) && std::isfinite(projection.fy) &&
                    std::isfinite(projection.ty);
                // A submitted ProjMatrix consists of four finite floats.  The
                // resident-ring fallback can observe an old 0x08000300 header
                // after its payload has already been overwritten.  Do not let
                // that diagnostic capture artifact poison all following ICHs;
                // Flycast would have consumed the complete command when it was
                // originally submitted.
                if (projection.valid) {
                    state.projection = projection;
                    r.projections.push_back(projection);
                }
                continue;
            }
            if (cid == 0xFu) {
                ++r.commands;
                ++r.links;
                if (cmd.kind() == NativeElanKind::LinkRecursive && cmd.size() != 0u) {
                    const uint32_t root = cmd.offset() & 0x01FFFFF8u;
                    if (!haveRoot) {
                        r.rootOffset = root;
                        haveRoot = true;
                    }
                    std::vector<uint64_t> visited;
                    decodeStream(ram, ramSize, root, cmd.size(), 1u,
                                 maxSamples, maxPayloadVertices, visited, state, r);
                }
                continue;
            }
            if (cid == 0x4u) {
                ++r.commands;
                decodeLightWords(cmd.w, 0u, state);
                continue;
            }
            if (cid == 0xEu) {
                ++r.commands;
                ++r.registerWaits;
                // Flycast's State::reset() clears per-object state at a real
                // RegisterWait but deliberately preserves the projection
                // coefficients until another projection command arrives.
                if (cmd.offset() != 0xFFFFFFFFu && cmd.waitMask() != 0u) {
                    ++r.activeRegisterWaits;
                    state.material = {};
                    state.instance = {};
                    state.lightModel = {};
                    state.lights = {};
                    ++r.registerWaitStateResets;
                }
                continue;
            }
            ++r.commands;
        }
        return r;
    }

private:
    static bool hasRange(size_t ramSize, uint32_t off, uint64_t len) {
        return static_cast<uint64_t>(off) + len <= static_cast<uint64_t>(ramSize);
    }

    static bool markVisited(std::vector<uint64_t>& visited, uint32_t off, uint32_t size) {
        const uint64_t key = (static_cast<uint64_t>(off) << 32) | size;
        if (std::find(visited.begin(), visited.end(), key) != visited.end()) return false;
        visited.push_back(key);
        return true;
    }

    static bool reexecuteSharedStreamsRequested() {
        static const bool requested = [] {
            const char* value = std::getenv("IDAS3_NATIVE_ELAN_REEXECUTE_SHARED_STREAMS");
            return value && *value && std::strcmp(value, "0") != 0;
        }();
        return requested;
    }

    static bool resetAtRegisterWaitRequested() {
        static const bool requested = [] {
            const char* value = std::getenv("IDAS3_NATIVE_ELAN_RESET_AT_REGISTER_WAIT");
            return value && *value && std::strcmp(value, "0") != 0;
        }();
        return requested;
    }

    static void decodeLightWords(const uint32_t words[8], uint32_t offset,
                                 NativeElanDecodeState& state) {
        if (words[1] == 0xFu && words[2] == 0x7Fu) return;
        if ((words[1] & 0x10u) != 0u) {
            state.lightModel = {};
            state.lightModel.offset = offset;
            std::copy(words, words + 8, state.lightModel.words.begin());
            state.lightModel.valid = true;
            return;
        }
        const uint32_t lightId = words[1] & 0xFu;
        if (lightId >= state.lights.size()) return;
        NativeElanLightState light{};
        light.offset = offset;
        std::copy(words, words + 8, light.words.begin());
        light.lightId = lightId;
        light.parallel = (words[0] & (1u << 20u)) != 0u;
        light.valid = true;
        state.lights[lightId] = light;
    }

    static void decodeStream(const uint8_t* ram, size_t ramSize,
                             uint32_t start, uint32_t size, uint32_t depth,
                             size_t maxSamples, size_t maxPayloadVertices,
                             std::vector<uint64_t>& visited, NativeElanDecodeState& state,
                             NativeElanDecodeResult& r) {
        if (!ram || size == 0 || depth > 8u || !hasRange(ramSize, start, size)) return;
        if (!markVisited(visited, start, size)) {
            ++r.streamRevisits;
            if (!reexecuteSharedStreamsRequested()) return;
            ++r.reexecutedStreamRevisits;
        }
        ++r.streams;
        r.maxDepth = std::max(r.maxDepth, depth);

        const uint64_t end64 = static_cast<uint64_t>(start) + size;
        uint32_t pos = start;
        while (static_cast<uint64_t>(pos) + 32u <= end64 && hasRange(ramSize, pos, 32u)) {
            uint32_t w[8]{};
            for (unsigned i = 0; i < 8; ++i) w[i] = nativeElanReadU32(ram, ramSize, pos + i * 4u);
            const int cid = nativeElanCommandId(w[0]);

            // 160-byte InstanceMatrix structure.  Its embedded command-looking
            // words must not be split into standalone records.
            if (cid == 0x4 && w[1] == 0xFu && w[2] == 0x7Fu &&
                static_cast<uint64_t>(pos) + 160u <= end64 && hasRange(ramSize, pos, 160u)) {
                ++r.commands;
                ++r.instanceMatrices;
                readInstanceState(ram, ramSize, pos, state.instance);
                r.instances.push_back(state.instance);
                pos += 160u;
                continue;
            }

            if (cid == 0x5) { // GMP: fixed 64-byte block.
                if (static_cast<uint64_t>(pos) + 64u > end64 || !hasRange(ramSize, pos, 64u)) break;
                ++r.commands;
                ++r.gmps;
                state.material = {};
                state.material.offset = pos;
                for (unsigned i = 0; i < 16; ++i)
                    state.material.words[i] = nativeElanReadU32(ram, ramSize, pos + i * 4u);
                state.material.valid = true;
                r.materials.push_back(state.material);
                pos += 64u;
                continue;
            }

            if (cid == 0x4) {
                // Command 4 multiplexes the 160-byte instance/normal matrix,
                // the 32-byte light model, and 32-byte light records.
                ++r.commands;
                decodeLightWords(w, pos, state);
                pos += 32u;
                continue;
            }

            if (cid == 0x7) { // ICH header + packed vertices.
                ++r.commands;
                ++r.ichs;
                const uint32_t flags = w[6];
                const uint32_t count = w[7];
                const uint32_t vsize = nativeElanVertexSize(flags);
                if (r.firstIchOffset == 0u) {
                    r.firstIchOffset = pos;
                    r.firstIchFlags = flags;
                    r.firstIchVertexCount = count;
                }
                if (!vsize || count > 1000000u) break;
                const uint64_t total = 32ull + static_cast<uint64_t>(count) * vsize;
                if (static_cast<uint64_t>(pos) + total > end64 || !hasRange(ramSize, pos, total)) break;

                ++r.completeIchs;
                r.vertices += count;
                const bool retainBatch = count <= maxPayloadVertices &&
                    r.payloadVertices <= maxPayloadVertices - count;
                NativeElanDrawBatch batch{};
                if (retainBatch) {
                    batch.ichOffset = pos;
                    batch.pcw = w[0];
                    batch.flags = flags;
                    batch.vertexCount = count;
                    batch.vertexSize = vsize;
                    batch.ispTsp = w[1];
                    batch.tsp0 = w[2];
                    batch.tcw0 = w[3];
                    batch.tsp1 = w[4];
                    batch.tcw1 = w[5];
                    batch.projection = state.projection;
                    batch.material = state.material;
                    batch.instance = state.instance;
                    batch.lightModel = state.lightModel;
                    batch.model = state.model;
                    if (state.lightModel.valid) {
                        const uint32_t referencedLights =
                            (state.lightModel.words[2] & 0xFFFFu) |
                            (state.lightModel.words[2] >> 16u);
                        for (uint32_t lightId = 0; lightId < state.lights.size(); ++lightId) {
                            if ((referencedLights & (1u << lightId)) != 0u &&
                                state.lights[lightId].valid)
                                batch.lights.push_back(state.lights[lightId]);
                        }
                    }
                    batch.vertices.reserve(count);
                }
                for (uint32_t i = 0; i < count; ++i) {
                    const uint32_t voff = pos + 32u + i * vsize;
                    NativeElanVertexSample s{};
                    s.ichOffset = pos;
                    s.flags = flags;
                    s.header = nativeElanReadU32(ram, ramSize, voff + 0u);
                    s.x = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 4u));
                    s.y = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 8u));
                    s.z = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 12u));
                    s.nx = static_cast<float>(static_cast<int8_t>(s.header & 0xFFu)) / 127.0f;
                    s.ny = static_cast<float>(static_cast<int8_t>((s.header >> 8u) & 0xFFu)) / 127.0f;
                    s.nz = static_cast<float>(static_cast<int8_t>((s.header >> 16u) & 0xFFu)) / 127.0f;
                    if (flags == 0x00Eu && vsize >= 40u) {
                        s.nx = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 16u));
                        s.ny = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 20u));
                        s.nz = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 24u));
                        s.u = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 32u));
                        s.v = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 36u));
                        s.hasUv = true;
                    } else if ((flags == 0x00Au || flags == 0x04Au || flags == 0x10Au) &&
                               vsize >= 24u) {
                        s.u = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 16u));
                        s.v = nativeElanU32ToFloat(nativeElanReadU32(ram, ramSize, voff + 20u));
                        s.hasUv = true;
                    }
                    if (flags == 0x042u && vsize >= 24u) {
                        s.baseArgb = nativeElanReadU32(ram, ramSize, voff + 16u);
                        s.offsetArgb = nativeElanReadU32(ram, ramSize, voff + 20u);
                        s.hasVertexColor = true;
                    } else if (flags == 0x04Au && vsize >= 32u) {
                        s.baseArgb = nativeElanReadU32(ram, ramSize, voff + 24u);
                        s.offsetArgb = nativeElanReadU32(ram, ramSize, voff + 28u);
                        s.hasVertexColor = true;
                    }
                    if (r.samples.size() < maxSamples) r.samples.push_back(s);
                    if (retainBatch) {
                        if (std::isfinite(s.x) && std::isfinite(s.y) && std::isfinite(s.z))
                            ++batch.finiteVertices;
                        batch.vertices.push_back(s);
                    }
                }
                if (retainBatch) {
                    r.payloadVertices += count;
                    r.drawBatches.push_back(std::move(batch));
                } else {
                    ++r.droppedDrawBatches;
                }
                pos = static_cast<uint32_t>(static_cast<uint64_t>(pos) + total);
                continue;
            }

            if (cid == 0x8) { // Model references another ELAN-RAM stream.
                ++r.commands;
                ++r.models;
                const uint32_t modelOff = w[4] & 0x01FFFFF8u;
                const uint32_t modelSize = w[6];
                // Flycast: cullingReversed = param.cwCulling == 0 (bit 27),
                // openModifierVolume = param.openVolume (bit 28),
                // shadowedVolume = pcw.shadow (bit 7), modelTSP = tsp (w2).
                // The state is active only for the nested stream and is
                // cleared afterwards.
                const NativeElanModelState outer = state.model;
                NativeElanModelState model{};
                model.offset = pos;
                model.pcw = w[0];
                model.param = w[1];
                model.tsp = w[2];
                model.cullingReversed = ((w[1] >> 27u) & 1u) == 0u;
                model.openVolume = ((w[1] >> 28u) & 1u) != 0u;
                model.shadowedVolume = ((w[0] >> 7u) & 1u) != 0u;
                model.valid = true;
                state.model = model;
                r.modelStates.push_back(model);
                if (modelSize && hasRange(ramSize, modelOff, modelSize))
                    decodeStream(ram, ramSize, modelOff, modelSize, depth + 1u,
                                 maxSamples, maxPayloadVertices, visited, state, r);
                state.model = outer;
                pos += 32u;
                continue;
            }

            if (cid == 0x3) {
                ++r.commands;
                ++r.projectionMatrices;
                NativeElanProjectionState projection{};
                projection.offset = pos;
                projection.fx = nativeElanU32ToFloat(w[2]);
                projection.tx = nativeElanU32ToFloat(w[3]);
                projection.fy = nativeElanU32ToFloat(w[4]);
                projection.ty = nativeElanU32ToFloat(w[5]);
                projection.valid = std::isfinite(projection.fx) &&
                    std::isfinite(projection.tx) && std::isfinite(projection.fy) &&
                    std::isfinite(projection.ty);
                if (projection.valid) {
                    state.projection = projection;
                    r.projections.push_back(projection);
                }
                pos += 32u;
                continue;
            }

            if (cid == 0xE) {
                ++r.commands;
                ++r.registerWaits;
                // Flycast resets GMP/instance/light state after an active
                // RegisterWait but retains the projection coefficients.  The
                // resident-link fallback historically treated this as an
                // uninterpreted record, allowing a prior object's matrix to
                // leak into later course geometry.  Keep the correction
                // opt-in until the exact-frame A/B is accepted.
                if (w[1] != 0xFFFFFFFFu && w[3] != 0u) {
                    ++r.activeRegisterWaits;
                    if (resetAtRegisterWaitRequested()) {
                        state.material = {};
                        state.instance = {};
                        state.lightModel = {};
                        state.lights = {};
                        ++r.registerWaitStateResets;
                    }
                }
                pos += 32u;
                continue;
            }

            if (cid == 0xF) { // Resident Link: recurse only for true ELAN-RAM links.
                ++r.commands;
                ++r.links;
                const uint32_t offRaw = w[1];
                const uint32_t linkSize = w[3];
                if ((offRaw & 0xA0000000u) == 0u && linkSize) {
                    const uint32_t linkOff = offRaw & 0x01FFFFF8u;
                    if (hasRange(ramSize, linkOff, linkSize))
                        decodeStream(ram, ramSize, linkOff, linkSize, depth + 1u,
                                     maxSamples, maxPayloadVertices, visited, state, r);
                }
                pos += 32u;
                continue;
            }

            if (cid >= 0) {
                ++r.commands;
                // Known but currently uninterpreted 32-byte commands are not
                // counted as unknown.  IDs 0/1/2/4/E are structurally retained.
                if (!(cid == 0x0 || cid == 0x1 || cid == 0x2 || cid == 0x4))
                    ++r.unknownRecords;
            } else {
                ++r.nonElanRecords;
            }
            pos += 32u;
        }
    }
};
