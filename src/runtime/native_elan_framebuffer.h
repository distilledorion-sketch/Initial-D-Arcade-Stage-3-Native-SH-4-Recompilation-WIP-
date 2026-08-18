#pragma once

#include "native_elan_decode.h"
#include "vram_texture.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

// Diagnostic host framebuffer for frame-final ELAN scenes. Fit-to-view remains
// the default for offline diagnostics. The native window and
// IDAS3_NATIVE_FRAMEBUFFER_PROJECTION use the exact NAOMI 2 instance/projection
// transform proven against Flycast's ELAN implementation.
// All triangle input still comes directly from the game's finalized ICH data.

struct NativeElanFramebufferResult {
    bool written = false;
    uint64_t sceneFrame = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t acceptedBatches = 0;
    uint32_t rejectedBatches = 0;
    uint32_t duplicateGeometryBatches = 0;
    uint32_t vertices = 0;
    uint32_t triangles = 0;
    uint32_t projectionMode = 0;
    uint32_t projectedBatches = 0;
    uint32_t identityInstanceBatches = 0;
    uint32_t projectionRejectedBatches = 0;
    uint32_t projectionRejectedTriangles = 0;
    uint32_t projectionVertices = 0;
    uint32_t projectionFiniteEyeVertices = 0;
    uint32_t projectionNearRejectedVertices = 0;
    uint32_t projectionGuardRejectedVertices = 0;
    uint32_t projectionValidVertices = 0;
    uint32_t projectionViewportRejectedTriangles = 0;
    uint32_t projectionNearClippedTriangles = 0;
    uint32_t projectionNearOutsideTriangles = 0;
    uint32_t projectionNearClipFailedTriangles = 0;
    uint32_t projectionNonFiniteTriangles = 0;
    uint32_t nearFarCulledBatches = 0;
    uint32_t diagnosticExcludedBatches = 0;
    uint32_t uvBatches = 0;
    uint32_t texturedPcwBatches = 0;
    uint32_t textureCandidateBatches = 0;
    uint32_t uniqueTextureStates = 0;
    uint32_t vramBackedTextureStates = 0;
    uint32_t decodedTextureStates = 0;
    uint32_t texturedBatches = 0;
    uint32_t unsupportedTextureBatches = 0;
    uint32_t texturedTriangles = 0;
    std::array<uint32_t, 8> sourceBlendBatches{};
    std::array<uint32_t, 8> destinationBlendBatches{};
    uint32_t useAlphaBatches = 0;
    uint32_t offsetColorBatches = 0;
    uint32_t sourceSelectBatches = 0;
    uint32_t destinationSelectBatches = 0;
    uint32_t lightModeledBatches = 0;
    uint32_t gouraudBatches = 0;
    uint32_t flatShadedBatches = 0;
    std::array<uint32_t, 4> fogModeBatches{};
    uint32_t colorClampBatches = 0;
    std::array<uint32_t, 8> listTypeBatches{};
    std::array<uint32_t, 8> listTypeTriangles{};
    std::array<uint64_t, 8> listTypeRasterPixels{};
    uint32_t shadowedBatches = 0;
    uint32_t openModifierVolumeBatches = 0;
    uint32_t modifierVolumeBatches = 0;
    uint32_t modifierVolumeTriangles = 0;
    uint64_t modifierDepthPixels = 0;
    uint64_t modifierFinalizePixels = 0;
    uint64_t modifierShadowPixels = 0;
    uint32_t modifierVolumes = 0;
    uint32_t punchAlphaTest = 0;
    uint64_t punchAlphaTestedPixels = 0;
    uint64_t punchAlphaRejectedPixels = 0;
    uint32_t volumeFlagBatches = 0;
    uint32_t twoVolumeBatches = 0;
    uint32_t twoVolumeVertexColorBatches = 0;
    uint32_t twoVolumeSecondTextureBatches = 0;
    uint32_t translucentAutosort = 0;
    uint32_t autosortedTranslucentTriangles = 0;
    uint64_t uvVertices = 0;
    uint64_t texturedPixels = 0;
    uint64_t litVertices = 0;
    float projectionNearMin = std::numeric_limits<float>::infinity();
    float projectionNearMax = -std::numeric_limits<float>::infinity();
    float projectionEyeZMin = std::numeric_limits<float>::infinity();
    float projectionEyeZMax = -std::numeric_limits<float>::infinity();
    float projectionScreenXMin = std::numeric_limits<float>::infinity();
    float projectionScreenXMax = -std::numeric_limits<float>::infinity();
    float projectionScreenYMin = std::numeric_limits<float>::infinity();
    float projectionScreenYMax = -std::numeric_limits<float>::infinity();
    std::string path;
};

struct NativeElanFramebufferImage {
    NativeElanFramebufferResult result{};
    std::vector<uint8_t> rgb;
};

class NativeElanDiagnosticFramebuffer {
public:
    // The submitted-scene queue contains intermediate ELAN construction
    // passes as well as complete frames. Select the scene with the most
    // prevalidated captured-projection geometry in the bounded recent window,
    // breaking ties toward the newest scene. Raw resident snapshots without
    // projection/instance state must not outrank a drawable frame. Legacy
    // callers that do not populate presentation counts retain the original
    // unique-vertex/raw-batch scoring.
    static size_t selectMostCompleteRecentScene(
            const std::vector<NativeElanFrameScene>& scenes) {
        if (scenes.empty()) return 0u;
        const bool havePresentationCounts = std::any_of(
            scenes.begin(), scenes.end(), [](const NativeElanFrameScene& scene) {
                return scene.presentationVertices != 0u ||
                       scene.presentationBatches != 0u;
            });
        const auto vertexScore = [havePresentationCounts](
                const NativeElanFrameScene& scene) {
            return havePresentationCounts
                ? scene.presentationVertices : scene.uniqueVertices;
        };
        const auto batchScore = [havePresentationCounts](
                const NativeElanFrameScene& scene) {
            return havePresentationCounts
                ? scene.presentationBatches : scene.rawBatches;
        };
        size_t selected = 0u;
        for (size_t i = 1u; i < scenes.size(); ++i) {
            const auto& candidate = scenes[i];
            const auto& best = scenes[selected];
            const uint32_t candidateVertices = vertexScore(candidate);
            const uint32_t bestVertices = vertexScore(best);
            const uint32_t candidateBatches = batchScore(candidate);
            const uint32_t bestBatches = batchScore(best);
            if (candidateVertices > bestVertices ||
                (candidateVertices == bestVertices &&
                 candidateBatches > bestBatches) ||
                (candidateVertices == bestVertices &&
                 candidateBatches == bestBatches &&
                 candidate.frame >= best.frame))
                selected = i;
        }
        return selected;
    }

    // Deterministic regression seam for Flycast's 128x2 linear fog-table
    // lookup. Production rendering calls the same private implementation.
    static float samplePvrFogCoefficientForTest(
            const NativePvrFogState& fog, float depth) {
        return fogCoefficient(fog, depth);
    }

    static std::vector<size_t> pvrTranslucentDepthOrderForTest(
            const std::vector<float>& depths) {
        std::vector<size_t> order;
        order.reserve(depths.size());
        for (size_t i = 0u; i < depths.size(); ++i) order.push_back(i);
        std::stable_sort(order.begin(), order.end(),
            [&](size_t lhs, size_t rhs) {
                return translucentDepthLess(depths[lhs], depths[rhs]);
            });
        return order;
    }

    // Deterministic regression seams for the exact low stencil-bit
    // operations used by the production modifier-volume rasterizer.
    static uint8_t pvrModifierDepthPassForTest(
            uint8_t stencil, bool depthPass, bool useOr) {
        applyModifierDepthPass(stencil, depthPass, useOr);
        return stencil;
    }

    static uint8_t pvrModifierFinalizeForTest(uint8_t stencil, uint8_t mode) {
        finalizeModifierStencil(stencil, mode);
        return stencil;
    }

    static uint8_t pvrModifierShadowScaleForTest(uint8_t color, uint8_t scale) {
        return scaleModifierShadowChannel(color, scale);
    }

    static std::array<uint8_t, 2> pvrModifierShadowPixelForTest(
            uint8_t color, uint8_t stencilValue, uint8_t scale) {
        std::vector<uint8_t> rgb{color, color, color};
        std::vector<uint8_t> stencil{stencilValue};
        applyModifierShadow(rgb, 1u, 1u, stencil, scale);
        return {rgb[0], stencil[0]};
    }

    static bool pvrPunchThroughAlphaPassForTest(
            uint8_t alpha, uint32_t alphaReference) {
        return punchThroughAlphaPass(alpha, alphaReference);
    }

    static std::array<uint8_t, 2> pvrListDepthStateForTest(
            uint32_t pcw, uint32_t ispTsp, uint32_t tsp,
            bool autosortTranslucent) {
        RasterState state = decodeRasterState(pcw, ispTsp, tsp);
        applyPvrListDepthState(state, autosortTranslucent);
        return {state.depthMode, state.depthWrite ? uint8_t{1u} : uint8_t{0u}};
    }

    static NativeElanFramebufferImage renderLatestSceneRgb(
        const std::vector<NativeElanFrameScene>& scenes,
        uint32_t width = 640u, uint32_t height = 480u,
        const std::vector<uint8_t>* naomi2Vram = nullptr,
        size_t sceneIndex = std::numeric_limits<size_t>::max()) {
        NativeElanFramebufferImage image{};
        const auto timingBegin = std::chrono::steady_clock::now();
        auto& result = image.result;
        result.width = width;
        result.height = height;
        const bool useCapturedProjection = capturedProjectionRequested();
        const bool useIdentityInstance = useCapturedProjection && identityInstanceRequested();
        const float identityMinFocal = identityMinimumFocalLength();
        const bool traceLighting = lightingTraceRequested();
        const char* ownerBmpPath = pixelOwnerBmpPathRequested();
        const bool tracePixelOwners = pixelOwnerTraceRequested() || ownerBmpPath != nullptr;
        uint32_t tracedLightingBatches = 0u;
        result.projectionMode = useCapturedProjection ? 1u : 0u;
        if (scenes.empty() || width < 32u || height < 32u ||
            width > 4096u || height > 4096u)
            return image;

        const NativeElanFrameScene& scene = sceneIndex < scenes.size()
            ? scenes[sceneIndex] : scenes.back();
        // Fog A/B disables only fog/clamp evaluation. Other frame-bound PVR
        // registers in this state (punch alpha and shadow scale) remain live.
        NativePvrFogState renderFog = scene.fog;
        if (!pvrFogRequested()) renderFog.valid = false;
        const bool autosortTranslucent = useCapturedProjection &&
            scene.fog.translucentAutosort && pvrAutosortRequested();
        result.translucentAutosort = autosortTranslucent ? 1u : 0u;
        const bool modifierVolumes = useCapturedProjection &&
            pvrModifierVolumesRequested();
        result.modifierVolumes = modifierVolumes ? 1u : 0u;
        result.punchAlphaTest = pvrPunchAlphaRequested() ? 1u : 0u;
        result.sceneFrame = scene.frame;
        std::vector<const NativeElanDrawBatch*> batches;
        // Command-port traversal legitimately revisits shared ELAN streams,
        // which can leave several thousand candidate batches in one frame.
        // The original exact duplicate pass compared every candidate against
        // every accepted batch and became quadratic on those production
        // scenes.  Bucket by an exact-field hash first, then retain the full
        // equality check inside the bucket so collisions cannot change output.
        // The bucket key deliberately samples the geometry instead of hashing
        // every vertex; equal geometry is guaranteed to share the sample, and
        // unequal geometry that collides is rejected by the exact check.
        std::unordered_map<uint64_t, std::vector<const NativeElanDrawBatch*>>
            duplicateBuckets;
        duplicateBuckets.reserve(scene.draws.size());
        for (const auto& draw : scene.draws) {
            const auto& batch = draw.batch;
            if (batch.vertexCount < 3u || batch.vertices.size() != batch.vertexCount ||
                batch.finiteVertices != batch.vertexCount) {
                ++result.rejectedBatches;
                continue;
            }
            const bool identityAllowed = useIdentityInstance &&
                std::fabs(batch.projection.fx) >= identityMinFocal &&
                std::fabs(batch.projection.fy) >= identityMinFocal;
            if (useCapturedProjection &&
                (!batch.projection.valid || (!batch.instance.valid && !identityAllowed))) {
                ++result.rejectedBatches;
                ++result.projectionRejectedBatches;
                continue;
            }
            if (batchDiagnosticallyExcluded(
                    batch.ichOffset, draw.sourceRootOffset)) {
                ++result.diagnosticExcludedBatches;
                continue;
            }
            if (useCapturedProjection && nearFarCullEnabled() &&
                !batchBetweenNearAndFar(batch,
                    projectionInstance(batch, useIdentityInstance))) {
                ++result.rejectedBatches;
                ++result.nearFarCulledBatches;
                continue;
            }
            const uint64_t duplicateKey = deduplicationHash(batch, useCapturedProjection);
            auto& duplicateCandidates = duplicateBuckets[duplicateKey];
            bool duplicate = false;
            for (const NativeElanDrawBatch* prior : duplicateCandidates) {
                if (sameGeometry(*prior, batch) &&
                    (!useCapturedProjection || sameRenderState(*prior, batch))) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                ++result.duplicateGeometryBatches;
                continue;
            }
            duplicateCandidates.push_back(&batch);
            batches.push_back(&batch);
            if (useCapturedProjection && !batch.instance.valid && useIdentityInstance)
                ++result.identityInstanceBatches;
        }
        result.acceptedBatches = static_cast<uint32_t>(batches.size());
        if (batches.empty()) return image;
        const auto timingFiltered = std::chrono::steady_clock::now();

        traceTextureStates(batches, naomi2Vram, result);
        const auto timingInventoried = std::chrono::steady_clock::now();

        // ELAN command submission may interleave TA list types, but the PVR
        // renders them as separate lists. Preserve order within each list and
        // execute the hardware list order so translucent environment passes
        // cannot pre-fill depth ahead of opaque body geometry.
        const auto listOrder = [](const NativeElanDrawBatch* batch) {
            switch ((batch->pcw >> 24u) & 7u) {
                case 0u: return 0u; // opaque
                case 4u: return 1u; // punch-through
                case 1u: return 2u; // opaque modifier volume
                case 2u: return 3u; // translucent
                case 3u: return 4u; // translucent modifier volume
                default: return 5u;
            }
        };
        std::stable_sort(batches.begin(), batches.end(),
            [&](const NativeElanDrawBatch* lhs, const NativeElanDrawBatch* rhs) {
                return listOrder(lhs) < listOrder(rhs);
            });

        if (useCapturedProjection && capturedProjectionTraceRequested()) {
            std::vector<const NativeElanDrawBatch*> uniqueStates;
            for (const NativeElanDrawBatch* batch : batches) {
                const auto& instance = projectionInstance(*batch, useIdentityInstance);
                const auto same = std::find_if(uniqueStates.begin(), uniqueStates.end(),
                    [&](const NativeElanDrawBatch* prior) {
                        const auto& priorInstance =
                            projectionInstance(*prior, useIdentityInstance);
                        return prior->projection.fx == batch->projection.fx &&
                               prior->projection.tx == batch->projection.tx &&
                               prior->projection.fy == batch->projection.fy &&
                               prior->projection.ty == batch->projection.ty &&
                               priorInstance.envMapUOffset == instance.envMapUOffset &&
                               priorInstance.envMapVOffset == instance.envMapVOffset &&
                               priorInstance.nearValue == instance.nearValue &&
                               priorInstance.farValue == instance.farValue &&
                               priorInstance.inverseNear == instance.inverseNear &&
                               priorInstance.normalTransform == instance.normalTransform &&
                               priorInstance.transform == instance.transform;
                    });
                if (same != uniqueStates.end()) continue;
                uniqueStates.push_back(batch);
                if (uniqueStates.size() > 16u) continue;
                const auto& m = instance.transform;
                std::fprintf(stderr,
                    "[NATIVE-ELAN-PROJECTION-STATE] index=%zu ich=%08X instance=%08X "
                    "projection=%08X guest_sequence=%llu guest_outer_pr=%08X "
                    "near=%.9g far=%.9g inv_near=%.9g "
                    "matrix=%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g "
                    "proj=%.9g,%.9g,%.9g,%.9g\n",
                    uniqueStates.size() - 1u, batch->ichOffset, instance.offset,
                    batch->projection.offset,
                    static_cast<unsigned long long>(instance.guestSubmitSequence),
                    instance.guestOuterPr, instance.nearValue,
                    instance.farValue, instance.inverseNear,
                    m[0], m[1], m[2], m[3], m[4], m[5],
                    m[6], m[7], m[8], m[9], m[10], m[11],
                    batch->projection.fx, batch->projection.tx,
                    batch->projection.fy, batch->projection.ty);
            }
            std::fprintf(stderr,
                "[NATIVE-ELAN-PROJECTION-STATES] unique=%zu sampled=%zu batches=%zu\n",
                uniqueStates.size(), std::min<size_t>(uniqueStates.size(), 16u), batches.size());
        }

        traceModelStates(batches);

        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = 1.0f;
        float maxY = 1.0f;
        const float fitTrimPercent = diagnosticFitTrimPercent();
        std::vector<float> fitX;
        std::vector<float> fitY;
        if (!useCapturedProjection) {
            minX = std::numeric_limits<float>::infinity();
            minY = std::numeric_limits<float>::infinity();
            maxX = -std::numeric_limits<float>::infinity();
            maxY = -std::numeric_limits<float>::infinity();
            if (fitTrimPercent > 0.0f) {
                size_t fitVertices = 0u;
                for (const auto* batch : batches) fitVertices += batch->vertices.size();
                fitX.reserve(fitVertices);
                fitY.reserve(fitVertices);
            }
        }
        for (const auto* batch : batches) {
            result.vertices += batch->vertexCount;
            if (!useCapturedProjection) {
                for (const auto& v : batch->vertices) {
                    minX = std::min(minX, v.x);
                    minY = std::min(minY, v.y);
                    maxX = std::max(maxX, v.x);
                    maxY = std::max(maxY, v.y);
                    if (fitTrimPercent > 0.0f) {
                        fitX.push_back(v.x);
                        fitY.push_back(v.y);
                    }
                }
            }
        }
        if (!useCapturedProjection && fitTrimPercent > 0.0f && fitX.size() >= 8u) {
            std::sort(fitX.begin(), fitX.end());
            std::sort(fitY.begin(), fitY.end());
            const size_t trim = std::min<size_t>(
                static_cast<size_t>(static_cast<double>(fitX.size()) *
                                    fitTrimPercent / 100.0),
                (fitX.size() - 2u) / 2u);
            minX = fitX[trim];
            maxX = fitX[fitX.size() - 1u - trim];
            minY = fitY[trim];
            maxY = fitY[fitY.size() - 1u - trim];
            std::fprintf(stderr,
                "[NATIVE-ELAN-FIT] trim_percent=%.3f vertices=%zu bounds=%.9g,%.9g,%.9g,%.9g\n",
                fitTrimPercent, fitX.size(), minX, minY, maxX, maxY);
        }
        const float spanX = std::max(maxX - minX, 1.0e-6f);
        const float spanY = std::max(maxY - minY, 1.0e-6f);
        const float margin = 24.0f;
        const float usableW = std::max(1.0f, static_cast<float>(width) - margin * 2.0f);
        const float usableH = std::max(1.0f, static_cast<float>(height) - margin * 2.0f);
        const float scale = std::min(usableW / spanX, usableH / spanY);
        const float centerX = (minX + maxX) * 0.5f;
        const float centerY = (minY + maxY) * 0.5f;

        std::vector<uint8_t> rgb(static_cast<size_t>(width) * height * 3u, 0u);
        // The PVR blend unit operates on RGBA primary and secondary
        // accumulators even though the diagnostic output is a 24-bit BMP.
        // Preserve their alpha planes so DST_ALPHA/SRC_ALPHA instructions are
        // evaluated from guest state rather than approximated as conventional
        // host alpha blending.
        std::vector<uint8_t> alpha(static_cast<size_t>(width) * height, 0xFFu);
        std::vector<uint8_t> secondaryRgb(static_cast<size_t>(width) * height * 3u, 0u);
        std::vector<uint8_t> secondaryAlpha(static_cast<size_t>(width) * height, 0u);
        std::vector<float> depth;
        if (useCapturedProjection)
            depth.assign(static_cast<size_t>(width) * height,
                         -std::numeric_limits<float>::infinity());
        // Flycast uses bit 7 to mark pixels that accept modifier shadows and
        // low bits 1:0 to accumulate/finalize modifier-volume coverage.
        std::vector<uint8_t> stencil;
        if (modifierVolumes)
            stencil.assign(static_cast<size_t>(width) * height, 0u);
        std::vector<uint32_t> pixelOwners;
        if (tracePixelOwners)
            pixelOwners.assign(static_cast<size_t>(width) * height,
                               std::numeric_limits<uint32_t>::max());
        for (size_t i = 0; i < rgb.size(); i += 3u) {
            rgb[i + 0u] = 42u;
            rgb[i + 1u] = 42u;
            rgb[i + 2u] = 46u;
        }

        const uint8_t palette[][3] = {
            {96u, 154u, 220u}, {212u, 132u, 96u}, {126u, 188u, 122u},
            {186u, 126u, 198u}, {204u, 184u, 100u}
        };
        // Decoded PVR textures are immutable until their backing VRAM bytes
        // change.  Re-decoding up to 155 large textures for every preview
        // frame made the Windows heap retain several gigabytes in seconds.
        // Keep a bounded per-render-thread cache and validate each texture by
        // its exact VRAM hash once per frame before reuse.
        static thread_local std::vector<TextureCacheEntry> textureCache;
        static thread_local uint64_t textureCacheGeneration = 0u;
        ++textureCacheGeneration;
        if (textureCache.capacity() < 256u) textureCache.reserve(256u);
        const auto textureFingerprint = [&](uint32_t tsp, uint32_t tcw) {
            const uint32_t width = 8u << ((tsp >> 3u) & 7u);
            const uint32_t height = 8u << (tsp & 7u);
            const uint32_t address = (tcw & 0x001FFFFFu) << 3u;
            const bool stride = (tcw & (1u << 25u)) != 0u;
            const uint32_t pixelFormat = (tcw >> 27u) & 7u;
            const bool vq = (tcw & (1u << 30u)) != 0u;
            const bool mipmapped = (tcw & (1u << 31u)) != 0u;
            uint32_t bytes = 0u;
            uint32_t hash = 0u;
            if (!stride && !vq && !mipmapped && pixelFormat <= 2u) {
                bytes = textureStorageBytes(
                    width, height, pixelFormat, false, false);
                uint32_t nonzero = 0u;
                hash = hashVramRange(naomi2Vram, address, bytes, nonzero);
                if (!naomi2Vram || bytes == 0u ||
                    address > naomi2Vram->size() ||
                    bytes > naomi2Vram->size() - address) {
                    bytes = 0u;
                    hash = 0u;
                }
            }
            return std::pair<uint32_t, uint32_t>{bytes, hash};
        };
        struct DeferredTriangle {
            Point a{};
            Point b{};
            Point c{};
            RasterState rasterState{};
            float sortDepth = 0.0f;
            uint32_t batchIndex = 0u;
            size_t textureCacheIndex = std::numeric_limits<size_t>::max();
        };
        struct ModifierTriangle {
            Point a{};
            Point b{};
            Point c{};
        };
        std::vector<DeferredTriangle> translucentTriangles;
        std::vector<ModifierTriangle> modifierGroup;
        const auto timingRasterBegin = std::chrono::steady_clock::now();
        bool opaqueModifierPassFinished = false;
        const auto finishOpaqueModifierPass = [&]() {
            if (opaqueModifierPassFinished) return;
            if (modifierVolumes) {
                result.modifierShadowPixels += applyModifierShadow(
                    rgb, width, height, stencil,
                    static_cast<uint8_t>(scene.fog.shadeScale & 0xFFu));
            }
            modifierGroup.clear();
            opaqueModifierPassFinished = true;
        };
        const auto flushTranslucent = [&]() {
            if (translucentTriangles.empty()) return;
            // Flycast sortTriangles uses the minimum Naomi 2 projected Z for
            // each primitive and a stable ascending sort. PVR depth is 1/-Z,
            // so this draws farther triangles before nearer triangles while
            // retaining submission order for equal depths.
            std::stable_sort(translucentTriangles.begin(), translucentTriangles.end(),
                [](const DeferredTriangle& lhs, const DeferredTriangle& rhs) {
                    return translucentDepthLess(lhs.sortDepth, rhs.sortDepth);
                });
            for (const DeferredTriangle& triangle : translucentTriangles) {
                const TextureBinding* triangleTexture =
                    triangle.textureCacheIndex < textureCache.size()
                        ? &textureCache[triangle.textureCacheIndex].binding : nullptr;
                const uint64_t pixels = fillTriangle(
                    rgb, alpha, secondaryRgb, secondaryAlpha, width, height,
                    triangle.a, triangle.b, triangle.c, &depth,
                    triangleTexture,
                    triangle.rasterState, renderFog,
                    modifierVolumes ? &stencil : nullptr,
                    &result.punchAlphaTestedPixels,
                    &result.punchAlphaRejectedPixels,
                    tracePixelOwners ? &pixelOwners : nullptr,
                    triangle.batchIndex);
                result.texturedPixels += pixels;
                result.listTypeRasterPixels[2] += pixels;
                if (triangle.textureCacheIndex < textureCache.size())
                    textureCache[triangle.textureCacheIndex].rasterPixels += pixels;
            }
            translucentTriangles.clear();
        };
        for (size_t bi = 0; bi < batches.size(); ++bi) {
            const auto& batch = *batches[bi];
            const auto& vertices = batch.vertices;
            const uint32_t listType = (batch.pcw >> 24u) & 7u;
            if (listType != 2u) flushTranslucent();
            // The classic Flycast path applies opaque modifier shadows before
            // any translucent color pass. Stable list sorting above makes
            // this the single transition out of the modifier stage.
            if (listOrder(&batch) > 2u) finishOpaqueModifierPass();
            ++result.listTypeBatches[listType];
            const uint32_t submittedPcw = effectivePcw(batch);
            const uint32_t effectiveTsp0 = effectiveTsp(batch, batch.tsp0, false);
            if ((submittedPcw & (1u << 7u)) != 0u)
                ++result.shadowedBatches;
            if ((submittedPcw & (1u << 6u)) != 0u)
                ++result.volumeFlagBatches;
            const uint32_t effectiveTsp1 = effectiveTsp(batch, batch.tsp1, true);
            const bool twoVolume = listType != 2u && listType != 3u &&
                (submittedPcw & (3u << 6u)) == (3u << 6u) &&
                effectiveTsp1 != 0xFFFFFFFFu;
            if (twoVolume) {
                ++result.twoVolumeBatches;
                if (batch.flags == 0x042u || batch.flags == 0x04Au)
                    ++result.twoVolumeVertexColorBatches;
                if ((submittedPcw & (1u << 3u)) != 0u &&
                    (effectiveTsp1 != effectiveTsp0 || batch.tcw1 != batch.tcw0))
                    ++result.twoVolumeSecondTextureBatches;
            }
            if (modelStateEnabled() && batch.model.valid && batch.model.openVolume)
                ++result.openModifierVolumeBatches;
            if (listType == 1u) ++result.modifierVolumeBatches;
            const auto& renderInstance = projectionInstance(batch, useIdentityInstance);
            // ELAN Model command state, applied exactly as Flycast's
            // setStateParams does: the model TSP is XORed into the polygon
            // TSP and the ISP cull mode is XORed with cullingReversed and the
            // left-handed projection flip.
            RasterState rasterState = decodeRasterState(
                submittedPcw, effectiveIspTsp(batch), effectiveTsp0);
            applyPvrListDepthState(rasterState, autosortTranslucent);
            rasterState.tileClip = batch.tileClip;
            if (listType == 1u || listType == 3u) {
                // Flycast sendMVPolygon uses the raw modifier ISP. Closed
                // volumes force CullMode=0; open volumes retain the list ISP.
                const bool openVolume = modelStateEnabled() && batch.model.valid &&
                    batch.model.openVolume;
                rasterState.cullMode = openVolume
                    ? static_cast<uint8_t>((batch.ispTsp >> 27u) & 3u) : 0u;
                rasterState.modifierMode =
                    static_cast<uint8_t>((batch.ispTsp >> 29u) & 3u);
                rasterState.volumeLast = (batch.pcw & (1u << 6u)) != 0u;
                rasterState.depthWrite = false;
            }
            ++result.sourceBlendBatches[rasterState.srcBlend & 7u];
            ++result.destinationBlendBatches[rasterState.dstBlend & 7u];
            result.useAlphaBatches += rasterState.useAlpha ? 1u : 0u;
            result.offsetColorBatches += rasterState.offset ? 1u : 0u;
            result.sourceSelectBatches += rasterState.srcSelect ? 1u : 0u;
            result.destinationSelectBatches += rasterState.dstSelect ? 1u : 0u;
            if (rasterState.gouraud)
                ++result.gouraudBatches;
            else
                ++result.flatShadedBatches;
            ++result.fogModeBatches[(effectiveTsp0 >> 22u) & 3u];
            if ((effectiveTsp0 & (1u << 21u)) != 0u)
                ++result.colorClampBatches;
            const uint64_t pixelsBeforeBatch = result.texturedPixels;
            const uint8_t* fallback = palette[bi % (sizeof(palette) / sizeof(palette[0]))];
            if (batch.lightModel.valid) ++result.lightModeledBatches;
            const bool environmentMapped = batch.material.valid &&
                (batch.material.words[2] & (1u << 11u)) != 0u && renderInstance.valid;
            const bool batchHasUv = std::any_of(vertices.begin(), vertices.end(),
                [](const NativeElanVertexSample& vertex) { return vertex.hasUv; }) ||
                environmentMapped;
            const bool batchTextured = (submittedPcw & (1u << 3u)) != 0u && batchHasUv;
            const TextureBinding* textureBinding = nullptr;
            TextureCacheEntry* textureEntry = nullptr;
            size_t textureCacheIndex = std::numeric_limits<size_t>::max();
            if (batchTextured) {
                auto cached = std::find_if(textureCache.begin(), textureCache.end(),
                    [&](const TextureCacheEntry& entry) {
                        return entry.tsp == effectiveTsp0 && entry.tcw == batch.tcw0;
                    });
                if (cached == textureCache.end()) {
                    TextureCacheEntry entry{};
                    entry.tsp = effectiveTsp0;
                    entry.tcw = batch.tcw0;
                    const auto fingerprint = textureFingerprint(
                        effectiveTsp0, batch.tcw0);
                    entry.vramBytes = fingerprint.first;
                    entry.vramHash = fingerprint.second;
                    entry.binding = decodeTextureBinding(
                        effectiveTsp0, batch.tcw0, naomi2Vram);
                    entry.validationGeneration = textureCacheGeneration;
                    textureCache.push_back(std::move(entry));
                    cached = textureCache.end() - 1;
                } else if (cached->validationGeneration != textureCacheGeneration) {
                    cached->firstIch = 0u;
                    cached->batches = 0u;
                    cached->vertices = 0u;
                    cached->rasterPixels = 0u;
                    const auto fingerprint = textureFingerprint(
                        effectiveTsp0, batch.tcw0);
                    if (cached->vramBytes != fingerprint.first ||
                        cached->vramHash != fingerprint.second) {
                        cached->binding = decodeTextureBinding(
                            effectiveTsp0, batch.tcw0, naomi2Vram);
                        cached->vramBytes = fingerprint.first;
                        cached->vramHash = fingerprint.second;
                    }
                    cached->validationGeneration = textureCacheGeneration;
                }
                if (cached->batches == 0u && cached->binding.valid)
                    ++result.decodedTextureStates;
                textureEntry = &*cached;
                textureCacheIndex = static_cast<size_t>(
                    std::distance(textureCache.begin(), cached));
                if (textureEntry->firstIch == 0u)
                    textureEntry->firstIch = batch.ichOffset;
                ++textureEntry->batches;
                textureEntry->vertices += batch.vertexCount;
                if (cached->binding.valid) {
                    textureBinding = &cached->binding;
                    ++result.texturedBatches;
                } else {
                    ++result.unsupportedTextureBatches;
                }
            }
            std::vector<Point> points;
            points.reserve(vertices.size());
            for (const auto& v : vertices) {
                const VertexColors colors = shadeVertexColors(batch, v, fallback);
                result.litVertices += colors.lit ? 1u : 0u;
                if (useCapturedProjection) {
                    Point point = projectVertex(v, renderInstance, batch.projection, result);
                    if (environmentMapped)
                        applyEnvironmentMap(renderInstance, v, point.u, point.v, point.hasUv);
                    point.argb = colors.base;
                    point.offsetArgb = colors.offset;
                    points.push_back(point);
                } else {
                    Point point{};
                    point.x = static_cast<float>(width) * 0.5f + (v.x - centerX) * scale;
                    point.y = static_cast<float>(height) * 0.5f - (v.y - centerY) * scale;
                    point.u = v.u;
                    point.v = v.v;
                    point.argb = colors.base;
                    point.offsetArgb = colors.offset;
                    point.hasUv = v.hasUv;
                    if (environmentMapped)
                        applyEnvironmentMap(renderInstance, v, point.u, point.v, point.hasUv);
                    point.valid = true;
                    points.push_back(point);
                }
            }
            if (useCapturedProjection) ++result.projectedBatches;
            dumpBatchGeometry(bi, batch, points, rasterState);
            uint32_t batchTriangles = 0u;
            uint32_t batchCulled = 0u;
            uint32_t batchDrawn = 0u;
            const uint32_t trianglesBeforeBatch = result.triangles;
            std::vector<ModifierTriangle> batchModifierTriangles;
            const auto pvrProjectedDepth = [](const Point& point) {
                if (!std::isfinite(point.eyeZ) || point.eyeZ == 0.0f)
                    return -std::numeric_limits<float>::infinity();
                return -1.0f / point.eyeZ;
            };
            const auto rasterOrDefer = [&](const Point& a, const Point& b,
                                           const Point& c,
                                           const TextureBinding* triangleTexture,
                                           float sortDepth) {
                if (autosortTranslucent && listType == 2u) {
                    DeferredTriangle triangle{};
                    triangle.a = a;
                    triangle.b = b;
                    triangle.c = c;
                    triangle.rasterState = rasterState;
                    triangle.sortDepth = sortDepth;
                    triangle.batchIndex = static_cast<uint32_t>(bi);
                    triangle.textureCacheIndex = triangleTexture
                        ? textureCacheIndex : std::numeric_limits<size_t>::max();
                    translucentTriangles.push_back(std::move(triangle));
                    ++result.autosortedTranslucentTriangles;
                    return uint32_t{0u};
                }
                if (useCapturedProjection && listType == 1u) {
                    batchModifierTriangles.push_back({a, b, c});
                    if (modifierVolumes) {
                        const bool useOr = !rasterState.volumeLast &&
                            rasterState.modifierMode > 0u;
                        result.modifierDepthPixels += rasterizeModifierDepth(
                            width, height, a, b, c, depth, stencil,
                            rasterState, useOr);
                    }
                    return uint32_t{0u};
                }
                return fillTriangle(
                    rgb, alpha, secondaryRgb, secondaryAlpha,
                    width, height, a, b, c,
                    useCapturedProjection ? &depth : nullptr,
                    triangleTexture, rasterState, renderFog,
                    modifierVolumes ? &stencil : nullptr,
                    &result.punchAlphaTestedPixels,
                    &result.punchAlphaRejectedPixels,
                    tracePixelOwners ? &pixelOwners : nullptr,
                    static_cast<uint32_t>(bi));
            };
            const auto renderTriangle = [&](size_t a, size_t b, size_t c) {
                ++batchTriangles;
                const float sortDepth = std::min({
                    pvrProjectedDepth(points[a]), pvrProjectedDepth(points[b]),
                    pvrProjectedDepth(points[c])});
                if (useCapturedProjection &&
                    (!points[a].valid || !points[b].valid || !points[c].valid)) {
                    if (!points[a].finiteEye || !points[b].finiteEye ||
                        !points[c].finiteEye) {
                        ++result.projectionNonFiniteTriangles;
                        ++result.projectionRejectedTriangles;
                        return;
                    }
                    auto clipped = clipNearTriangle(
                        points[a], points[b], points[c], renderInstance, batch.projection);
                    if (clipped.size() >= 3u) {
                        // Dreamcast/NAOMI uses the last vertex as the provoking
                        // vertex. Clipping creates new vertices, but flat color
                        // still belongs to the original primitive's last input.
                        if (!rasterState.gouraud) {
                            for (Point& point : clipped) {
                                point.argb = points[c].argb;
                                point.offsetArgb = points[c].offsetArgb;
                            }
                        }
                        ++result.projectionNearClippedTriangles;
                        for (size_t ci = 1u; ci + 1u < clipped.size(); ++ci) {
                            if (!intersectsViewport(clipped[0], clipped[ci], clipped[ci + 1u],
                                                    width, height)) {
                                ++result.projectionRejectedTriangles;
                                ++result.projectionViewportRejectedTriangles;
                                continue;
                            }
                            const TextureBinding* triangleTexture =
                                textureBinding && clipped[0].hasUv && clipped[ci].hasUv &&
                                clipped[ci + 1u].hasUv ? textureBinding : nullptr;
                            if (triangleTexture) ++result.texturedTriangles;
                            result.texturedPixels += rasterOrDefer(
                                clipped[0], clipped[ci], clipped[ci + 1u],
                                triangleTexture, sortDepth);
                            ++result.triangles;
                        }
                        return;
                    }
                    const float nearZ =
                        -std::max(renderInstance.nearValue, 1.0e-6f);
                    if (points[a].eyeZ > nearZ && points[b].eyeZ > nearZ &&
                        points[c].eyeZ > nearZ)
                        ++result.projectionNearOutsideTriangles;
                    else
                        ++result.projectionNearClipFailedTriangles;
                    ++result.projectionRejectedTriangles;
                    return;
                }
                if (useCapturedProjection &&
                    !intersectsViewport(points[a], points[b], points[c], width, height)) {
                    ++result.projectionRejectedTriangles;
                    ++result.projectionViewportRejectedTriangles;
                    return;
                }
                const TextureBinding* triangleTexture =
                    textureBinding && points[a].hasUv && points[b].hasUv && points[c].hasUv
                        ? textureBinding : nullptr;
                if (triangleTexture) ++result.texturedTriangles;
                if (triangleCulled(points[a], points[b], points[c], rasterState))
                    ++batchCulled;
                else
                    ++batchDrawn;
                result.texturedPixels += rasterOrDefer(
                    points[a], points[b], points[c], triangleTexture, sortDepth);
                if (!useCapturedProjection && diagnosticOutlinesEnabled()) {
                    const uint8_t outline[3] = {235u, 235u, 238u};
                    drawLine(rgb, width, height, points[a], points[b], outline);
                    drawLine(rgb, width, height, points[b], points[c], outline);
                    drawLine(rgb, width, height, points[c], points[a], outline);
                }
                ++result.triangles;
            };

            // ICH payloads contain multiple strips and triangle fans in one
            // vertex array. Header bits 29/30 select strip/fan semantics and
            // bit 31 ends the current primitive. Treating the whole batch as
            // one strip creates large false triangles between independent
            // polygons. This mirrors Flycast's sendVertices topology while
            // omitting only its deliberately degenerate connector vertices.
            bool stripStart = true;
            size_t fanCenter = 0u;
            size_t previous = 0u;
            size_t previous2 = 0u;
            uint32_t stripVertices = 0u;
            for (size_t i = 0; i < points.size(); ++i) {
                const uint32_t header = vertices[i].header;
                const bool isFan = ((header >> 29u) & 3u) == 2u;
                if (stripStart) {
                    fanCenter = i;
                    previous = i;
                    previous2 = i;
                    stripVertices = 1u;
                    stripStart = false;
                } else if (stripVertices == 1u) {
                    previous = i;
                    stripVertices = 2u;
                } else {
                    if (isFan)
                        renderTriangle(fanCenter, previous, i);
                    else if ((stripVertices & 1u) == 0u)
                        renderTriangle(previous2, previous, i);
                    else
                        // Triangle strips alternate their first two vertices
                        // so every generated primitive keeps the same front
                        // winding. The host rasterizer receives independent
                        // triangles, so reproduce that hardware parity here.
                        renderTriangle(previous, previous2, i);
                    previous2 = previous;
                    previous = i;
                    ++stripVertices;
                }
                if ((header & 0x80000000u) != 0u)
                    stripStart = true;
            }
            if (useCapturedProjection && listType == 1u) {
                result.modifierVolumeTriangles +=
                    static_cast<uint32_t>(batchModifierTriangles.size());
                if (modifierVolumes)
                    modifierGroup.insert(modifierGroup.end(),
                        batchModifierTriangles.begin(), batchModifierTriangles.end());
                if (modifierVolumes &&
                    (rasterState.modifierMode == 1u ||
                     rasterState.modifierMode == 2u)) {
                    for (const ModifierTriangle& triangle : modifierGroup) {
                        result.modifierFinalizePixels += rasterizeModifierFinalize(
                            width, height, triangle.a, triangle.b, triangle.c,
                            stencil, rasterState, rasterState.modifierMode);
                    }
                    modifierGroup.clear();
                }
            }
            const uint64_t batchPixels = result.texturedPixels - pixelsBeforeBatch;
            result.listTypeTriangles[listType] +=
                result.triangles - trianglesBeforeBatch;
            result.listTypeRasterPixels[listType] += batchPixels;
            if (textureEntry) textureEntry->rasterPixels += batchPixels;
            if (cullTraceRequested())
                std::fprintf(stderr,
                    "[NATIVE-ELAN-CULL] batch=%zu ich=%08X list=%u pcw=%08X isp=%08X "
                    "tsp=%08X tcw=%08X textured=%u material=%08X params=%08X "
                    "tileclip=%08X model=%u,%08X cull=%u depth_mode=%u depth_write=%u triangles=%u "
                    "drawn=%u culled=%u pixels=%llu\n",
                    bi, batch.ichOffset,
                    static_cast<unsigned>((batch.pcw >> 24u) & 7u),
                    batch.pcw, batch.ispTsp, effectiveTsp0, batch.tcw0,
                    textureBinding ? 1u : 0u, batch.material.offset,
                    batch.material.valid ? batch.material.words[2] : 0u,
                    batch.tileClip,
                    batch.model.valid ? 1u : 0u, batch.model.pcw,
                    static_cast<unsigned>(rasterState.cullMode),
                    static_cast<unsigned>(rasterState.depthMode),
                    rasterState.depthWrite ? 1u : 0u,
                    batchTriangles, batchDrawn, batchCulled,
                    static_cast<unsigned long long>(batchPixels));
            if (traceLighting && batchPixels != 0u && tracedLightingBatches < 192u) {
                const VertexColors firstColors = shadeVertexColors(
                    batch, vertices.front(), fallback);
                const auto& first = vertices.front();
                std::fprintf(stderr,
                    "[NATIVE-ELAN-LIGHT-BATCH] bi=%zu ich=%08X pixels=%llu pcw=%08X isp=%08X "
                    "tsp=%08X tcw=%08X texenv=%u material=%08X params=%08X "
                    "diffuse=%08X offset=%08X light_model=%08X flags=%08X masks=%08X "
                    "lights=%zu first_normal=%.6g,%.6g,%.6g first_in=%08X,%08X "
                    "first_out=%08X,%08X lit=%u\n",
                    bi, batch.ichOffset,
                    static_cast<unsigned long long>(batchPixels), batch.pcw, batch.ispTsp,
                    batch.tsp0, batch.tcw0,
                    static_cast<unsigned>((batch.tsp0 >> 6u) & 3u),
                    batch.material.offset,
                    batch.material.valid ? batch.material.words[2] : 0u,
                    batch.material.valid ? batch.material.words[3] : 0u,
                    batch.material.valid ? batch.material.words[4] : 0u,
                    batch.lightModel.offset,
                    batch.lightModel.valid ? batch.lightModel.words[1] : 0u,
                    batch.lightModel.valid ? batch.lightModel.words[2] : 0u,
                    batch.lights.size(), first.nx, first.ny, first.nz,
                    first.baseArgb, first.offsetArgb,
                    firstColors.base, firstColors.offset, firstColors.lit ? 1u : 0u);
                for (const NativeElanLightState& light : batch.lights) {
                    std::fprintf(stderr,
                        "[NATIVE-ELAN-LIGHT-RECORD] bi=%zu ich=%08X id=%u offset=%08X "
                        "parallel=%u words=%08X,%08X,%08X,%08X,%08X,%08X,%08X,%08X\n",
                        bi, batch.ichOffset, light.lightId, light.offset,
                        light.parallel ? 1u : 0u,
                        light.words[0], light.words[1], light.words[2], light.words[3],
                        light.words[4], light.words[5], light.words[6], light.words[7]);
                }
                ++tracedLightingBatches;
            }
        }
        finishOpaqueModifierPass();
        flushTranslucent();
        const auto timingRasterDone = std::chrono::steady_clock::now();

        if (const char* atlasPath = textureAtlasPathRequested();
            atlasPath && *atlasPath) {
            const bool written = writeTextureAtlas(atlasPath, textureCache);
            std::fprintf(stderr,
                "[NATIVE-ELAN-TEXTURE-ATLAS] written=%u path='%s' entries=%zu\n",
                written ? 1u : 0u, atlasPath, textureCache.size());
            for (size_t i = 0; i < textureCache.size(); ++i) {
                const TextureCacheEntry& entry = textureCache[i];
                const uint32_t width = 8u << ((entry.tsp >> 3u) & 7u);
                const uint32_t height = 8u << (entry.tsp & 7u);
                const uint32_t address = (entry.tcw & 0x001FFFFFu) << 3u;
                const uint32_t pixelFormat = (entry.tcw >> 27u) & 7u;
                const bool vq = (entry.tcw & (1u << 30u)) != 0u;
                const bool mipmapped = (entry.tcw & (1u << 31u)) != 0u;
                const uint32_t bytes = textureStorageBytes(
                    width, height, pixelFormat, vq, mipmapped);
                uint32_t nonzero = 0u;
                const uint32_t hash = hashVramRange(
                    naomi2Vram, address, bytes, nonzero);
                std::fprintf(stderr,
                    "[NATIVE-ELAN-TEXTURE-ATLAS-ENTRY] index=%zu tsp=%08X tcw=%08X "
                    "address=%08X size=%ux%u bytes=%u format=%u valid=%u "
                    "nonzero=%u hash=%08X first_ich=%08X batches=%llu "
                    "vertices=%llu raster_pixels=%llu\n",
                    i, entry.tsp, entry.tcw, address, width, height, bytes,
                    pixelFormat, entry.binding.valid ? 1u : 0u, nonzero, hash,
                    entry.firstIch,
                    static_cast<unsigned long long>(entry.batches),
                    static_cast<unsigned long long>(entry.vertices),
                    static_cast<unsigned long long>(entry.rasterPixels));
            }
        }

        if (tracePixelOwners) {
            struct OwnerSummary {
                uint32_t batch = 0u;
                uint64_t pixels = 0u;
                uint64_t red = 0u;
                uint64_t green = 0u;
                uint64_t blue = 0u;
                uint32_t minX = std::numeric_limits<uint32_t>::max();
                uint32_t minY = std::numeric_limits<uint32_t>::max();
                uint32_t maxX = 0u;
                uint32_t maxY = 0u;
                float minDepth = std::numeric_limits<float>::infinity();
                float maxDepth = -std::numeric_limits<float>::infinity();
                double depthSum = 0.0;
            };
            std::vector<OwnerSummary> owners(batches.size());
            for (uint32_t i = 0u; i < owners.size(); ++i) owners[i].batch = i;
            uint64_t ownedPixels = 0u;
            for (size_t pixel = 0u; pixel < pixelOwners.size(); ++pixel) {
                const uint32_t owner = pixelOwners[pixel];
                if (owner >= owners.size()) continue;
                OwnerSummary& summary = owners[owner];
                const size_t colorOffset = pixel * 3u;
                ++summary.pixels;
                ++ownedPixels;
                summary.red += rgb[colorOffset + 0u];
                summary.green += rgb[colorOffset + 1u];
                summary.blue += rgb[colorOffset + 2u];
                const uint32_t x = static_cast<uint32_t>(pixel % width);
                const uint32_t y = static_cast<uint32_t>(pixel / width);
                summary.minX = std::min(summary.minX, x);
                summary.minY = std::min(summary.minY, y);
                summary.maxX = std::max(summary.maxX, x);
                summary.maxY = std::max(summary.maxY, y);
                if (pixel < depth.size() && std::isfinite(depth[pixel])) {
                    summary.minDepth = std::min(summary.minDepth, depth[pixel]);
                    summary.maxDepth = std::max(summary.maxDepth, depth[pixel]);
                    summary.depthSum += static_cast<double>(depth[pixel]);
                }
            }

            // False-color owner image. Every pixel is painted with the
            // deterministic color of the batch that last changed it, so broad
            // constant-color batches become identifiable surfaces instead of
            // an undifferentiated black region. Read-only diagnostic output;
            // it never feeds the rendered frame.
            if (ownerBmpPath) {
                std::vector<uint8_t> ownerRgb(
                    static_cast<size_t>(width) * height * 3u, 0u);
                for (size_t pixel = 0u; pixel < pixelOwners.size(); ++pixel) {
                    const size_t colorOffset = pixel * 3u;
                    const uint32_t owner = pixelOwners[pixel];
                    if (owner >= batches.size()) {
                        ownerRgb[colorOffset + 0u] = 16u;
                        ownerRgb[colorOffset + 1u] = 16u;
                        ownerRgb[colorOffset + 2u] = 18u;
                        continue;
                    }
                    uint8_t color[3];
                    falseColorForBatch(owner, color);
                    ownerRgb[colorOffset + 0u] = color[0];
                    ownerRgb[colorOffset + 1u] = color[1];
                    ownerRgb[colorOffset + 2u] = color[2];
                }
                const bool ownerWritten =
                    writeBmp(ownerBmpPath, width, height, ownerRgb);
                std::fprintf(stderr,
                    "[NATIVE-ELAN-PIXEL-OWNER-BMP] written=%u path='%s' "
                    "size=%ux%u owned_pixels=%llu total_pixels=%llu\n",
                    ownerWritten ? 1u : 0u, ownerBmpPath, width, height,
                    static_cast<unsigned long long>(ownedPixels),
                    static_cast<unsigned long long>(
                        static_cast<uint64_t>(width) * height));
            }
            std::stable_sort(owners.begin(), owners.end(),
                [](const OwnerSummary& lhs, const OwnerSummary& rhs) {
                    return lhs.pixels > rhs.pixels;
                });
            uint32_t emitted = 0u;
            for (const OwnerSummary& owner : owners) {
                if (owner.pixels == 0u || emitted >= 64u) break;
                const NativeElanDrawBatch& batch = *batches[owner.batch];
                const auto source = std::find_if(scene.draws.begin(), scene.draws.end(),
                    [&](const NativeElanSceneDraw& draw) {
                        return &draw.batch == &batch;
                    });
                const uint32_t sourceRecord = source != scene.draws.end()
                    ? source->sourceLinkRecord : 0u;
                const uint32_t sourceRoot = source != scene.draws.end()
                    ? source->sourceRootOffset : 0u;
                const uint32_t params = batch.material.valid ? batch.material.words[2] : 0u;
                uint8_t color[3];
                falseColorForBatch(owner.batch, color);
                std::fprintf(stderr,
                    "[NATIVE-ELAN-PIXEL-OWNER] rank=%u batch=%u pixels=%llu "
                    "avg=%u,%u,%u bbox=%u,%u..%u,%u ich=%08X list=%u pcw=%08X isp=%08X tsp=%08X "
                    "tcw=%08X params=%08X vertices=%u source_record=%08X source_root=%08X "
                    "instance=%08X projection=%08X model=%08X guest_sequence=%llu "
                    "guest_outer_pr=%08X translation=%.9g,%.9g,%.9g "
                    "depth=%.9g..%.9g mean_depth=%.9g eye_z=%.9g..%.9g "
                    "false_color=%u,%u,%u\n",
                    emitted, owner.batch,
                    static_cast<unsigned long long>(owner.pixels),
                    static_cast<unsigned>(owner.red / owner.pixels),
                    static_cast<unsigned>(owner.green / owner.pixels),
                    static_cast<unsigned>(owner.blue / owner.pixels),
                    owner.minX, owner.minY, owner.maxX, owner.maxY,
                    batch.ichOffset, static_cast<unsigned>((batch.pcw >> 24u) & 7u),
                    batch.pcw, batch.ispTsp, batch.tsp0, batch.tcw0, params,
                    batch.vertexCount, sourceRecord, sourceRoot,
                    batch.instance.offset, batch.projection.offset, batch.model.offset,
                    static_cast<unsigned long long>(batch.instance.guestSubmitSequence),
                    batch.instance.guestOuterPr,
                    batch.instance.transform[9], batch.instance.transform[10],
                    batch.instance.transform[11],
                    owner.minDepth, owner.maxDepth,
                    owner.pixels != 0u
                        ? owner.depthSum / static_cast<double>(owner.pixels) : 0.0,
                    batchEyeZMin(batch), batchEyeZMax(batch),
                    static_cast<unsigned>(color[0]),
                    static_cast<unsigned>(color[1]),
                    static_cast<unsigned>(color[2]));
                ++emitted;
            }
        }

        image.rgb = std::move(rgb);
        if (const char* timing = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_TIMING_TRACE");
            timing && *timing && std::strcmp(timing, "0") != 0) {
            const auto milliseconds = [](auto begin, auto end) {
                return std::chrono::duration<double, std::milli>(end - begin).count();
            };
            const auto timingDone = std::chrono::steady_clock::now();
            std::fprintf(stderr,
                "[NATIVE-ELAN-FRAMEBUFFER-TIMING] frame=%llu batches=%u vertices=%u "
                "triangles=%u filter_ms=%.3f inventory_ms=%.3f setup_ms=%.3f "
                "raster_ms=%.3f finalize_ms=%.3f total_ms=%.3f\n",
                static_cast<unsigned long long>(result.sceneFrame),
                result.acceptedBatches, result.vertices, result.triangles,
                milliseconds(timingBegin, timingFiltered),
                milliseconds(timingFiltered, timingInventoried),
                milliseconds(timingInventoried, timingRasterBegin),
                milliseconds(timingRasterBegin, timingRasterDone),
                milliseconds(timingRasterDone, timingDone),
                milliseconds(timingBegin, timingDone));
        }
        return image;
    }

    static NativeElanFramebufferResult writeLatestSceneBmp(
        const std::vector<NativeElanFrameScene>& scenes, const std::string& path,
        uint32_t width = 640u, uint32_t height = 480u,
        const std::vector<uint8_t>* naomi2Vram = nullptr) {
        NativeElanFramebufferImage image = renderLatestSceneRgb(
            scenes, width, height, naomi2Vram);
        image.result.path = path;
        if (!path.empty() && !image.rgb.empty())
            image.result.written = writeBmp(path, width, height, image.rgb);
        return image.result;
    }

private:
    static bool translucentDepthLess(float lhs, float rhs) {
        return lhs < rhs;
    }

    static bool pvrFogRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_PVR_FOG");
        return !value || !*value || std::strcmp(value, "0") != 0;
    }

    static bool pvrAutosortRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_PVR_AUTOSORT");
        return !value || !*value || std::strcmp(value, "0") != 0;
    }

    static bool pvrModifierVolumesRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_PVR_MODIFIER_VOLUMES");
        return !value || !*value || std::strcmp(value, "0") != 0;
    }

    static bool pvrPunchAlphaRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_PVR_PUNCH_ALPHA");
        return !value || !*value || std::strcmp(value, "0") != 0;
    }

    static bool diagnosticOutlinesEnabled() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_OUTLINES");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    static float diagnosticFitTrimPercent() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_TRIM_PERCENT");
        if (!value || !*value) return 0.0f;
        char* end = nullptr;
        const float parsed = std::strtof(value, &end);
        if (end == value || !std::isfinite(parsed)) return 0.0f;
        return std::clamp(parsed, 0.0f, 45.0f);
    }

    struct Point {
        float x = 0.0f;
        float y = 0.0f;
        float depth = 0.0f;
        float eyeX = 0.0f;
        float eyeY = 0.0f;
        float eyeZ = 0.0f;
        float u = 0.0f;
        float v = 0.0f;
        uint32_t argb = 0xFFFFFFFFu;
        uint32_t offsetArgb = 0u;
        bool finiteEye = false;
        bool valid = false;
        bool hasUv = false;
    };

    struct TextureBinding {
        Texture texture{};
        TexEnv environment = TexEnv::Replace;
        bool ignoreAlpha = false;
        bool flipU = false;
        bool flipV = false;
        bool clampU = false;
        bool clampV = false;
        bool bilinear = false;
        bool valid = false;
    };

    struct RasterState {
        bool gouraud = true;
        bool useAlpha = false;
        bool offset = false;
        bool srcSelect = false;
        bool dstSelect = false;
        uint8_t srcBlend = 1u;
        uint8_t dstBlend = 0u;
        uint8_t cullMode = 0u;
        uint8_t depthMode = 6u;
        bool depthWrite = true;
        uint8_t fogMode = 2u;
        bool colorClamp = false;
        uint8_t listType = 0u;
        bool shadowed = false;
        uint8_t modifierMode = 0u;
        bool volumeLast = false;
        bool punchAlphaTest = false;
        // PVR User Tile Clip rectangle plus the Model command's clip mode.
        // Initial D uses this to confine its second camera to the rear-view
        // mirror instead of presenting it as a full-screen second view.
        uint32_t tileClip = (39u << 6u) | (14u << 17u);
    };

    struct TextureCacheEntry {
        uint32_t tsp = 0u;
        uint32_t tcw = 0u;
        TextureBinding binding{};
        uint32_t vramBytes = 0u;
        uint32_t vramHash = 0u;
        uint64_t validationGeneration = 0u;
        uint32_t firstIch = 0u;
        uint64_t batches = 0u;
        uint64_t vertices = 0u;
        uint64_t rasterPixels = 0u;
    };

    struct VertexColors {
        uint32_t base = 0xFFFFFFFFu;
        uint32_t offset = 0u;
        bool lit = false;
    };

    static std::array<float, 4> unpackArgb(uint32_t color) {
        return {
            static_cast<float>((color >> 16u) & 0xFFu) / 255.0f,
            static_cast<float>((color >> 8u) & 0xFFu) / 255.0f,
            static_cast<float>(color & 0xFFu) / 255.0f,
            static_cast<float>((color >> 24u) & 0xFFu) / 255.0f
        };
    }

    static uint32_t packArgb(const std::array<float, 4>& color) {
        const auto channel = [](float value) {
            return static_cast<uint32_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
        };
        return (channel(color[3]) << 24u) | (channel(color[0]) << 16u) |
               (channel(color[1]) << 8u) | channel(color[2]);
    }

    static float truncatedFloat16(uint16_t value) {
        const uint32_t bits = static_cast<uint32_t>(value) << 16u;
        float result = 0.0f;
        std::memcpy(&result, &bits, sizeof(result));
        return result;
    }

    static bool normalize3(std::array<float, 3>& value) {
        const float lengthSquared = value[0] * value[0] + value[1] * value[1] +
                                    value[2] * value[2];
        if (!std::isfinite(lengthSquared) || lengthSquared <= 1.0e-20f) return false;
        const float inverseLength = 1.0f / std::sqrt(lengthSquared);
        value[0] *= inverseLength;
        value[1] *= inverseLength;
        value[2] *= inverseLength;
        return true;
    }

    static float dot3(const std::array<float, 3>& lhs,
                      const std::array<float, 3>& rhs) {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
    }

    static VertexColors shadeVertexColors(const NativeElanDrawBatch& batch,
                                          const NativeElanVertexSample& vertex,
                                          const uint8_t fallback[3]) {
        VertexColors output{};
        output.base = vertex.hasVertexColor ? vertex.baseArgb : 0xFFFFFFFFu;
        output.offset = vertex.hasVertexColor ? vertex.offsetArgb : 0u;
        if (batch.material.valid) {
            const uint32_t params = batch.material.words[2];
            if ((params & 1u) != 0u) output.base = batch.material.words[3];
            if ((params & 2u) != 0u) output.offset = batch.material.words[4];
            // b0 selects a constant base color and explicitly bypasses ELAN
            // lighting. This is distinct from d0, which only supplies the
            // pre-light material diffuse color.
            if ((params & (1u << 9u)) != 0u) return output;
        } else if (!vertex.hasVertexColor) {
            output.base = 0xFF000000u | (static_cast<uint32_t>(fallback[0]) << 16u) |
                          (static_cast<uint32_t>(fallback[1]) << 8u) | fallback[2];
        }
        if (!batch.lightModel.valid || !batch.instance.valid) return output;

        const auto& normalMatrix = batch.instance.normalTransform;
        std::array<float, 3> normal = {
            normalMatrix[0] * vertex.nx + normalMatrix[1] * vertex.ny +
                normalMatrix[2] * vertex.nz,
            normalMatrix[3] * vertex.nx + normalMatrix[4] * vertex.ny +
                normalMatrix[5] * vertex.nz,
            normalMatrix[6] * vertex.nx + normalMatrix[7] * vertex.ny +
                normalMatrix[8] * vertex.nz
        };
        if (!normalize3(normal)) return output;

        const auto& transform = batch.instance.transform;
        std::array<float, 3> position = {
            -(transform[0] * vertex.x + transform[3] * vertex.y +
              transform[6] * vertex.z + transform[9]),
              transform[1] * vertex.x + transform[4] * vertex.y +
              transform[7] * vertex.z + transform[10],
            -(transform[2] * vertex.x + transform[5] * vertex.y +
              transform[8] * vertex.z + transform[11])
        };

        std::array<float, 4> base = unpackArgb(output.base);
        std::array<float, 4> offset = unpackArgb(output.offset);
        std::array<float, 3> diffuse{};
        std::array<float, 3> specular{};
        float diffuseAlpha = 0.0f;
        float specularAlpha = 0.0f;
        std::array<float, 3> view = position;
        normalize3(view);
        std::array<float, 3> reflected = {
            view[0] - 2.0f * dot3(view, normal) * normal[0],
            view[1] - 2.0f * dot3(view, normal) * normal[1],
            view[2] - 2.0f * dot3(view, normal) * normal[2]
        };
        normalize3(reflected);

        const auto& model = batch.lightModel.words;
        const uint32_t diffuseMask = model[2] & 0xFFFFu;
        const uint32_t specularMask = model[2] >> 16u;
        float gloss = 1.0f;
        if (batch.material.valid) {
            const uint32_t glossWord = batch.material.words[1];
            const float fraction = static_cast<float>(glossWord & 0x1Fu) / 32.0f;
            const float exponent = static_cast<float>((glossWord >> 5u) & 7u) - 1.0f;
            gloss = std::pow(2.0f, exponent) * (1.0f + fraction);
        }
        constexpr uint32_t kRoutingSpecToOffset = 1u;
        constexpr uint32_t kRoutingDiffToOffset = 2u;
        constexpr uint32_t kRoutingAlpha = 4u;
        constexpr uint32_t kRoutingSubtract = 8u;
        constexpr float kBaseFactor = 2.0f;

        for (const NativeElanLightState& light : batch.lights) {
            const uint32_t lightId = light.lightId;
            if (lightId >= 16u) continue;
            const bool hasDiffuse = (diffuseMask & (1u << lightId)) != 0u;
            const bool hasSpecular = (specularMask & (1u << lightId)) != 0u;
            if (!hasDiffuse && !hasSpecular) continue;
            if (!light.valid) continue;
            const auto& words = light.words;
            std::array<float, 3> lightColor = {
                static_cast<float>((words[1] >> 24u) & 0xFFu) / 255.0f,
                static_cast<float>((words[1] >> 16u) & 0xFFu) / 255.0f,
                static_cast<float>((words[1] >> 8u) & 0xFFu) / 255.0f
            };
            const auto signedByte = [](uint32_t value) {
                return static_cast<int32_t>(static_cast<int8_t>(value & 0xFFu));
            };
            std::array<float, 3> lightDirection = {
                -static_cast<float>((signedByte(words[2] >> 16u) << 4) |
                                    static_cast<int32_t>((words[0] >> 16u) & 0xFu)) / 2047.0f,
                -static_cast<float>((signedByte(words[2] >> 8u) << 4) |
                                    static_cast<int32_t>((words[0] >> 4u) & 0xFu)) / 2047.0f,
                -static_cast<float>((signedByte(words[2]) << 4) |
                                    static_cast<int32_t>(words[0] & 0xFu)) / 2047.0f
            };
            bool parallel = light.parallel;
            uint32_t dmode = parallel ? ((words[2] >> 28u) & 3u)
                                      : ((words[1] >> 5u) & 7u);
            const uint32_t smode = parallel ? 0u : ((words[2] >> 28u) & 3u);
            const uint32_t routing = (words[2] >> 24u) & 0xFu;
            if (!parallel) {
                const float distA = truncatedFloat16(static_cast<uint16_t>(words[6]));
                const float distB = truncatedFloat16(static_cast<uint16_t>(words[6] >> 16u));
                const float angleA = truncatedFloat16(static_cast<uint16_t>(words[7]));
                const float angleB = truncatedFloat16(static_cast<uint16_t>(words[7] >> 16u));
                const std::array<float, 3> lightPosition = {
                    nativeElanU32ToFloat(words[3]), nativeElanU32ToFloat(words[4]),
                    nativeElanU32ToFloat(words[5])
                };
                if (lightPosition[0] == 0.0f && lightPosition[1] == 0.0f &&
                    lightPosition[2] == 0.0f && distA == 0.0f && distB == 0.0f &&
                    angleA == 0.0f && angleB == 0.0f) {
                    parallel = true;
                    dmode = (words[1] >> 5u) & 7u;
                } else {
                    lightDirection = {
                        lightPosition[0] - position[0],
                        lightPosition[1] - position[1],
                        lightPosition[2] - position[2]
                    };
                    const float distanceSquared = dot3(lightDirection, lightDirection);
                    const float distance = std::sqrt(std::max(distanceSquared, 0.0f));
                    normalize3(lightDirection);
                    if (distA != 1.0f || distB != 0.0f) {
                        const float inputDistance = ((words[2] >> 31u) & 1u) == 0u &&
                                                    distance > 0.0f ? 1.0f / distance : distance;
                        const float attenuation = std::clamp(
                            distB * inputDistance + distA, 0.0f, 1.0f);
                        for (float& component : lightColor) component *= attenuation;
                    }
                    if (angleA != 1.0f || angleB != 0.0f) {
                        std::array<float, 3> spotDirection = {
                            -static_cast<float>((signedByte(words[2] >> 16u) << 4) |
                                                static_cast<int32_t>((words[0] >> 16u) & 0xFu)) / 2047.0f,
                            -static_cast<float>((signedByte(words[2] >> 8u) << 4) |
                                                static_cast<int32_t>((words[0] >> 4u) & 0xFu)) / 2047.0f,
                            -static_cast<float>((signedByte(words[2]) << 4) |
                                                static_cast<int32_t>(words[0] & 0xFu)) / 2047.0f
                        };
                        const float cosAngle = 1.0f - std::max(0.0f, dot3(lightDirection, spotDirection));
                        const float attenuation = std::clamp(cosAngle * angleB + angleA,
                                                             0.0f, 1.0f);
                        for (float& component : lightColor) component *= attenuation;
                    }
                }
            }
            normalize3(lightDirection);
            if (hasDiffuse) {
                float factor = (routing & kRoutingSubtract) != 0u ? -kBaseFactor : kBaseFactor;
                const float ndotl = dot3(normal, lightDirection);
                if (dmode == 0u) factor *= std::max(ndotl, 0.0f);
                else if (dmode == 1u) factor *= std::fabs(ndotl);
                if ((routing & kRoutingAlpha) != 0u) {
                    diffuseAlpha += lightColor[0] * factor;
                } else {
                    auto& destination = (routing & kRoutingDiffToOffset) == 0u ? diffuse : specular;
                    for (unsigned channel = 0; channel < 3; ++channel)
                        destination[channel] += lightColor[channel] * factor * base[channel];
                }
            }
            if (hasSpecular) {
                float factor = (routing & kRoutingSubtract) != 0u ? -kBaseFactor : kBaseFactor;
                const float dotValue = dot3(lightDirection, reflected);
                if (smode == 0u)
                    factor *= std::clamp(std::pow(std::max(dotValue, 0.0f), gloss), 0.0f, 1.0f);
                else if (smode == 1u)
                    factor *= std::clamp(std::pow(std::fabs(dotValue), gloss), 0.0f, 1.0f);
                if ((routing & kRoutingAlpha) != 0u) {
                    specularAlpha += lightColor[0] * factor;
                } else {
                    auto& destination = (routing & kRoutingSpecToOffset) == 0u ? diffuse : specular;
                    for (unsigned channel = 0; channel < 3; ++channel)
                        destination[channel] += lightColor[channel] * factor * offset[channel];
                }
            }
        }

        const uint32_t modelFlags = model[1];
        const auto ambientBase = unpackArgb(model[3]);
        const auto ambientOffset = unpackArgb(model[4]);
        for (unsigned channel = 0; channel < 3; ++channel) {
            diffuse[channel] += (modelFlags & (1u << 5u)) != 0u
                ? ambientBase[channel] * base[channel] : ambientBase[channel];
            specular[channel] += (modelFlags & (1u << 6u)) != 0u
                ? ambientOffset[channel] * offset[channel] : ambientOffset[channel];
        }
        base = {diffuse[0], diffuse[1], diffuse[2], base[3] + diffuseAlpha};
        offset = {specular[0], specular[1], specular[2], offset[3] + specularAlpha};
        if ((modelFlags & (1u << 9u)) != 0u) {
            for (unsigned channel = 0; channel < 4; ++channel)
                offset[channel] += std::max(base[channel] - 1.0f, 0.0f);
        }
        output.base = packArgb(base);
        output.offset = packArgb(offset);
        output.lit = true;
        return output;
    }

    static void applyEnvironmentMap(const NativeElanInstanceState& instance,
                                    const NativeElanVertexSample& vertex,
                                    float& u, float& v, bool& hasUv) {
        const auto& matrix = instance.normalTransform;
        std::array<float, 3> normal = {
            matrix[0] * vertex.nx + matrix[1] * vertex.ny + matrix[2] * vertex.nz,
            matrix[3] * vertex.nx + matrix[4] * vertex.ny + matrix[5] * vertex.nz,
            matrix[6] * vertex.nx + matrix[7] * vertex.ny + matrix[8] * vertex.nz
        };
        if (!normalize3(normal)) return;
        // Flycast's Naomi 2 vertex path: UV += normal.xy / 2 + 0.5.
        // Clamp is part of ELAN environment mapping, independent of the PVR
        // texture wrap bits that are applied later by the sampler.
        // Flycast seeds in_uv from InstanceMatrix::envMapU/envMapV whenever
        // GMP selects environment mapping; the Naomi 2 vertex shader then
        // adds the transformed normal term. The ordinary model UV is not used
        // for this path.
        u = std::clamp(instance.envMapUOffset + normal[0] * 0.5f + 0.5f,
                       0.0f, 1.0f);
        v = std::clamp(instance.envMapVOffset + normal[1] * 0.5f + 0.5f,
                       0.0f, 1.0f);
        hasUv = true;
    }

    static bool capturedProjectionRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_PROJECTION");
        if (value && *value) return std::strcmp(value, "0") != 0;
        const char* window = std::getenv("IDAS3_NATIVE_WINDOW");
        return window && *window && std::strcmp(window, "0") != 0;
    }

    static bool identityInstanceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_IDENTITY_INSTANCE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    // Diagnostic render-pass isolation. Initial D builds multiple ELAN passes
    // in the same resident ring; their distinct projection focal lengths must
    // not automatically be composited into one framebuffer. Zero preserves
    // the unrestricted Flycast identity-matrix A/B behaviour.
    static float identityMinimumFocalLength() {
        const char* value =
            std::getenv("IDAS3_NATIVE_FRAMEBUFFER_IDENTITY_MIN_FOCAL");
        if (!value || !*value) return 0.0f;
        char* end = nullptr;
        const float parsed = std::strtof(value, &end);
        return end != value && std::isfinite(parsed) && parsed > 0.0f
            ? parsed : 0.0f;
    }

    // Flycast assigns IdentityMatIndex when ELAN submits a polygon without an
    // active Instance command. NativeElanInstanceState stores the unexpanded
    // ELAN coefficients, so (-1,+1,-1) on the diagonal expands to a true host
    // identity matrix in projectVertex's ELAN sign convention.
    static const NativeElanInstanceState& identityInstanceState() {
        static const NativeElanInstanceState identity = [] {
            NativeElanInstanceState state{};
            state.offset = 0xFFFFFFFFu;
            state.envMapUOffset = 0.0f;
            state.envMapVOffset = 0.0f;
            state.nearValue = 0.001f;
            state.normalTransform = {1.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f};
            state.transform = {-1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, -1.0f,
                                0.0f, 0.0f, 0.0f};
            state.farValue = 100000.0f;
            state.inverseNear = 1000.0f;
            state.valid = true;
            return state;
        }();
        return identity;
    }

    static const NativeElanInstanceState& projectionInstance(
            const NativeElanDrawBatch& batch, bool useIdentityInstance) {
        return batch.instance.valid || !useIdentityInstance
            ? batch.instance : identityInstanceState();
    }

    static bool lightingTraceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_LIGHT_TRACE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    static bool capturedProjectionTraceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_PROJECTION_TRACE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    static bool textureTraceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_TEXTURE_TRACE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    static bool pixelOwnerTraceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_PIXEL_OWNER_TRACE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    // Read-only per-vertex dump for one ICH offset, selected by
    // IDAS3_NATIVE_DIAG_DUMP_ICH=<hex>. Used to identify exactly what a broad
    // batch covers without changing any rendering behaviour.
    static void dumpBatchGeometry(size_t batchIndex,
                                  const NativeElanDrawBatch& batch,
                                  const std::vector<Point>& points,
                                  const RasterState& rasterState) {
        const char* value = std::getenv("IDAS3_NATIVE_DIAG_DUMP_ICH");
        if (!value || !*value) return;
        const uint32_t wanted =
            static_cast<uint32_t>(std::strtoul(value, nullptr, 16));
        if (batch.ichOffset != wanted) return;
        std::fprintf(stderr,
            "[NATIVE-ELAN-BATCH-DUMP] batch=%zu ich=%08X flags=%03X pcw=%08X isp=%08X "
            "tsp0=%08X tcw0=%08X tsp1=%08X tcw1=%08X vertices=%u "
            "cull=%u depth_mode=%u depth_write=%u "
            "instance=%08X projection=%08X "
            "model_valid=%u model_pcw=%08X model_param=%08X model_tsp=%08X "
            "material=%08X params=%08X diffuse=%08X specular=%08X "
            "proj=%.9g,%.9g,%.9g,%.9g near=%.9g far=%.9g\n",
            batchIndex, batch.ichOffset, batch.flags, batch.pcw, batch.ispTsp,
            batch.tsp0, batch.tcw0, batch.tsp1, batch.tcw1, batch.vertexCount,
            static_cast<unsigned>(rasterState.cullMode),
            static_cast<unsigned>(rasterState.depthMode),
            rasterState.depthWrite ? 1u : 0u,
            batch.instance.offset, batch.projection.offset,
            batch.model.valid ? 1u : 0u, batch.model.pcw, batch.model.param,
            batch.model.tsp,
            batch.material.offset,
            batch.material.valid ? batch.material.words[2] : 0u,
            batch.material.valid ? batch.material.words[3] : 0u,
            batch.material.valid ? batch.material.words[4] : 0u,
            batch.projection.fx, batch.projection.tx,
            batch.projection.fy, batch.projection.ty,
            batch.instance.nearValue, batch.instance.farValue);
        for (size_t i = 0; i < points.size() && i < 512u; ++i) {
            const auto& v = batch.vertices[i];
            const auto& p = points[i];
            std::fprintf(stderr,
                "[NATIVE-ELAN-BATCH-VERTEX] batch=%zu i=%zu header=%08X strip=%u fan=%u "
                "eos=%u obj=%.6g,%.6g,%.6g eye=%.6g,%.6g,%.6g screen=%.6g,%.6g "
                "depth=%.9g valid=%u uv=%.6g,%.6g argb=%08X offset=%08X\n",
                batchIndex, i, v.header,
                static_cast<unsigned>((v.header >> 29u) & 1u),
                static_cast<unsigned>((v.header >> 30u) & 1u),
                static_cast<unsigned>((v.header >> 31u) & 1u),
                v.x, v.y, v.z, p.eyeX, p.eyeY, p.eyeZ, p.x, p.y, p.depth,
                p.valid ? 1u : 0u, p.u, p.v, p.argb, p.offsetArgb);
        }
    }

    // Flycast setStateParams():
    //   pp.tsp.full  ^= modelTSP.full;
    //   pp.tsp1.full ^= modelTSP.full;
    //   bool projFlip = signbit(projMat[0][0]) == signbit(projMat[1][1]);
    //   pp.isp.CullMode ^= (u32)cullingReversed ^ (u32)projFlip;
    // projectionMatrix[0][0] is -fx and [1][1] is +fy, so the flip test is
    // signbit(-fx) == signbit(fy).
    static bool modelStateEnabled() {
        const char* value = std::getenv("IDAS3_NATIVE_ELAN_MODEL_STATE");
        return !value || !*value || std::strcmp(value, "0") != 0;
    }

    static bool projectionFlipped(const NativeElanDrawBatch& batch) {
        if (!batch.projection.valid) return false;
        return std::signbit(-batch.projection.fx) == std::signbit(batch.projection.fy);
    }

    static bool environmentMappingSelected(const NativeElanDrawBatch& batch,
                                           bool secondVolume) {
        if (!batch.material.valid) return false;
        const uint32_t bit = secondVolume ? 12u : 11u;
        return (batch.material.words[2] & (1u << bit)) != 0u;
    }

    static uint32_t effectiveTsp(const NativeElanDrawBatch& batch, uint32_t tsp,
                                 bool secondVolume) {
        // Flycast setStateParams forces environment-mapped volumes to use
        // texture alpha before applying the model TSP XOR.
        if (environmentMappingSelected(batch, secondVolume)) {
            tsp |= 1u << 20u;  // UseAlpha
            tsp &= ~(1u << 19u); // IgnoreTexA
        }
        if (!modelStateEnabled() || !batch.model.valid) return tsp;
        return tsp ^ batch.model.tsp;
    }

    static uint32_t effectivePcw(const NativeElanDrawBatch& batch) {
        uint32_t pcw = batch.pcw;
        // curGmp->paramSelect.e0/e1 in Flycast enables texturing and disables
        // offset color even when the ICH's original object control did not.
        if (environmentMappingSelected(batch, false) ||
            environmentMappingSelected(batch, true)) {
            pcw |= 1u << 3u;   // Texture
            pcw &= ~(1u << 2u); // Offset
        }
        if (modelStateEnabled() && batch.model.valid &&
            batch.model.shadowedVolume)
            pcw ^= 1u << 7u;
        return pcw;
    }

    static bool diagFlipCullRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_DIAG_FLIP_CULL");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    static uint32_t effectiveIspTsp(const NativeElanDrawBatch& batch) {
        bool flip = diagFlipCullRequested();
        if (modelStateEnabled())
            flip = flip != ((batch.model.valid && batch.model.cullingReversed) !=
                            projectionFlipped(batch));
        if (!flip) return batch.ispTsp;
        return batch.ispTsp ^ (1u << 27u);
    }

    // Flycast isBetweenNearAndFar(): the object-space bounding box is
    // transformed by the model-view matrix using the absolute per-axis
    // extents, and the whole polygon is dropped when it lies entirely nearer
    // than the near plane or entirely beyond the far plane.
    static bool nearFarCullEnabled() {
        const char* value = std::getenv("IDAS3_NATIVE_ELAN_NEAR_FAR_CULL");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    static bool batchBetweenNearAndFar(const NativeElanDrawBatch& batch,
                                       const NativeElanInstanceState& instance) {
        if (!instance.valid || batch.vertices.empty()) return true;
        float minObject[3] = {1.0e38f, 1.0e38f, 1.0e38f};
        float maxObject[3] = {-1.0e38f, -1.0e38f, -1.0e38f};
        for (const auto& v : batch.vertices) {
            if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z))
                return true;
            minObject[0] = std::min(minObject[0], v.x);
            minObject[1] = std::min(minObject[1], v.y);
            minObject[2] = std::min(minObject[2], v.z);
            maxObject[0] = std::max(maxObject[0], v.x);
            maxObject[1] = std::max(maxObject[1], v.y);
            maxObject[2] = std::max(maxObject[2], v.z);
        }
        const float centerObject[3] = {
            (minObject[0] + maxObject[0]) * 0.5f,
            (minObject[1] + maxObject[1]) * 0.5f,
            (minObject[2] + maxObject[2]) * 0.5f
        };
        const float extents[3] = {
            maxObject[0] - centerObject[0],
            maxObject[1] - centerObject[1],
            maxObject[2] - centerObject[2]
        };
        const auto& m = instance.transform;
        // Eye Z row of the ELAN model-view matrix: -(tm02, tm12, tm22, tm32).
        const float centerZ = -(m[2] * centerObject[0] + m[5] * centerObject[1] +
                                m[8] * centerObject[2] + m[11]);
        const float extentZ = std::fabs(m[2] * extents[0]) +
                              std::fabs(m[5] * extents[1]) +
                              std::fabs(m[8] * extents[2]);
        const float minZ = centerZ - extentZ;
        const float maxZ = centerZ + extentZ;
        const float nearPlane = instance.nearValue;
        const float farPlane = instance.farValue;
        if (!std::isfinite(minZ) || !std::isfinite(maxZ)) return true;
        if (minZ > -nearPlane) return false;
        if (farPlane > 0.0f && maxZ < -farPlane) return false;
        return true;
    }

    // Diagnostic isolation: render only, or skip, the listed ICH offsets.
    // Comma-separated hex. Default empty; never used by an accepted frame.
    static bool ichInEnvList(const char* name, uint32_t ichOffset, bool& present) {
        const char* value = std::getenv(name);
        present = value && *value;
        if (!present) return false;
        const char* cursor = value;
        while (*cursor) {
            char* end = nullptr;
            const unsigned long parsed = std::strtoul(cursor, &end, 16);
            if (end == cursor) break;
            if (static_cast<uint32_t>(parsed) == ichOffset) return true;
            cursor = end;
            while (*cursor == ',' || *cursor == ' ') ++cursor;
        }
        return false;
    }

    static bool batchDiagnosticallyExcluded(uint32_t ichOffset,
                                             uint32_t sourceRootOffset) {
        bool present = false;
        if (ichInEnvList("IDAS3_NATIVE_DIAG_SKIP_ICH", ichOffset, present)) return true;
        const bool onlyMatch =
            ichInEnvList("IDAS3_NATIVE_DIAG_ONLY_ICH", ichOffset, present);
        if (present && !onlyMatch) return true;

        if (ichInEnvList("IDAS3_NATIVE_DIAG_SKIP_SOURCE_ROOT",
                         sourceRootOffset, present))
            return true;
        const bool onlyRootMatch =
            ichInEnvList("IDAS3_NATIVE_DIAG_ONLY_SOURCE_ROOT",
                         sourceRootOffset, present);
        return present && !onlyRootMatch;
    }

    static bool cullTraceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_DIAG_CULL_TRACE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    // PVR ISP culling, per the hardware table Flycast documents in
    // rend/gles/gldraw.cpp:
    //   0  no culling
    //   1  cull if small       (|det| < fpu_cull_val)
    //   2  cull if negative    (det < 0)
    //   3  cull if positive    (det > 0)
    // `det` is the conventional 2-D cross product (b-a) x (c-a) evaluated in
    // the PVR's screen space. edge() returns the negative of that product, so
    // the determinant has to be negated before the table is applied. The
    // earlier code compared edge()'s sign directly and therefore culled the
    // opposite face of every batch: an inner hull whose object-space bounding
    // box is strictly contained by the painted body (ICH 00E827A0 inside ICH
    // 00EA1140) was occluding the body it sits inside, which is geometrically
    // impossible. Mode 1's tiny-triangle threshold is left uncullled, matching
    // Flycast's host renderers.
    static bool triangleCulled(const Point& a, const Point& b, const Point& c,
                               const RasterState& rasterState) {
        const float determinant = -edge(a, b, c.x, c.y);
        return (rasterState.cullMode == 2u && determinant < 0.0f) ||
               (rasterState.cullMode == 3u && determinant > 0.0f);
    }

    static bool modelStateTraceRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_MODEL_STATE_TRACE");
        return value && *value && std::strcmp(value, "0") != 0;
    }

    // Read-only inventory of the ELAN Model command context attached to each
    // accepted batch. Flycast XORs modelTSP into the polygon TSP and XORs the
    // ISP cull mode with cullingReversed ^ projFlip, so a nonzero inventory
    // here means the raw ICH words are not the submitted render state.
    static void traceModelStates(
            const std::vector<const NativeElanDrawBatch*>& batches) {
        if (!modelStateTraceRequested()) return;
        struct ModelSummary {
            uint32_t pcw = 0u;
            uint32_t param = 0u;
            uint32_t tsp = 0u;
            uint32_t batches = 0u;
            uint32_t vertices = 0u;
            uint32_t cullModes = 0u;
        };
        std::vector<ModelSummary> summaries;
        uint32_t modelledBatches = 0u;
        uint32_t nonZeroTspBatches = 0u;
        uint32_t cullingReversedBatches = 0u;
        uint32_t shadowedBatches = 0u;
        uint32_t openVolumeBatches = 0u;
        for (const NativeElanDrawBatch* batch : batches) {
            const auto& model = batch->model;
            if (!model.valid) continue;
            ++modelledBatches;
            if (model.tsp != 0u) ++nonZeroTspBatches;
            if (model.cullingReversed) ++cullingReversedBatches;
            if (model.shadowedVolume) ++shadowedBatches;
            if (model.openVolume) ++openVolumeBatches;
            auto found = std::find_if(summaries.begin(), summaries.end(),
                [&](const ModelSummary& summary) {
                    return summary.pcw == model.pcw && summary.param == model.param &&
                           summary.tsp == model.tsp;
                });
            if (found == summaries.end()) {
                summaries.push_back({model.pcw, model.param, model.tsp, 0u, 0u, 0u});
                found = summaries.end() - 1;
            }
            ++found->batches;
            found->vertices += batch->vertexCount;
            found->cullModes |= 1u << ((batch->ispTsp >> 27u) & 3u);
        }
        std::fprintf(stderr,
            "[NATIVE-ELAN-MODEL-STATE] batches=%zu modelled=%u nonzero_model_tsp=%u "
            "culling_reversed=%u shadowed=%u open_volume=%u unique=%zu\n",
            batches.size(), modelledBatches, nonZeroTspBatches,
            cullingReversedBatches, shadowedBatches, openVolumeBatches,
            summaries.size());
        std::stable_sort(summaries.begin(), summaries.end(),
            [](const ModelSummary& lhs, const ModelSummary& rhs) {
                return lhs.batches > rhs.batches;
            });
        uint32_t emitted = 0u;
        for (const ModelSummary& summary : summaries) {
            if (emitted >= 24u) break;
            std::fprintf(stderr,
                "[NATIVE-ELAN-MODEL-STATE-ENTRY] rank=%u pcw=%08X param=%08X tsp=%08X "
                "cw_culling=%u open_volume=%u shadow=%u batches=%u vertices=%u "
                "ich_cull_mode_mask=%X\n",
                emitted, summary.pcw, summary.param, summary.tsp,
                static_cast<unsigned>((summary.param >> 27u) & 1u),
                static_cast<unsigned>((summary.param >> 28u) & 1u),
                static_cast<unsigned>((summary.pcw >> 7u) & 1u),
                summary.batches, summary.vertices, summary.cullModes);
            ++emitted;
        }
    }

    static const char* pixelOwnerBmpPathRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_PIXEL_OWNER_BMP");
        return value && *value ? value : nullptr;
    }

    // Deterministic, well-separated false color for a batch index. The golden
    // angle keeps neighbouring batch numbers far apart in hue so adjacent
    // submissions never share a shade, and the alternating value/saturation
    // ladder separates hues that wrap around. Diagnostic only.
    static void falseColorForBatch(uint32_t batch, uint8_t rgb[3]) {
        const float hue = std::fmod(static_cast<float>(batch) * 137.50776f, 360.0f);
        const float saturation = 0.62f + 0.30f * static_cast<float>(batch % 3u) / 2.0f;
        const float value = 0.58f + 0.38f * static_cast<float>((batch / 3u) % 3u) / 2.0f;
        const float chroma = value * saturation;
        const float sector = hue / 60.0f;
        const float second = chroma * (1.0f - std::fabs(std::fmod(sector, 2.0f) - 1.0f));
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        switch (static_cast<int>(sector)) {
            case 0: r = chroma; g = second; break;
            case 1: r = second; g = chroma; break;
            case 2: g = chroma; b = second; break;
            case 3: g = second; b = chroma; break;
            case 4: r = second; b = chroma; break;
            default: r = chroma; b = second; break;
        }
        const float match = value - chroma;
        rgb[0] = static_cast<uint8_t>(std::clamp((r + match) * 255.0f, 0.0f, 255.0f));
        rgb[1] = static_cast<uint8_t>(std::clamp((g + match) * 255.0f, 0.0f, 255.0f));
        rgb[2] = static_cast<uint8_t>(std::clamp((b + match) * 255.0f, 0.0f, 255.0f));
    }

    static const char* textureAtlasPathRequested() {
        return std::getenv("IDAS3_NATIVE_FRAMEBUFFER_TEXTURE_ATLAS");
    }

    struct TextureStateSummary {
        uint32_t pcw = 0u;
        uint32_t tsp = 0u;
        uint32_t tcw = 0u;
        uint32_t materialParams = 0u;
        uint64_t batches = 0u;
        uint64_t vertices = 0u;
        uint64_t uvVertices = 0u;
    };

    static uint32_t textureStorageBytes(uint32_t width, uint32_t height,
                                        uint32_t pixelFormat, bool vq,
                                        bool mipmapped) {
        uint64_t bytes = 0u;
        const auto levelBytes = [&](uint32_t w, uint32_t h) -> uint64_t {
            if (vq) return 2048ull + static_cast<uint64_t>(w) * h / 4u;
            if (pixelFormat == 5u) return static_cast<uint64_t>(w) * h / 2u;
            if (pixelFormat == 6u) return static_cast<uint64_t>(w) * h;
            return static_cast<uint64_t>(w) * h * 2u;
        };
        if (!mipmapped) {
            bytes = levelBytes(width, height);
        } else {
            for (uint32_t w = width, h = height;; w = std::max(1u, w / 2u),
                                                    h = std::max(1u, h / 2u)) {
                bytes += levelBytes(w, h);
                if (w == 1u && h == 1u) break;
            }
        }
        return bytes > 0xFFFFFFFFull ? 0u : static_cast<uint32_t>(bytes);
    }

    static uint32_t hashVramRange(const std::vector<uint8_t>* vram,
                                  uint32_t address, uint32_t bytes,
                                  uint32_t& nonzero) {
        nonzero = 0u;
        if (!vram || bytes == 0u || address > vram->size() ||
            bytes > vram->size() - address) return 0u;
        uint32_t hash = 2166136261u;
        for (uint32_t i = 0; i < bytes; ++i) {
            const uint8_t value = (*vram)[address + i];
            nonzero += value != 0u ? 1u : 0u;
            hash ^= value;
            hash *= 16777619u;
        }
        return hash;
    }

    static TextureBinding decodeTextureBinding(
            uint32_t tsp, uint32_t tcw, const std::vector<uint8_t>* vram) {
        TextureBinding binding{};
        binding.environment = static_cast<TexEnv>((tsp >> 6u) & 3u);
        binding.flipU = (tsp & (1u << 18u)) != 0u;
        binding.flipV = (tsp & (1u << 17u)) != 0u;
        binding.clampU = (tsp & (1u << 16u)) != 0u;
        binding.clampV = (tsp & (1u << 15u)) != 0u;
        binding.bilinear = ((tsp >> 13u) & 3u) != 0u;

        const uint32_t width = 8u << ((tsp >> 3u) & 7u);
        const uint32_t height = 8u << (tsp & 7u);
        const uint32_t address = (tcw & 0x001FFFFFu) << 3u;
        const bool stride = (tcw & (1u << 25u)) != 0u;
        const bool scanOrder = (tcw & (1u << 26u)) != 0u;
        const uint32_t pixelFormat = (tcw >> 27u) & 7u;
        // Flycast treats RGB565 as alpha-one regardless of IgnoreTexA.
        binding.ignoreAlpha = (tsp & (1u << 19u)) != 0u || pixelFormat == 1u;
        const bool vq = (tcw & (1u << 30u)) != 0u;
        const bool mipmapped = (tcw & (1u << 31u)) != 0u;
        const uint64_t pixelCount = static_cast<uint64_t>(width) * height;
        const uint64_t bytes = pixelCount * sizeof(uint16_t);

        // The currently captured IDAS3 states are direct 16-bit PVR textures.
        // Keep unsupported palette/VQ/mipmap/stride modes explicit so the
        // diagnostic renderer never guesses a hardware layout.
        if (!vram || stride || vq || mipmapped || pixelFormat > 2u ||
            pixelCount == 0u || bytes > 0xFFFFFFFFull ||
            address > vram->size() || bytes > vram->size() - address)
            return binding;

        std::vector<uint16_t> raw(static_cast<size_t>(pixelCount));
        std::memcpy(raw.data(), vram->data() + address, static_cast<size_t>(bytes));
        const auto format = static_cast<PvrPixelFormat>(pixelFormat);
        if (scanOrder) {
            binding.texture.width = static_cast<int>(width);
            binding.texture.height = static_cast<int>(height);
            binding.texture.texels.resize(static_cast<size_t>(pixelCount));
            for (size_t i = 0; i < raw.size(); ++i)
                binding.texture.texels[i] = pvrPixelToRGBA(raw[i], format);
        } else {
            binding.texture = pvrUntwiddleTexture(
                raw.data(), static_cast<int>(width), static_cast<int>(height), format);
        }
        binding.valid = binding.texture.valid();
        return binding;
    }

    static RasterState decodeRasterState(uint32_t pcw, uint32_t ispTsp, uint32_t tsp) {
        RasterState state{};
        // PCW.Gouraud is copied into ISP/TSP by the TA and is the authoritative
        // polygon declaration in Flycast's renderer state.
        state.gouraud = (pcw & (1u << 1u)) != 0u;
        state.useAlpha = (tsp & (1u << 20u)) != 0u;
        state.offset = (pcw & (1u << 2u)) != 0u;
        state.dstSelect = (tsp & (1u << 24u)) != 0u;
        state.srcSelect = (tsp & (1u << 25u)) != 0u;
        state.dstBlend = static_cast<uint8_t>((tsp >> 26u) & 7u);
        state.srcBlend = static_cast<uint8_t>((tsp >> 29u) & 7u);
        state.cullMode = static_cast<uint8_t>((ispTsp >> 27u) & 3u);
        state.depthMode = static_cast<uint8_t>((ispTsp >> 29u) & 7u);
        state.depthWrite = (ispTsp & (1u << 26u)) == 0u;
        state.fogMode = static_cast<uint8_t>((tsp >> 22u) & 3u);
        state.colorClamp = (tsp & (1u << 21u)) != 0u;
        state.listType = static_cast<uint8_t>((pcw >> 24u) & 7u);
        state.shadowed = (pcw & (1u << 7u)) != 0u;
        state.punchAlphaTest = state.listType == 4u && pvrPunchAlphaRequested();
        // PowerVR punch-through polygons use the same fixed depth behavior as
        // Flycast's hardware path: greater-or-equal testing with depth writes
        // enabled, regardless of the polygon's encoded DepthMode/ZWriteDis.
        // PCW bits 26:24 select the TA list and value 4 is punch-through.
        if (((pcw >> 24u) & 7u) == 4u) {
            state.depthMode = 6u;
            state.depthWrite = true;
        }
        return state;
    }

    static void applyPvrListDepthState(RasterState& state,
                                       bool autosortTranslucent) {
        // Flycast v2.6 SetGPState uses fixed >= depth testing for both
        // punch-through polygons and auto-sorted translucent polygons. Sorted
        // translucent fragments do not update depth; punch-through fragments
        // always do. decodeRasterState already applies the punch-through rule.
        if (state.listType == 2u && autosortTranslucent) {
            state.depthMode = 6u;
            state.depthWrite = false;
        }
    }

    static int textureIndex(int64_t value, int size, bool clamp, bool flip) {
        if (clamp)
            return static_cast<int>(std::clamp<int64_t>(value, 0, size - 1));
        // Native PVR texture dimensions are powers of two. Wrapping and
        // mirrored wrapping can therefore use masks instead of signed 64-bit
        // division/modulo for every nearest or bilinear texel fetch.
        if (size > 0 && (size & (size - 1)) == 0) {
            const uint64_t period = static_cast<uint64_t>(size) * (flip ? 2u : 1u);
            uint64_t index = static_cast<uint64_t>(value) & (period - 1u);
            if (flip && index >= static_cast<uint64_t>(size))
                index = period - 1u - index;
            return static_cast<int>(index);
        }
        int64_t period = value / size;
        int64_t index = value % size;
        if (index < 0) {
            index += size;
            --period;
        }
        if (flip && (period % 2) != 0)
            index = size - 1 - index;
        return static_cast<int>(index);
    }

    static uint32_t textureTexel(const TextureBinding& binding, int64_t x, int64_t y) {
        const int ix = textureIndex(x, binding.texture.width,
                                    binding.clampU, binding.flipU);
        const int iy = textureIndex(y, binding.texture.height,
                                    binding.clampV, binding.flipV);
        return binding.texture.texels[
            static_cast<size_t>(iy) * binding.texture.width + ix];
    }

    static uint32_t sampleTexture(const TextureBinding& binding, float u, float v) {
        if (!binding.valid || !std::isfinite(u) || !std::isfinite(v))
            return 0u;
        const double tx = static_cast<double>(u) * binding.texture.width;
        const double ty = static_cast<double>(v) * binding.texture.height;
        if (!binding.bilinear) {
            return textureTexel(binding,
                static_cast<int64_t>(std::floor(tx)),
                static_cast<int64_t>(std::floor(ty)));
        }

        const double fx = tx - 0.5;
        const double fy = ty - 0.5;
        const int64_t x0 = static_cast<int64_t>(std::floor(fx));
        const int64_t y0 = static_cast<int64_t>(std::floor(fy));
        const double dx = fx - static_cast<double>(x0);
        const double dy = fy - static_cast<double>(y0);
        // Resolve each wrapped/clamped coordinate once. The four bilinear
        // texels share two X and two Y coordinates; calling textureTexel four
        // times used to repeat the power-of-two wrap work eight times for
        // every shaded pixel.
        const int ix0 = textureIndex(x0, binding.texture.width,
                                     binding.clampU, binding.flipU);
        const int ix1 = textureIndex(x0 + 1, binding.texture.width,
                                     binding.clampU, binding.flipU);
        const int iy0 = textureIndex(y0, binding.texture.height,
                                     binding.clampV, binding.flipV);
        const int iy1 = textureIndex(y0 + 1, binding.texture.height,
                                     binding.clampV, binding.flipV);
        const size_t row0 = static_cast<size_t>(iy0) * binding.texture.width;
        const size_t row1 = static_cast<size_t>(iy1) * binding.texture.width;
        const uint32_t c00 = binding.texture.texels[row0 + ix0];
        const uint32_t c10 = binding.texture.texels[row0 + ix1];
        const uint32_t c01 = binding.texture.texels[row1 + ix0];
        const uint32_t c11 = binding.texture.texels[row1 + ix1];
        const auto channel = [&](uint32_t shift) -> uint32_t {
            const double top = static_cast<double>((c00 >> shift) & 0xFFu) * (1.0 - dx) +
                               static_cast<double>((c10 >> shift) & 0xFFu) * dx;
            const double bottom = static_cast<double>((c01 >> shift) & 0xFFu) * (1.0 - dx) +
                                  static_cast<double>((c11 >> shift) & 0xFFu) * dx;
            // All inputs and bilinear weights are non-negative and bounded by
            // 255, so round-to-nearest is exactly floor(value + 0.5). Avoid a
            // general signed lround/clamp call for each of four channels.
            return static_cast<uint32_t>(
                top * (1.0 - dy) + bottom * dy + 0.5);
        };
        return (channel(24u) << 24u) | (channel(16u) << 16u) |
               (channel(8u) << 8u) | channel(0u);
    }

    static bool writeTextureAtlas(const std::string& path,
                                  const std::vector<TextureCacheEntry>& entries) {
        if (path.empty() || entries.empty()) return false;
        const uint32_t columns = std::min<uint32_t>(
            16u, std::max<uint32_t>(1u, static_cast<uint32_t>(entries.size())));
        constexpr uint32_t cell = 256u;
        const uint32_t rows = static_cast<uint32_t>(
            (entries.size() + columns - 1u) / columns);
        const uint32_t width = columns * cell;
        const uint32_t height = rows * cell;
        std::vector<uint8_t> rgb(static_cast<size_t>(width) * height * 3u, 24u);
        for (size_t entryIndex = 0; entryIndex < entries.size(); ++entryIndex) {
            const Texture& texture = entries[entryIndex].binding.texture;
            if (!entries[entryIndex].binding.valid) continue;
            const uint32_t cellX = static_cast<uint32_t>(entryIndex % columns) * cell;
            const uint32_t cellY = static_cast<uint32_t>(entryIndex / columns) * cell;
            const float scale = std::min(
                static_cast<float>(cell - 4u) / texture.width,
                static_cast<float>(cell - 4u) / texture.height);
            const uint32_t drawWidth = std::max(1u, static_cast<uint32_t>(texture.width * scale));
            const uint32_t drawHeight = std::max(1u, static_cast<uint32_t>(texture.height * scale));
            const uint32_t originX = cellX + (cell - drawWidth) / 2u;
            const uint32_t originY = cellY + (cell - drawHeight) / 2u;
            for (uint32_t y = 0; y < drawHeight; ++y) {
                const uint32_t sourceY = std::min<uint32_t>(
                    texture.height - 1, static_cast<uint64_t>(y) * texture.height / drawHeight);
                for (uint32_t x = 0; x < drawWidth; ++x) {
                    const uint32_t sourceX = std::min<uint32_t>(
                        texture.width - 1, static_cast<uint64_t>(x) * texture.width / drawWidth);
                    const uint32_t color = texture.texels[
                        static_cast<size_t>(sourceY) * texture.width + sourceX];
                    const size_t output = (static_cast<size_t>(originY + y) * width +
                                           originX + x) * 3u;
                    rgb[output + 0u] = static_cast<uint8_t>(color >> 16u);
                    rgb[output + 1u] = static_cast<uint8_t>(color >> 8u);
                    rgb[output + 2u] = static_cast<uint8_t>(color);
                }
            }
        }
        return writeBmp(path, width, height, rgb);
    }

    static void traceTextureStates(const std::vector<const NativeElanDrawBatch*>& batches,
                                   const std::vector<uint8_t>* vram,
                                   NativeElanFramebufferResult& result) {
        std::vector<TextureStateSummary> states;
        for (const NativeElanDrawBatch* batch : batches) {
            uint64_t uvVertices = 0u;
            for (const auto& vertex : batch->vertices)
                uvVertices += vertex.hasUv ? 1u : 0u;
            if (uvVertices != 0u) {
                ++result.uvBatches;
                result.uvVertices += uvVertices;
            }
            const uint32_t effectiveTexturePcw = effectivePcw(*batch);
            const uint32_t effectiveTextureTsp =
                effectiveTsp(*batch, batch->tsp0, false);
            const bool environmentMapped =
                environmentMappingSelected(*batch, false);
            const bool pcwTexture = (effectiveTexturePcw & (1u << 3u)) != 0u;
            if (pcwTexture) ++result.texturedPcwBatches;
            if (!pcwTexture || (uvVertices == 0u && !environmentMapped)) continue;
            ++result.textureCandidateBatches;
            const uint32_t materialParams = batch->material.valid
                ? batch->material.words[2] : 0u;
            auto it = std::find_if(states.begin(), states.end(), [&](const auto& state) {
                return state.pcw == effectiveTexturePcw &&
                       state.tsp == effectiveTextureTsp &&
                       state.tcw == batch->tcw0 &&
                       state.materialParams == materialParams;
            });
            if (it == states.end()) {
                states.push_back({effectiveTexturePcw, effectiveTextureTsp,
                                  batch->tcw0,
                                  materialParams, 0u, 0u, 0u});
                it = states.end() - 1;
            }
            ++it->batches;
            it->vertices += batch->vertexCount;
            it->uvVertices += uvVertices;
        }
        result.uniqueTextureStates = static_cast<uint32_t>(states.size());
        const bool trace = textureTraceRequested();
        for (size_t i = 0; i < states.size(); ++i) {
            const auto& state = states[i];
            const uint32_t width = 8u << ((state.tsp >> 3u) & 7u);
            const uint32_t height = 8u << (state.tsp & 7u);
            const uint32_t address = (state.tcw & 0x001FFFFFu) << 3u;
            const bool stride = (state.tcw & (1u << 25u)) != 0u;
            const bool scanOrder = (state.tcw & (1u << 26u)) != 0u;
            const uint32_t pixelFormat = (state.tcw >> 27u) & 7u;
            const bool vq = (state.tcw & (1u << 30u)) != 0u;
            const bool mipmapped = (state.tcw & (1u << 31u)) != 0u;
            const uint32_t bytes = textureStorageBytes(
                width, height, pixelFormat, vq, mipmapped);
            uint32_t nonzero = 0u;
            const uint32_t hash = hashVramRange(vram, address, bytes, nonzero);
            const bool backed = vram && bytes != 0u && address <= vram->size() &&
                                bytes <= vram->size() - address && nonzero != 0u;
            result.vramBackedTextureStates += backed ? 1u : 0u;
            if (!trace || i >= 256u) continue;
            std::fprintf(stderr,
                "[NATIVE-ELAN-TEXTURE-STATE] index=%zu batches=%llu vertices=%llu "
                "uv_vertices=%llu pcw=%08X tsp=%08X tcw=%08X material_params=%08X "
                "size=%ux%u address=%08X bytes=%u format=%u stride=%u scan_order=%u "
                "vq=%u mipmapped=%u vram_backed=%u nonzero=%u hash=%08X\n",
                i, static_cast<unsigned long long>(state.batches),
                static_cast<unsigned long long>(state.vertices),
                static_cast<unsigned long long>(state.uvVertices), state.pcw,
                state.tsp, state.tcw, state.materialParams, width, height,
                address, bytes, pixelFormat, stride ? 1u : 0u,
                scanOrder ? 1u : 0u, vq ? 1u : 0u, mipmapped ? 1u : 0u,
                backed ? 1u : 0u, nonzero, hash);
        }
        if (trace) {
            std::fprintf(stderr,
                "[NATIVE-ELAN-TEXTURE-STATES] accepted_batches=%zu uv_batches=%u "
                "uv_vertices=%llu pcw_textured_batches=%u candidates=%u unique=%u "
                "vram_backed=%u vram_size=%zu\n",
                batches.size(), result.uvBatches,
                static_cast<unsigned long long>(result.uvVertices),
                result.texturedPcwBatches, result.textureCandidateBatches,
                result.uniqueTextureStates, result.vramBackedTextureStates,
                vram ? vram->size() : 0u);
        }
    }

    static Point projectVertex(const NativeElanVertexSample& v,
                               const NativeElanInstanceState& instance,
                               const NativeElanProjectionState& projection,
                               NativeElanFramebufferResult& result) {
        // Flycast constructs the ELAN model-view matrix column-wise as:
        // {-tm00,+tm01,-tm02}, {-tm10,+tm11,-tm12},
        // {-tm20,+tm21,-tm22}, {-tm30,+tm31,-tm32}.
        const auto& m = instance.transform;
        const float eyeX = -(m[0] * v.x + m[3] * v.y + m[6] * v.z + m[9]);
        const float eyeY =   m[1] * v.x + m[4] * v.y + m[7] * v.z + m[10];
        const float eyeZ = -(m[2] * v.x + m[5] * v.y + m[8] * v.z + m[11]);
        const float nearPlane = std::max(instance.nearValue, 1.0e-6f);
        ++result.projectionVertices;
        result.projectionNearMin = std::min(result.projectionNearMin, instance.nearValue);
        result.projectionNearMax = std::max(result.projectionNearMax, instance.nearValue);
        Point point{};
        point.eyeX = eyeX;
        point.eyeY = eyeY;
        point.eyeZ = eyeZ;
        point.u = v.u;
        point.v = v.v;
        point.hasUv = v.hasUv;
        point.finiteEye = std::isfinite(eyeX) && std::isfinite(eyeY) && std::isfinite(eyeZ);
        if (!point.finiteEye) {
            ++result.projectionGuardRejectedVertices;
            return point;
        }
        ++result.projectionFiniteEyeVertices;
        result.projectionEyeZMin = std::min(result.projectionEyeZMin, eyeZ);
        result.projectionEyeZMax = std::max(result.projectionEyeZMax, eyeZ);
        if (eyeZ >= -nearPlane) {
            ++result.projectionNearRejectedVertices;
            return point;
        }

        // Before Flycast's renderer-specific NDC matrix, ELAN projection
        // produces native viewport pixel coordinates directly.
        const float x = projection.tx + projection.fx * eyeX / eyeZ;
        const float y = projection.ty - projection.fy * eyeY / eyeZ;
        // Near-plane clipping can legitimately project an intersection very
        // far outside the viewport before the PVR's later screen clip.  The
        // environment dome at ICH 0103F520, for example, produces finite
        // intersections around 1.6e8 pixels.  Rejecting those vertices here
        // discards every camera-crossing triangle.  Keep only an int-safe
        // sanity bound; fillTriangle clamps the float bounds to the viewport
        // before converting them to raster coordinates.
        constexpr float kCoordinateGuard = 5.0e8f;
        if (!std::isfinite(x) || !std::isfinite(y) ||
            std::fabs(x) > kCoordinateGuard || std::fabs(y) > kCoordinateGuard) {
            ++result.projectionGuardRejectedVertices;
            return point;
        }
        ++result.projectionValidVertices;
        result.projectionScreenXMin = std::min(result.projectionScreenXMin, x);
        result.projectionScreenXMax = std::max(result.projectionScreenXMax, x);
        result.projectionScreenYMin = std::min(result.projectionScreenYMin, y);
        result.projectionScreenYMax = std::max(result.projectionScreenYMax, y);
        // Flycast's wDivide stores 1/w in the post-projection z channel.
        // Here w=-eyeZ, so larger positive values are closer to the camera.
        point.x = x;
        point.y = y;
        point.depth = 1.0f / -eyeZ;
        point.valid = true;
        return point;
    }

    // Read-only eye-space depth extent of a batch, using the same column-wise
    // ELAN model-view convention as projectVertex. Diagnostic only: it lets
    // the owner report state how far a broad batch actually sits from the eye
    // without re-running projection for the frame.
    static float batchEyeZ(const NativeElanDrawBatch& batch,
                           const NativeElanVertexSample& v) {
        const auto& m = batch.instance.transform;
        return -(m[2] * v.x + m[5] * v.y + m[8] * v.z + m[11]);
    }

    static float batchEyeZMin(const NativeElanDrawBatch& batch) {
        if (!batch.instance.valid) return 0.0f;
        float value = std::numeric_limits<float>::infinity();
        for (const auto& v : batch.vertices) {
            const float eyeZ = batchEyeZ(batch, v);
            if (std::isfinite(eyeZ)) value = std::min(value, eyeZ);
        }
        return value;
    }

    static float batchEyeZMax(const NativeElanDrawBatch& batch) {
        if (!batch.instance.valid) return 0.0f;
        float value = -std::numeric_limits<float>::infinity();
        for (const auto& v : batch.vertices) {
            const float eyeZ = batchEyeZ(batch, v);
            if (std::isfinite(eyeZ)) value = std::max(value, eyeZ);
        }
        return value;
    }

    static Point projectEyePoint(float eyeX, float eyeY, float eyeZ,
                                 float u, float v, uint32_t argb, uint32_t offsetArgb,
                                 bool hasUv,
                                 const NativeElanProjectionState& projection) {
        Point point{};
        point.eyeX = eyeX;
        point.eyeY = eyeY;
        point.eyeZ = eyeZ;
        point.u = u;
        point.v = v;
        point.argb = argb;
        point.offsetArgb = offsetArgb;
        point.hasUv = hasUv;
        point.finiteEye = std::isfinite(eyeX) && std::isfinite(eyeY) && std::isfinite(eyeZ);
        if (!point.finiteEye || eyeZ >= 0.0f) return point;
        point.x = projection.tx + projection.fx * eyeX / eyeZ;
        point.y = projection.ty - projection.fy * eyeY / eyeZ;
        // Match projectVertex's int-safe bound.  Near-plane intersections are
        // expected to be far outside the viewport until screen clipping.
        constexpr float kCoordinateGuard = 5.0e8f;
        point.valid = std::isfinite(point.x) && std::isfinite(point.y) &&
                      std::fabs(point.x) <= kCoordinateGuard &&
                      std::fabs(point.y) <= kCoordinateGuard;
        if (point.valid) point.depth = 1.0f / -eyeZ;
        return point;
    }

    static std::vector<Point> clipNearTriangle(
            const Point& a, const Point& b, const Point& c,
            const NativeElanInstanceState& instance,
            const NativeElanProjectionState& projection) {
        if (!a.finiteEye || !b.finiteEye || !c.finiteEye) return {};
        const float nearZ = -std::max(instance.nearValue, 1.0e-6f);
        std::vector<Point> input{a, b, c};
        std::vector<Point> output;
        output.reserve(4u);
        Point previous = input.back();
        bool previousInside = previous.eyeZ <= nearZ;
        for (const Point& current : input) {
            const bool currentInside = current.eyeZ <= nearZ;
            if (currentInside != previousInside) {
                const float denominator = current.eyeZ - previous.eyeZ;
                if (denominator != 0.0f) {
                    const float t = (nearZ - previous.eyeZ) / denominator;
                    output.push_back(projectEyePoint(
                        previous.eyeX + (current.eyeX - previous.eyeX) * t,
                        previous.eyeY + (current.eyeY - previous.eyeY) * t,
                        nearZ,
                        previous.u + (current.u - previous.u) * t,
                        previous.v + (current.v - previous.v) * t,
                        interpolateArgb(previous.argb, current.argb, t),
                        interpolateArgb(previous.offsetArgb, current.offsetArgb, t),
                        previous.hasUv && current.hasUv, projection));
                }
            }
            if (currentInside)
                output.push_back(current.valid
                    ? current
                    : projectEyePoint(current.eyeX, current.eyeY, current.eyeZ,
                                      current.u, current.v, current.argb,
                                      current.offsetArgb,
                                      current.hasUv, projection));
            previous = current;
            previousInside = currentInside;
        }
        if (std::any_of(output.begin(), output.end(),
                        [](const Point& point) { return !point.valid; }))
            return {};
        return output;
    }

    static bool intersectsViewport(const Point& a, const Point& b, const Point& c,
                                   uint32_t width, uint32_t height) {
        const float minX = std::min({a.x, b.x, c.x});
        const float maxX = std::max({a.x, b.x, c.x});
        const float minY = std::min({a.y, b.y, c.y});
        const float maxY = std::max({a.y, b.y, c.y});
        return maxX >= 0.0f && maxY >= 0.0f &&
               minX <= static_cast<float>(width - 1u) &&
               minY <= static_cast<float>(height - 1u);
    }

    static bool sameGeometry(const NativeElanDrawBatch& a, const NativeElanDrawBatch& b) {
        return nativeElanExactSameGeometry(a, b);
    }

    static uint64_t deduplicationHash(const NativeElanDrawBatch& batch,
                                      bool includeRenderState) {
        return nativeElanExactDrawHash(batch, includeRenderState);
    }

    static bool sameRenderState(const NativeElanDrawBatch& a, const NativeElanDrawBatch& b) {
        return nativeElanExactSameRenderState(a, b);
    }

    static void setPixel(std::vector<uint8_t>& rgb, uint32_t width, uint32_t height,
                         int x, int y, const uint8_t color[3]) {
        if (x < 0 || y < 0 || x >= static_cast<int>(width) || y >= static_cast<int>(height)) return;
        const size_t off = (static_cast<size_t>(y) * width + static_cast<uint32_t>(x)) * 3u;
        rgb[off + 0u] = color[0];
        rgb[off + 1u] = color[1];
        rgb[off + 2u] = color[2];
    }

    static float edge(const Point& a, const Point& b, float x, float y) {
        return (x - a.x) * (b.y - a.y) - (y - a.y) * (b.x - a.x);
    }

    static uint32_t interpolateArgb(uint32_t a, uint32_t b, float t) {
        const auto channel = [t](uint32_t lhs, uint32_t rhs, unsigned shift) {
            const float value = static_cast<float>((lhs >> shift) & 0xFFu) +
                                (static_cast<float>((rhs >> shift) & 0xFFu) -
                                 static_cast<float>((lhs >> shift) & 0xFFu)) * t;
            return static_cast<uint32_t>(std::clamp(value, 0.0f, 255.0f) + 0.5f);
        };
        return (channel(a, b, 24u) << 24u) | (channel(a, b, 16u) << 16u) |
               (channel(a, b, 8u) << 8u) | channel(a, b, 0u);
    }

    static uint32_t blendCoefficient(uint8_t instruction, unsigned channel,
                                     const uint8_t source[4],
                                     const uint8_t destination[4],
                                     bool sourceCoefficient) {
        const uint8_t* other = sourceCoefficient ? destination : source;
        switch (instruction & 7u) {
            case 0u: return 0u;
            case 1u: return 255u;
            case 2u: return other[channel];
            case 3u: return 255u - other[channel];
            case 4u: return source[3];
            case 5u: return 255u - source[3];
            case 6u: return destination[3];
            case 7u: return 255u - destination[3];
        }
        return 0u;
    }

    static bool passesDepth(uint8_t mode, float incoming, float stored) {
        switch (mode & 7u) {
            case 0u: return false;
            case 1u: return incoming < stored;
            case 2u: return incoming == stored;
            case 3u: return incoming <= stored;
            case 4u: return incoming > stored;
            case 5u: return incoming != stored;
            case 6u: return incoming >= stored;
            case 7u: return true;
        }
        return false;
    }

    static float fogDensity(const NativePvrFogState& fog) {
        const int exponent = static_cast<int>(
            static_cast<int8_t>(fog.density & 0xFFu));
        const float mantissa =
            static_cast<float>((fog.density >> 8u) & 0xFFu) / 128.0f;
        return std::ldexp(mantissa, exponent);
    }

    static float fogCoefficientWithDensity(const NativePvrFogState& fog,
                                           float depth, float density) {
        if (!fog.valid || !std::isfinite(depth)) return 0.0f;
        const float fogDepth = std::clamp(density * depth, 1.0f, 255.9999f);
        uint32_t fogDepthBits = 0u;
        std::memcpy(&fogDepthBits, &fogDepth, sizeof(fogDepthBits));
        const int exponent = std::clamp(
            static_cast<int>((fogDepthBits >> 23u) & 0xFFu) - 127, 0, 7);
        constexpr float scaleByExponent[8] = {
            16.0f, 8.0f, 4.0f, 2.0f, 1.0f, 0.5f, 0.25f, 0.125f
        };
        const float scaled = fogDepth * scaleByExponent[exponent] - 16.0f;
        const float scaledFloor = std::floor(scaled);
        const int index = std::clamp(
            static_cast<int>(scaledFloor) + exponent * 16, 0, 127);
        const float fraction = scaled - scaledFloor;
        const uint32_t word = fog.table[static_cast<size_t>(index)];
        // Flycast uploads byte 0 and byte 1 as the two rows of a linear
        // 128x2 texture and samples from y=.75 toward y=.25 as m increases.
        const float upper = static_cast<float>((word >> 8u) & 0xFFu);
        const float lower = static_cast<float>(word & 0xFFu);
        return std::clamp(
            (upper + (lower - upper) * fraction) / 255.0f, 0.0f, 1.0f);
    }

    static float fogCoefficient(const NativePvrFogState& fog, float depth) {
        return fogCoefficientWithDensity(fog, depth, fogDensity(fog));
    }

    static uint8_t mixByte(uint8_t source, uint8_t target, float amount) {
        return static_cast<uint8_t>(std::clamp(
            static_cast<float>(source) +
            (static_cast<float>(target) - static_cast<float>(source)) * amount,
            0.0f, 255.0f) + 0.5f);
    }

    static void applyModifierDepthPass(
            uint8_t& stencil, bool depthPass, bool useOr) {
        if (!depthPass) return;
        if (useOr)
            stencil = static_cast<uint8_t>(stencil | 0x02u);
        else
            stencil = static_cast<uint8_t>(stencil ^ 0x02u);
    }

    static void finalizeModifierStencil(uint8_t& stencil, uint8_t mode) {
        const uint8_t low = static_cast<uint8_t>(stencil & 0x03u);
        uint8_t result = low;
        if (mode == 1u)
            result = low != 0u ? 0x01u : 0u;
        else if (mode == 2u)
            result = low == 0x01u ? 0x01u : 0u;
        stencil = static_cast<uint8_t>((stencil & ~0x03u) | result);
    }

    static uint8_t scaleModifierShadowChannel(uint8_t color, uint8_t scale) {
        // Flycast's final black-alpha blend is algebraically dst*scale/256.
        return static_cast<uint8_t>(
            (static_cast<uint32_t>(color) * scale + 128u) / 256u);
    }

    static bool punchThroughAlphaPass(uint8_t alpha, uint32_t alphaReference) {
        // Flycast rounds the shader alpha to an 8-bit step and discards only
        // when PT_ALPHA_REF is greater, so equality passes.
        return alpha >= static_cast<uint8_t>(alphaReference & 0xFFu);
    }

    struct TileClipRaster {
        int minX = 0;
        int minY = 0;
        int maxXExclusive = 0;
        int maxYExclusive = 0;
        bool discardInside = false;
    };

    static bool prepareTileClip(uint32_t packed, uint32_t width, uint32_t height,
                                int& minX, int& maxX, int& minY, int& maxY,
                                TileClipRaster& clip) {
        const uint32_t mode = packed >> 28u;
        if (mode < 2u) return true;

        clip.minX = static_cast<int>((packed & 63u) * 32u);
        clip.maxXExclusive = static_cast<int>((((packed >> 6u) & 63u) + 1u) * 32u);
        clip.minY = static_cast<int>(((packed >> 12u) & 31u) * 32u);
        clip.maxYExclusive = static_cast<int>((((packed >> 17u) & 31u) + 1u) * 32u);
        clip.discardInside = (mode & 1u) != 0u;

        // A full-screen rectangle is equivalent to clipping disabled. This
        // is both the hardware result and the common fast path.
        if (clip.minX <= 0 && clip.minY <= 0 &&
            clip.maxXExclusive >= static_cast<int>(width) &&
            clip.maxYExclusive >= static_cast<int>(height))
            return true;

        if (!clip.discardInside) {
            // Mode 2 draws only inside the rectangle. Intersect the triangle's
            // scan bounds up front so clipped pixels never enter the hot loop.
            minX = std::max(minX, clip.minX);
            maxX = std::min(maxX, clip.maxXExclusive - 1);
            minY = std::max(minY, clip.minY);
            maxY = std::min(maxY, clip.maxYExclusive - 1);
        }
        return minX <= maxX && minY <= maxY;
    }

    static bool tileClipReject(const TileClipRaster& clip, int x, int y) {
        if (!clip.discardInside) return false;
        return x >= clip.minX && x < clip.maxXExclusive &&
               y >= clip.minY && y < clip.maxYExclusive;
    }

    static uint32_t rasterizeModifierDepth(
            uint32_t width, uint32_t height,
            const Point& a, const Point& b, const Point& c,
            const std::vector<float>& depth, std::vector<uint8_t>& stencil,
            const RasterState& rasterState, bool useOr) {
        const float area = edge(a, b, c.x, c.y);
        if (std::fabs(area) < 1.0e-12f ||
            triangleCulled(a, b, c, rasterState))
            return 0u;
        const float viewportMaxX = static_cast<float>(width - 1u);
        const float viewportMaxY = static_cast<float>(height - 1u);
        int minX = static_cast<int>(std::floor(std::clamp(
            std::min({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        int maxX = static_cast<int>(std::ceil(std::clamp(
            std::max({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        int minY = static_cast<int>(std::floor(std::clamp(
            std::min({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        int maxY = static_cast<int>(std::ceil(std::clamp(
            std::max({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        TileClipRaster tileClip{};
        if (!prepareTileClip(rasterState.tileClip, width, height,
                             minX, maxX, minY, maxY, tileClip))
            return 0u;
        const bool positive = area > 0.0f;
        uint32_t touched = 0u;
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                const float px = static_cast<float>(x) + 0.5f;
                const float py = static_cast<float>(y) + 0.5f;
                const float e0 = edge(a, b, px, py);
                const float e1 = edge(b, c, px, py);
                const float e2 = edge(c, a, px, py);
                const bool inside = positive
                    ? (e0 >= 0.0f && e1 >= 0.0f && e2 >= 0.0f)
                    : (e0 <= 0.0f && e1 <= 0.0f && e2 <= 0.0f);
                if (!inside) continue;
                if (tileClipReject(tileClip, x, y)) continue;
                const float wa = e1 / area;
                const float wb = e2 / area;
                const float wc = e0 / area;
                const float z = wa * a.depth + wb * b.depth + wc * c.depth;
                const size_t index = static_cast<size_t>(y) * width +
                                     static_cast<uint32_t>(x);
                const bool depthPass = std::isfinite(z) &&
                    index < depth.size() && z > depth[index];
                applyModifierDepthPass(stencil[index], depthPass, useOr);
                touched += depthPass ? 1u : 0u;
            }
        }
        return touched;
    }

    static uint32_t rasterizeModifierFinalize(
            uint32_t width, uint32_t height,
            const Point& a, const Point& b, const Point& c,
            std::vector<uint8_t>& stencil, const RasterState& rasterState,
            uint8_t mode) {
        const float area = edge(a, b, c.x, c.y);
        if (std::fabs(area) < 1.0e-12f ||
            triangleCulled(a, b, c, rasterState))
            return 0u;
        const float viewportMaxX = static_cast<float>(width - 1u);
        const float viewportMaxY = static_cast<float>(height - 1u);
        int minX = static_cast<int>(std::floor(std::clamp(
            std::min({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        int maxX = static_cast<int>(std::ceil(std::clamp(
            std::max({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        int minY = static_cast<int>(std::floor(std::clamp(
            std::min({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        int maxY = static_cast<int>(std::ceil(std::clamp(
            std::max({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        TileClipRaster tileClip{};
        if (!prepareTileClip(rasterState.tileClip, width, height,
                             minX, maxX, minY, maxY, tileClip))
            return 0u;
        const bool positive = area > 0.0f;
        uint32_t covered = 0u;
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                const float px = static_cast<float>(x) + 0.5f;
                const float py = static_cast<float>(y) + 0.5f;
                const float e0 = edge(a, b, px, py);
                const float e1 = edge(b, c, px, py);
                const float e2 = edge(c, a, px, py);
                const bool inside = positive
                    ? (e0 >= 0.0f && e1 >= 0.0f && e2 >= 0.0f)
                    : (e0 <= 0.0f && e1 <= 0.0f && e2 <= 0.0f);
                if (!inside) continue;
                if (tileClipReject(tileClip, x, y)) continue;
                const size_t index = static_cast<size_t>(y) * width +
                                     static_cast<uint32_t>(x);
                finalizeModifierStencil(stencil[index], mode);
                ++covered;
            }
        }
        return covered;
    }

    static uint64_t applyModifierShadow(
            std::vector<uint8_t>& rgb, uint32_t width, uint32_t height,
            std::vector<uint8_t>& stencil, uint8_t scale) {
        const size_t pixels = std::min(
            static_cast<size_t>(width) * height, stencil.size());
        uint64_t affected = 0u;
        for (size_t pixel = 0u; pixel < pixels; ++pixel) {
            if ((stencil[pixel] & 0x81u) == 0x81u) {
                const size_t color = pixel * 3u;
                rgb[color + 0u] = scaleModifierShadowChannel(rgb[color + 0u], scale);
                rgb[color + 1u] = scaleModifierShadowChannel(rgb[color + 1u], scale);
                rgb[color + 2u] = scaleModifierShadowChannel(rgb[color + 2u], scale);
                ++affected;
            }
            // Flycast's final pass clears only the two modifier result bits.
            stencil[pixel] = static_cast<uint8_t>(stencil[pixel] & ~0x03u);
        }
        return affected;
    }

    static uint32_t fillTriangle(std::vector<uint8_t>& rgb,
                                 std::vector<uint8_t>& alpha,
                                 std::vector<uint8_t>& secondaryRgb,
                                 std::vector<uint8_t>& secondaryAlpha,
                                 uint32_t width, uint32_t height,
                                 const Point& a, const Point& b, const Point& c,
                                 std::vector<float>* depth,
                                 const TextureBinding* texture,
                                 const RasterState& rasterState,
                                 const NativePvrFogState& fog,
                                 std::vector<uint8_t>* stencil,
                                 uint64_t* punchAlphaTestedPixels,
                                 uint64_t* punchAlphaRejectedPixels,
                                 std::vector<uint32_t>* pixelOwners,
                                 uint32_t batchIndex) {
        const float area = edge(a, b, c.x, c.y);
        if (std::fabs(area) < 1.0e-12f) return 0u;
        if (triangleCulled(a, b, c, rasterState)) return 0u;
        // Clamp while still in floating point.  A near-clipped vertex may be
        // hundreds of millions of pixels off-screen; converting that value
        // before the viewport clamp needlessly depends on host int range.
        const float viewportMaxX = static_cast<float>(width - 1u);
        const float viewportMaxY = static_cast<float>(height - 1u);
        int minX = static_cast<int>(std::floor(std::clamp(
            std::min({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        int maxX = static_cast<int>(std::ceil(std::clamp(
            std::max({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        int minY = static_cast<int>(std::floor(std::clamp(
            std::min({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        int maxY = static_cast<int>(std::ceil(std::clamp(
            std::max({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        TileClipRaster tileClip{};
        if (!prepareTileClip(rasterState.tileClip, width, height,
                             minX, maxX, minY, maxY, tileClip))
            return 0u;
        const bool positive = area > 0.0f;
        const bool usesTableFog = fog.valid &&
            (rasterState.fogMode == 0u || rasterState.fogMode == 3u);
        const float tableFogDensity = usesTableFog ? fogDensity(fog) : 0.0f;
        const float inverseArea = 1.0f / area;
        const float e0StepX = b.y - a.y;
        const float e1StepX = c.y - b.y;
        const float e2StepX = a.y - c.y;
        uint32_t written = 0u;
        for (int y = minY; y <= maxY; ++y) {
            const float firstX = static_cast<float>(minX) + 0.5f;
            const float sampleY = static_cast<float>(y) + 0.5f;
            float scanE0 = edge(a, b, firstX, sampleY);
            float scanE1 = edge(b, c, firstX, sampleY);
            float scanE2 = edge(c, a, firstX, sampleY);
            for (int x = minX; x <= maxX; ++x) {
                const float e0 = scanE0;
                const float e1 = scanE1;
                const float e2 = scanE2;
                // Advance before any early continue below. Edge functions are
                // affine in X, so this replaces six multiplies per sample with
                // three additions while preserving the same triangle test.
                scanE0 += e0StepX;
                scanE1 += e1StepX;
                scanE2 += e2StepX;
                const bool inside = positive ? (e0 >= 0.0f && e1 >= 0.0f && e2 >= 0.0f)
                                             : (e0 <= 0.0f && e1 <= 0.0f && e2 <= 0.0f);
                if (!inside) continue;
                if (tileClipReject(tileClip, x, y)) continue;
                const float wa = e1 * inverseArea;
                const float wb = e2 * inverseArea;
                const float wc = e0 * inverseArea;
                const float z = wa * a.depth + wb * b.depth + wc * c.depth;
                const size_t index = static_cast<size_t>(y) * width +
                                     static_cast<uint32_t>(x);
                if (depth) {
                    if (!std::isfinite(z) ||
                        !passesDepth(rasterState.depthMode, z, (*depth)[index]))
                        continue;
                }

                const auto vertexChannel = [&](unsigned shift) {
                    if (!rasterState.gouraud)
                        return static_cast<int>((c.argb >> shift) & 0xFFu);
                    return static_cast<int>(std::clamp(
                        wa * static_cast<float>((a.argb >> shift) & 0xFFu) +
                        wb * static_cast<float>((b.argb >> shift) & 0xFFu) +
                        wc * static_cast<float>((c.argb >> shift) & 0xFFu),
                        0.0f, 255.0f) + 0.5f);
                };
                int vertexA = rasterState.useAlpha ? vertexChannel(24u) : 255;
                int vertexR = vertexChannel(16u);
                int vertexG = vertexChannel(8u);
                int vertexB = vertexChannel(0u);
                const auto offsetChannel = [&](unsigned shift) {
                    if (!rasterState.gouraud)
                        return static_cast<int>((c.offsetArgb >> shift) & 0xFFu);
                    return static_cast<int>(std::clamp(
                        wa * static_cast<float>((a.offsetArgb >> shift) & 0xFFu) +
                        wb * static_cast<float>((b.offsetArgb >> shift) & 0xFFu) +
                        wc * static_cast<float>((c.offsetArgb >> shift) & 0xFFu),
                        0.0f, 255.0f) + 0.5f);
                };
                const int offsetR = rasterState.offset ? offsetChannel(16u) : 0;
                const int offsetG = rasterState.offset ? offsetChannel(8u) : 0;
                const int offsetB = rasterState.offset ? offsetChannel(0u) : 0;
                const int offsetA = rasterState.fogMode == 1u
                    ? offsetChannel(24u) : 0;
                const float fogAmount = usesTableFog
                    ? fogCoefficientWithDensity(fog, z, tableFogDensity) : 0.0f;
                if (rasterState.fogMode == 3u && fog.valid) {
                    vertexA = static_cast<int>(fogAmount * 255.0f + 0.5f);
                    vertexR = static_cast<int>((fog.ramColor >> 16u) & 0xFFu);
                    vertexG = static_cast<int>((fog.ramColor >> 8u) & 0xFFu);
                    vertexB = static_cast<int>(fog.ramColor & 0xFFu);
                }
                uint32_t shaded =
                    (static_cast<uint32_t>(rasterState.useAlpha ? vertexA : 255) << 24u) |
                    (static_cast<uint32_t>(vertexR) << 16u) |
                    (static_cast<uint32_t>(vertexG) << 8u) |
                    static_cast<uint32_t>(vertexB);
                if (texture) {
                    float u = wa * a.u + wb * b.u + wc * c.u;
                    float v = wa * a.v + wb * b.v + wc * c.v;
                    if (std::isfinite(z) && z != 0.0f) {
                        u = (wa * a.u * a.depth + wb * b.u * b.depth +
                             wc * c.u * c.depth) / z;
                        v = (wa * a.v * a.depth + wb * b.v * b.depth +
                             wc * c.v * c.depth) / z;
                    }
                    uint32_t texel = sampleTexture(*texture, u, v);
                    if (texture->ignoreAlpha) texel |= 0xFF000000u;
                    shaded = applyTexEnv(
                        texture->environment,
                        rasterState.useAlpha ? vertexA : 255,
                        vertexR, vertexG, vertexB, texel);
                    if (rasterState.offset) {
                        shaded = (shaded & 0xFF000000u) |
                            (static_cast<uint32_t>(std::min<int>(
                                static_cast<int>((shaded >> 16u) & 0xFFu) + offsetR, 255)) << 16u) |
                            (static_cast<uint32_t>(std::min<int>(
                                static_cast<int>((shaded >> 8u) & 0xFFu) + offsetG, 255)) << 8u) |
                            static_cast<uint32_t>(std::min<int>(
                                static_cast<int>(shaded & 0xFFu) + offsetB, 255));
                    }
                }

                uint8_t shadedChannels[4] = {
                    static_cast<uint8_t>((shaded >> 16u) & 0xFFu),
                    static_cast<uint8_t>((shaded >> 8u) & 0xFFu),
                    static_cast<uint8_t>(shaded & 0xFFu),
                    static_cast<uint8_t>((shaded >> 24u) & 0xFFu)
                };
                if (rasterState.colorClamp && fog.valid) {
                    const uint8_t minimum[4] = {
                        static_cast<uint8_t>((fog.clampMin >> 16u) & 0xFFu),
                        static_cast<uint8_t>((fog.clampMin >> 8u) & 0xFFu),
                        static_cast<uint8_t>(fog.clampMin & 0xFFu),
                        static_cast<uint8_t>((fog.clampMin >> 24u) & 0xFFu)
                    };
                    const uint8_t maximum[4] = {
                        static_cast<uint8_t>((fog.clampMax >> 16u) & 0xFFu),
                        static_cast<uint8_t>((fog.clampMax >> 8u) & 0xFFu),
                        static_cast<uint8_t>(fog.clampMax & 0xFFu),
                        static_cast<uint8_t>((fog.clampMax >> 24u) & 0xFFu)
                    };
                    for (unsigned channel = 0u; channel < 4u; ++channel)
                        shadedChannels[channel] = std::clamp(
                            shadedChannels[channel], minimum[channel], maximum[channel]);
                }
                if (rasterState.fogMode == 0u && fog.valid) {
                    shadedChannels[0] = mixByte(shadedChannels[0],
                        static_cast<uint8_t>((fog.ramColor >> 16u) & 0xFFu), fogAmount);
                    shadedChannels[1] = mixByte(shadedChannels[1],
                        static_cast<uint8_t>((fog.ramColor >> 8u) & 0xFFu), fogAmount);
                    shadedChannels[2] = mixByte(shadedChannels[2],
                        static_cast<uint8_t>(fog.ramColor & 0xFFu), fogAmount);
                } else if (rasterState.fogMode == 1u && fog.valid) {
                    const float vertexFog = static_cast<float>(offsetA) / 255.0f;
                    shadedChannels[0] = mixByte(shadedChannels[0],
                        static_cast<uint8_t>((fog.vertexColor >> 16u) & 0xFFu), vertexFog);
                    shadedChannels[1] = mixByte(shadedChannels[1],
                        static_cast<uint8_t>((fog.vertexColor >> 8u) & 0xFFu), vertexFog);
                    shadedChannels[2] = mixByte(shadedChannels[2],
                        static_cast<uint8_t>(fog.vertexColor & 0xFFu), vertexFog);
                }

                if (rasterState.punchAlphaTest) {
                    if (punchAlphaTestedPixels) ++*punchAlphaTestedPixels;
                    if (!punchThroughAlphaPass(
                            shadedChannels[3], fog.punchAlphaRef)) {
                        if (punchAlphaRejectedPixels) ++*punchAlphaRejectedPixels;
                        continue;
                    }
                    // Flycast forces passing punch-through fragments opaque.
                    shadedChannels[3] = 0xFFu;
                }

                const size_t colorOffset = index * 3u;
                uint8_t shadedSource[4] = {
                    shadedChannels[0], shadedChannels[1],
                    shadedChannels[2], shadedChannels[3]
                };
                uint8_t secondarySource[4] = {
                    secondaryRgb[colorOffset + 0u],
                    secondaryRgb[colorOffset + 1u],
                    secondaryRgb[colorOffset + 2u],
                    secondaryAlpha[index]
                };
                const uint8_t* source = rasterState.srcSelect
                    ? secondarySource : shadedSource;
                std::vector<uint8_t>& destinationRgb = rasterState.dstSelect
                    ? secondaryRgb : rgb;
                std::vector<uint8_t>& destinationAlpha = rasterState.dstSelect
                    ? secondaryAlpha : alpha;
                uint8_t destination[4] = {
                    destinationRgb[colorOffset + 0u],
                    destinationRgb[colorOffset + 1u],
                    destinationRgb[colorOffset + 2u],
                    destinationAlpha[index]
                };
                uint8_t output[4];
                if (rasterState.srcBlend == 1u && rasterState.dstBlend == 0u) {
                    // ONE/ZERO is an exact source copy. This is the dominant
                    // opaque PVR state and the general integer blend equation
                    // produces these same four bytes exactly.
                    output[0] = source[0];
                    output[1] = source[1];
                    output[2] = source[2];
                    output[3] = source[3];
                } else {
                    for (unsigned channel = 0u; channel < 4u; ++channel) {
                        const uint32_t srcCoef = blendCoefficient(
                            rasterState.srcBlend, channel, source, destination, true);
                        const uint32_t dstCoef = blendCoefficient(
                            rasterState.dstBlend, channel, source, destination, false);
                        output[channel] = static_cast<uint8_t>(std::min<uint32_t>(
                            (static_cast<uint32_t>(source[channel]) * srcCoef +
                             static_cast<uint32_t>(destination[channel]) * dstCoef + 127u) / 255u,
                            255u));
                    }
                }
                if (depth && rasterState.depthWrite) (*depth)[index] = z;
                if (stencil &&
                    (rasterState.listType == 0u || rasterState.listType == 4u)) {
                    (*stencil)[index] = static_cast<uint8_t>(
                        ((*stencil)[index] & 0x7Fu) |
                        (rasterState.shadowed ? 0x80u : 0u));
                }
                destinationRgb[colorOffset + 0u] = output[0];
                destinationRgb[colorOffset + 1u] = output[1];
                destinationRgb[colorOffset + 2u] = output[2];
                destinationAlpha[index] = output[3];
                const bool changedDestination = output[0] != destination[0] ||
                                                output[1] != destination[1] ||
                                                output[2] != destination[2] ||
                                                output[3] != destination[3];
                if (pixelOwners && !rasterState.dstSelect && changedDestination)
                    (*pixelOwners)[index] = batchIndex;
                ++written;
            }
        }
        return written;
    }

    static void drawLine(std::vector<uint8_t>& rgb, uint32_t width, uint32_t height,
                         const Point& a, const Point& b, const uint8_t color[3]) {
        float x0f = a.x;
        float y0f = a.y;
        float x1f = b.x;
        float y1f = b.y;
        const float dxFloat = x1f - x0f;
        const float dyFloat = y1f - y0f;
        float t0 = 0.0f;
        float t1 = 1.0f;
        const auto clip = [&t0, &t1](float p, float q) {
            if (p == 0.0f) return q >= 0.0f;
            const float r = q / p;
            if (p < 0.0f) {
                if (r > t1) return false;
                t0 = std::max(t0, r);
            } else {
                if (r < t0) return false;
                t1 = std::min(t1, r);
            }
            return true;
        };
        if (!clip(-dxFloat, x0f) ||
            !clip(dxFloat, static_cast<float>(width - 1u) - x0f) ||
            !clip(-dyFloat, y0f) ||
            !clip(dyFloat, static_cast<float>(height - 1u) - y0f))
            return;
        x1f = x0f + t1 * dxFloat;
        y1f = y0f + t1 * dyFloat;
        x0f += t0 * dxFloat;
        y0f += t0 * dyFloat;
        int x0 = static_cast<int>(std::lround(x0f));
        int y0 = static_cast<int>(std::lround(y0f));
        const int x1 = static_cast<int>(std::lround(x1f));
        const int y1 = static_cast<int>(std::lround(y1f));
        const int dx = std::abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -std::abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            setPixel(rgb, width, height, x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            const int e2 = err * 2;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    static void writeLe16(std::ofstream& out, uint16_t v) {
        const uint8_t b[2] = {static_cast<uint8_t>(v), static_cast<uint8_t>(v >> 8u)};
        out.write(reinterpret_cast<const char*>(b), 2);
    }

    static void writeLe32(std::ofstream& out, uint32_t v) {
        const uint8_t b[4] = {static_cast<uint8_t>(v), static_cast<uint8_t>(v >> 8u),
                              static_cast<uint8_t>(v >> 16u), static_cast<uint8_t>(v >> 24u)};
        out.write(reinterpret_cast<const char*>(b), 4);
    }

    static bool writeBmp(const std::string& path, uint32_t width, uint32_t height,
                         const std::vector<uint8_t>& rgb) {
        const uint32_t rowBytes = width * 3u;
        const uint32_t stride = (rowBytes + 3u) & ~3u;
        const uint32_t pixelBytes = stride * height;
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out.put('B'); out.put('M');
        writeLe32(out, 54u + pixelBytes);
        writeLe16(out, 0u); writeLe16(out, 0u);
        writeLe32(out, 54u);
        writeLe32(out, 40u);
        writeLe32(out, width); writeLe32(out, height);
        writeLe16(out, 1u); writeLe16(out, 24u);
        writeLe32(out, 0u); writeLe32(out, pixelBytes);
        writeLe32(out, 2835u); writeLe32(out, 2835u);
        writeLe32(out, 0u); writeLe32(out, 0u);
        const uint8_t padding[3] = {0u, 0u, 0u};
        for (uint32_t row = 0; row < height; ++row) {
            const uint32_t y = height - 1u - row;
            for (uint32_t x = 0; x < width; ++x) {
                const size_t off = (static_cast<size_t>(y) * width + x) * 3u;
                const uint8_t bgr[3] = {rgb[off + 2u], rgb[off + 1u], rgb[off + 0u]};
                out.write(reinterpret_cast<const char*>(bgr), 3);
            }
            out.write(reinterpret_cast<const char*>(padding), stride - rowBytes);
        }
        return static_cast<bool>(out);
    }
};
