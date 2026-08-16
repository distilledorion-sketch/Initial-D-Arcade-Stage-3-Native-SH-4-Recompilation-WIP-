#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

struct NativeSqIchVertex {
    uint32_t header = 0;
    float x = 0, y = 0, z = 0;
    float u = 0, v = 0;
    bool hasUv = false;
    bool finite = false;
};

struct NativeSqIchBatch {
    uint32_t sourceAddr = 0;
    uint64_t frame = 0;
    uint32_t pcw = 0;
    uint32_t ispTsp = 0;
    uint32_t tsp0 = 0;
    uint32_t tcw0 = 0;
    uint32_t tsp1 = 0;
    uint32_t tcw1 = 0;
    uint32_t flags = 0;
    uint32_t vertexCount = 0;
    uint32_t vertexSize = 0;
    uint32_t finiteVertices = 0;
    float zSpan = 0;
    std::vector<NativeSqIchVertex> vertices;
};

class NativeSqGeometryObserver {
public:
    void observe(uint32_t sourceAddr, const uint8_t bytes[32], uint64_t frame) {
        if (pending_) {
            if (sourceAddr == nextSource_) {
                appendPayload(bytes);
                nextSource_ += 32u;
                if (payload_.size() >= payloadNeeded_) finish();
                return;
            }
            ++aborted_;
            resetPending();
        }

        uint32_t w[8]{};
        std::memcpy(w, bytes, 32);
        if ((w[0] & 0x08000000u) == 0u || ((w[0] >> 8) & 0xFu) != 7u) return;
        const uint32_t vs = vertexSizeFor(w[6]);
        const uint32_t count = w[7];
        ++headers_;
        if (!vs || !count || count > 4096u) return;
        pending_ = true;
        source_ = sourceAddr;
        frame_ = frame;
        nextSource_ = sourceAddr + 32u;
        pcw_ = w[0];
        ispTsp_ = w[1];
        tsp0_ = w[2];
        tcw0_ = w[3];
        tsp1_ = w[4];
        tcw1_ = w[5];
        flags_ = w[6];
        count_ = count;
        vertexSize_ = vs;
        payloadNeeded_ = static_cast<size_t>(count) * vs;
        payload_.clear();
        payload_.reserve((payloadNeeded_ + 31u) & ~size_t(31u));
    }

    uint64_t headers() const { return headers_; }
    uint64_t complete() const { return complete_; }
    uint64_t aborted() const { return aborted_; }
    uint64_t rollingReplaced() const { return rollingReplaced_; }
    uint64_t largestReplaced() const { return largestReplaced_; }
    uint64_t threeDimensionalComplete() const { return threeDimensionalComplete_; }
    uint64_t threeDimensionalReplaced() const { return threeDimensionalReplaced_; }
    float maximumZSpan() const { return maximumZSpan_; }
    const std::vector<NativeSqIchBatch>& batches() const { return batches_; }

private:
    static uint32_t vertexSizeFor(uint32_t flags) {
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
    static float asFloat(const uint8_t* p) { float f; std::memcpy(&f,p,4); return f; }
    static uint32_t asU32(const uint8_t* p) { uint32_t v; std::memcpy(&v,p,4); return v; }

    void appendPayload(const uint8_t bytes[32]) {
        const size_t need = payloadNeeded_ > payload_.size() ? payloadNeeded_ - payload_.size() : 0u;
        const size_t n = need < 32u ? need : 32u;
        payload_.insert(payload_.end(), bytes, bytes+n);
    }

    void finish() {
        NativeSqIchBatch b{};
        b.sourceAddr = source_;
        b.frame = frame_;
        b.pcw = pcw_;
        b.ispTsp = ispTsp_;
        b.tsp0 = tsp0_;
        b.tcw0 = tcw0_;
        b.tsp1 = tsp1_;
        b.tcw1 = tcw1_;
        b.flags = flags_;
        b.vertexCount = count_;
        b.vertexSize = vertexSize_;
        b.vertices.reserve(count_);
        float minimumZ = std::numeric_limits<float>::infinity();
        float maximumZ = -std::numeric_limits<float>::infinity();
        for (uint32_t i=0;i<count_;++i) {
            const size_t o = static_cast<size_t>(i) * vertexSize_;
            NativeSqIchVertex v{};
            v.header = asU32(payload_.data()+o);
            v.x = asFloat(payload_.data()+o+4);
            v.y = asFloat(payload_.data()+o+8);
            v.z = asFloat(payload_.data()+o+12);
            if (flags_ == 0x00Au && vertexSize_ >= 24u) {
                v.u = asFloat(payload_.data()+o+16);
                v.v = asFloat(payload_.data()+o+20);
                v.hasUv = true;
            }
            v.finite = std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
            if (v.finite) {
                ++b.finiteVertices;
                minimumZ = std::min(minimumZ, v.z);
                maximumZ = std::max(maximumZ, v.z);
            }
            b.vertices.push_back(v);
        }
        if (b.finiteVertices != 0u)
            b.zSpan = std::max(0.0f, maximumZ - minimumZ);
        constexpr float kThreeDimensionalZSpan = 1.0e-4f;
        const bool threeDimensional = b.finiteVertices == b.vertexCount &&
                                      b.zSpan > kThreeDimensionalZSpan;
        if (threeDimensional) {
            ++threeDimensionalComplete_;
            maximumZSpan_ = std::max(maximumZSpan_, b.zSpan);
        }
        const auto& target = targetFrameWindow();
        if (target.enabled) {
            if (b.frame >= target.first && b.frame <= target.last &&
                batches_.size() < 256u)
                batches_.push_back(std::move(b));
        } else if (retainThreeDimensionalEnabled()) {
            if (threeDimensional) {
                if (batches_.size() < 256u) {
                    batches_.push_back(std::move(b));
                } else {
                    size_t smallest = 0u;
                    for (size_t i = 1u; i < batches_.size(); ++i) {
                        if (batches_[i].zSpan < batches_[smallest].zSpan)
                            smallest = i;
                    }
                    if (b.zSpan > batches_[smallest].zSpan) {
                        batches_[smallest] = std::move(b);
                        ++threeDimensionalReplaced_;
                    }
                }
            }
        } else if (batches_.size() < 256u) {
            batches_.push_back(std::move(b));
        } else if (retainLargestEnabled()) {
            // Keep a fixed-size inventory of the most detailed completed
            // meshes across the whole run. Four-vertex display quads dominate
            // the end of long boot traces; prioritizing vertex count prevents
            // them from evicting larger model geometry.
            size_t smallest = 0u;
            for (size_t i = 1u; i < batches_.size(); ++i) {
                if (batches_[i].vertexCount < batches_[smallest].vertexCount)
                    smallest = i;
            }
            if (b.vertexCount > batches_[smallest].vertexCount) {
                batches_[smallest] = std::move(b);
                ++largestReplaced_;
            }
        } else if (rollingCaptureEnabled()) {
            // Diagnostic-only rolling retention.  The observer remains
            // read-only and bounded at 256 batches, but a long native boot can
            // inspect its newest geometry instead of being permanently
            // limited to the first startup/UI submissions.
            batches_[rollingWrite_] = std::move(b);
            rollingWrite_ = (rollingWrite_ + 1u) % batches_.size();
            ++rollingReplaced_;
        }
        ++complete_;
        resetPending();
    }

    static bool rollingCaptureEnabled() {
        static const bool enabled = [] {
            const char* value = std::getenv("IDAS3_NATIVE_SQ_ROLLING");
            return value && *value && std::strcmp(value, "0") != 0;
        }();
        return enabled;
    }

    static bool retainLargestEnabled() {
        static const bool enabled = [] {
            const char* value = std::getenv("IDAS3_NATIVE_SQ_RETAIN_LARGEST");
            return value && *value && std::strcmp(value, "0") != 0;
        }();
        return enabled;
    }

    static bool retainThreeDimensionalEnabled() {
        static const bool enabled = [] {
            const char* value = std::getenv("IDAS3_NATIVE_SQ_RETAIN_3D");
            return value && *value && std::strcmp(value, "0") != 0;
        }();
        return enabled;
    }

    struct TargetFrameWindow {
        bool enabled = false;
        uint64_t first = 0;
        uint64_t last = 0;
    };

    static const TargetFrameWindow& targetFrameWindow() {
        static const TargetFrameWindow window = [] {
            TargetFrameWindow value{};
            const char* first = std::getenv("IDAS3_NATIVE_SQ_FRAME_MIN");
            const char* last = std::getenv("IDAS3_NATIVE_SQ_FRAME_MAX");
            if (!first || !*first || !last || !*last) return value;
            char* firstEnd = nullptr;
            char* lastEnd = nullptr;
            value.first = std::strtoull(first, &firstEnd, 0);
            value.last = std::strtoull(last, &lastEnd, 0);
            value.enabled = firstEnd != first && lastEnd != last &&
                            value.first <= value.last;
            return value;
        }();
        return window;
    }

    void resetPending() {
        pending_ = false;
        source_=nextSource_=pcw_=ispTsp_=tsp0_=tcw0_=tsp1_=tcw1_=0;
        flags_=count_=vertexSize_=0; frame_=0;
        payloadNeeded_=0; payload_.clear();
    }

    bool pending_ = false;
    uint32_t source_ = 0, nextSource_ = 0;
    uint32_t pcw_ = 0, ispTsp_ = 0, tsp0_ = 0, tcw0_ = 0, tsp1_ = 0, tcw1_ = 0;
    uint32_t flags_ = 0, count_ = 0, vertexSize_ = 0;
    uint64_t frame_ = 0;
    size_t payloadNeeded_ = 0;
    std::vector<uint8_t> payload_;
    std::vector<NativeSqIchBatch> batches_;
    size_t rollingWrite_ = 0u;
    uint64_t headers_ = 0, complete_ = 0, aborted_ = 0;
    uint64_t rollingReplaced_ = 0;
    uint64_t largestReplaced_ = 0;
    uint64_t threeDimensionalComplete_ = 0;
    uint64_t threeDimensionalReplaced_ = 0;
    float maximumZSpan_ = 0;
};
