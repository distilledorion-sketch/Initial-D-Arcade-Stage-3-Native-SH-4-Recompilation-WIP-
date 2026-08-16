#pragma once

#include "native_elan_decode.h"
#include "vram_texture.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

// Diagnostic host framebuffer for frame-final ELAN scenes. Fit-to-view remains
// the default. IDAS3_NATIVE_FRAMEBUFFER_PROJECTION opts into the exact NAOMI 2
// instance/projection transform proven against Flycast's ELAN implementation.
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
    uint32_t projectionRejectedBatches = 0;
    uint32_t projectionRejectedTriangles = 0;
    uint32_t projectionVertices = 0;
    uint32_t projectionFiniteEyeVertices = 0;
    uint32_t projectionNearRejectedVertices = 0;
    uint32_t projectionGuardRejectedVertices = 0;
    uint32_t projectionValidVertices = 0;
    uint32_t projectionViewportRejectedTriangles = 0;
    uint32_t projectionNearClippedTriangles = 0;
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
    uint32_t lightModeledBatches = 0;
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
    static NativeElanFramebufferImage renderLatestSceneRgb(
        const std::vector<NativeElanFrameScene>& scenes,
        uint32_t width = 640u, uint32_t height = 480u,
        const std::vector<uint8_t>* naomi2Vram = nullptr) {
        NativeElanFramebufferImage image{};
        auto& result = image.result;
        result.width = width;
        result.height = height;
        const bool useCapturedProjection = capturedProjectionRequested();
        const bool traceLighting = lightingTraceRequested();
        const char* ownerBmpPath = pixelOwnerBmpPathRequested();
        const bool tracePixelOwners = pixelOwnerTraceRequested() || ownerBmpPath != nullptr;
        uint32_t tracedLightingBatches = 0u;
        result.projectionMode = useCapturedProjection ? 1u : 0u;
        if (scenes.empty() || width < 32u || height < 32u ||
            width > 4096u || height > 4096u)
            return image;

        const NativeElanFrameScene& scene = scenes.back();
        result.sceneFrame = scene.frame;
        std::vector<const NativeElanDrawBatch*> batches;
        for (const auto& draw : scene.draws) {
            const auto& batch = draw.batch;
            if (batch.vertexCount < 3u || batch.vertices.size() != batch.vertexCount ||
                batch.finiteVertices != batch.vertexCount) {
                ++result.rejectedBatches;
                continue;
            }
            if (useCapturedProjection && (!batch.projection.valid || !batch.instance.valid)) {
                ++result.rejectedBatches;
                ++result.projectionRejectedBatches;
                continue;
            }
            if (batchDiagnosticallyExcluded(batch.ichOffset)) {
                ++result.diagnosticExcludedBatches;
                continue;
            }
            if (useCapturedProjection && nearFarCullEnabled() &&
                !batchBetweenNearAndFar(batch)) {
                ++result.rejectedBatches;
                ++result.nearFarCulledBatches;
                continue;
            }
            bool duplicate = false;
            for (const NativeElanDrawBatch* prior : batches) {
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
            batches.push_back(&batch);
        }
        result.acceptedBatches = static_cast<uint32_t>(batches.size());
        if (batches.empty()) return image;

        traceTextureStates(batches, naomi2Vram, result);

        // ELAN command submission may interleave TA list types, but the PVR
        // renders them as separate lists. Preserve order within each list and
        // execute the hardware list order so translucent environment passes
        // cannot pre-fill depth ahead of opaque body geometry.
        const auto listOrder = [](const NativeElanDrawBatch* batch) {
            switch ((batch->pcw >> 24u) & 7u) {
                case 0u: return 0u; // opaque
                case 1u: return 1u; // opaque modifier volume
                case 4u: return 2u; // punch-through
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
                const auto same = std::find_if(uniqueStates.begin(), uniqueStates.end(),
                    [&](const NativeElanDrawBatch* prior) {
                        return prior->projection.fx == batch->projection.fx &&
                               prior->projection.tx == batch->projection.tx &&
                               prior->projection.fy == batch->projection.fy &&
                               prior->projection.ty == batch->projection.ty &&
                               prior->instance.nearValue == batch->instance.nearValue &&
                               prior->instance.farValue == batch->instance.farValue &&
                               prior->instance.inverseNear == batch->instance.inverseNear &&
                               prior->instance.transform == batch->instance.transform;
                    });
                if (same != uniqueStates.end()) continue;
                uniqueStates.push_back(batch);
                if (uniqueStates.size() > 16u) continue;
                const auto& m = batch->instance.transform;
                std::fprintf(stderr,
                    "[NATIVE-ELAN-PROJECTION-STATE] index=%zu ich=%08X instance=%08X "
                    "projection=%08X guest_sequence=%llu guest_outer_pr=%08X "
                    "near=%.9g far=%.9g inv_near=%.9g "
                    "matrix=%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g "
                    "proj=%.9g,%.9g,%.9g,%.9g\n",
                    uniqueStates.size() - 1u, batch->ichOffset, batch->instance.offset,
                    batch->projection.offset,
                    static_cast<unsigned long long>(batch->instance.guestSubmitSequence),
                    batch->instance.guestOuterPr, batch->instance.nearValue,
                    batch->instance.farValue, batch->instance.inverseNear,
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
        if (!useCapturedProjection) {
            minX = std::numeric_limits<float>::infinity();
            minY = std::numeric_limits<float>::infinity();
            maxX = -std::numeric_limits<float>::infinity();
            maxY = -std::numeric_limits<float>::infinity();
        }
        for (const auto* batch : batches) {
            result.vertices += batch->vertexCount;
            if (!useCapturedProjection) {
                for (const auto& v : batch->vertices) {
                    minX = std::min(minX, v.x);
                    minY = std::min(minY, v.y);
                    maxX = std::max(maxX, v.x);
                    maxY = std::max(maxY, v.y);
                }
            }
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
        std::vector<TextureCacheEntry> textureCache;
        textureCache.reserve(result.uniqueTextureStates);
        for (size_t bi = 0; bi < batches.size(); ++bi) {
            const auto& batch = *batches[bi];
            const auto& vertices = batch.vertices;
            // ELAN Model command state, applied exactly as Flycast's
            // setStateParams does: the model TSP is XORed into the polygon
            // TSP and the ISP cull mode is XORed with cullingReversed and the
            // left-handed projection flip.
            const uint32_t effectiveTsp0 = effectiveTsp(batch, batch.tsp0);
            const RasterState rasterState =
                decodeRasterState(batch.pcw, effectiveIspTsp(batch), effectiveTsp0);
            const uint64_t pixelsBeforeBatch = result.texturedPixels;
            const uint8_t* fallback = palette[bi % (sizeof(palette) / sizeof(palette[0]))];
            if (batch.lightModel.valid) ++result.lightModeledBatches;
            const bool environmentMapped = batch.material.valid &&
                (batch.material.words[2] & (1u << 11u)) != 0u && batch.instance.valid;
            const bool batchHasUv = std::any_of(vertices.begin(), vertices.end(),
                [](const NativeElanVertexSample& vertex) { return vertex.hasUv; }) ||
                environmentMapped;
            const bool batchTextured = (batch.pcw & (1u << 3u)) != 0u && batchHasUv;
            const TextureBinding* textureBinding = nullptr;
            TextureCacheEntry* textureEntry = nullptr;
            if (batchTextured) {
                auto cached = std::find_if(textureCache.begin(), textureCache.end(),
                    [&](const TextureCacheEntry& entry) {
                        return entry.tsp == effectiveTsp0 && entry.tcw == batch.tcw0;
                    });
                if (cached == textureCache.end()) {
                    textureCache.push_back({effectiveTsp0, batch.tcw0,
                        decodeTextureBinding(effectiveTsp0, batch.tcw0, naomi2Vram)});
                    cached = textureCache.end() - 1;
                    if (cached->binding.valid) ++result.decodedTextureStates;
                }
                textureEntry = &*cached;
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
                    Point point = projectVertex(v, batch.instance, batch.projection, result);
                    if (environmentMapped)
                        applyEnvironmentMap(batch.instance, v, point.u, point.v, point.hasUv);
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
                        applyEnvironmentMap(batch.instance, v, point.u, point.v, point.hasUv);
                    point.valid = true;
                    points.push_back(point);
                }
            }
            if (useCapturedProjection) ++result.projectedBatches;
            dumpBatchGeometry(bi, batch, points, rasterState);
            uint32_t batchTriangles = 0u;
            uint32_t batchCulled = 0u;
            uint32_t batchDrawn = 0u;
            const auto renderTriangle = [&](size_t a, size_t b, size_t c) {
                ++batchTriangles;
                if (useCapturedProjection &&
                    (!points[a].valid || !points[b].valid || !points[c].valid)) {
                    const auto clipped = clipNearTriangle(
                        points[a], points[b], points[c], batch.instance, batch.projection);
                    if (clipped.size() >= 3u) {
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
                            result.texturedPixels += fillTriangle(
                                rgb, alpha, secondaryRgb, secondaryAlpha,
                                width, height, clipped[0], clipped[ci],
                                clipped[ci + 1u], &depth, triangleTexture, rasterState,
                                tracePixelOwners ? &pixelOwners : nullptr,
                                static_cast<uint32_t>(bi));
                            ++result.triangles;
                        }
                        return;
                    }
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
                result.texturedPixels += fillTriangle(
                    rgb, alpha, secondaryRgb, secondaryAlpha,
                    width, height, points[a], points[b], points[c],
                    useCapturedProjection ? &depth : nullptr, triangleTexture,
                    rasterState, tracePixelOwners ? &pixelOwners : nullptr,
                    static_cast<uint32_t>(bi));
                if (!useCapturedProjection) {
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
            const uint64_t batchPixels = result.texturedPixels - pixelsBeforeBatch;
            if (textureEntry) textureEntry->rasterPixels += batchPixels;
            if (cullTraceRequested())
                std::fprintf(stderr,
                    "[NATIVE-ELAN-CULL] batch=%zu ich=%08X list=%u pcw=%08X isp=%08X "
                    "tsp=%08X tcw=%08X textured=%u material=%08X params=%08X "
                    "cull=%u depth_mode=%u depth_write=%u triangles=%u "
                    "drawn=%u culled=%u pixels=%llu\n",
                    bi, batch.ichOffset,
                    static_cast<unsigned>((batch.pcw >> 24u) & 7u),
                    batch.pcw, batch.ispTsp, effectiveTsp0, batch.tcw0,
                    textureBinding ? 1u : 0u, batch.material.offset,
                    batch.material.valid ? batch.material.words[2] : 0u,
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
                ++tracedLightingBatches;
            }
        }

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
        bool useAlpha = false;
        bool offset = false;
        bool srcSelect = false;
        bool dstSelect = false;
        uint8_t srcBlend = 1u;
        uint8_t dstBlend = 0u;
        uint8_t cullMode = 0u;
        uint8_t depthMode = 6u;
        bool depthWrite = true;
    };

    struct TextureCacheEntry {
        uint32_t tsp = 0u;
        uint32_t tcw = 0u;
        TextureBinding binding{};
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
        u = std::clamp(vertex.u + normal[0] * 0.5f + 0.5f, 0.0f, 1.0f);
        v = std::clamp(vertex.v + normal[1] * 0.5f + 0.5f, 0.0f, 1.0f);
        hasUv = true;
    }

    static bool capturedProjectionRequested() {
        const char* value = std::getenv("IDAS3_NATIVE_FRAMEBUFFER_PROJECTION");
        return value && *value && std::strcmp(value, "0") != 0;
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

    static uint32_t effectiveTsp(const NativeElanDrawBatch& batch, uint32_t tsp) {
        if (!modelStateEnabled() || !batch.model.valid) return tsp;
        return tsp ^ batch.model.tsp;
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

    static bool batchBetweenNearAndFar(const NativeElanDrawBatch& batch) {
        if (!batch.instance.valid || batch.vertices.empty()) return true;
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
        const auto& m = batch.instance.transform;
        // Eye Z row of the ELAN model-view matrix: -(tm02, tm12, tm22, tm32).
        const float centerZ = -(m[2] * centerObject[0] + m[5] * centerObject[1] +
                                m[8] * centerObject[2] + m[11]);
        const float extentZ = std::fabs(m[2] * extents[0]) +
                              std::fabs(m[5] * extents[1]) +
                              std::fabs(m[8] * extents[2]);
        const float minZ = centerZ - extentZ;
        const float maxZ = centerZ + extentZ;
        const float nearPlane = batch.instance.nearValue;
        const float farPlane = batch.instance.farValue;
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

    static bool batchDiagnosticallyExcluded(uint32_t ichOffset) {
        bool present = false;
        if (ichInEnvList("IDAS3_NATIVE_DIAG_SKIP_ICH", ichOffset, present)) return true;
        const bool onlyMatch =
            ichInEnvList("IDAS3_NATIVE_DIAG_ONLY_ICH", ichOffset, present);
        return present && !onlyMatch;
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
        state.useAlpha = (tsp & (1u << 20u)) != 0u;
        state.offset = (pcw & (1u << 2u)) != 0u;
        state.dstSelect = (tsp & (1u << 24u)) != 0u;
        state.srcSelect = (tsp & (1u << 25u)) != 0u;
        state.dstBlend = static_cast<uint8_t>((tsp >> 26u) & 7u);
        state.srcBlend = static_cast<uint8_t>((tsp >> 29u) & 7u);
        state.cullMode = static_cast<uint8_t>((ispTsp >> 27u) & 3u);
        state.depthMode = static_cast<uint8_t>((ispTsp >> 29u) & 7u);
        state.depthWrite = (ispTsp & (1u << 26u)) == 0u;
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

    static int textureIndex(int64_t value, int size, bool clamp, bool flip) {
        if (clamp)
            return static_cast<int>(std::clamp<int64_t>(value, 0, size - 1));
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
        const uint32_t c00 = textureTexel(binding, x0, y0);
        const uint32_t c10 = textureTexel(binding, x0 + 1, y0);
        const uint32_t c01 = textureTexel(binding, x0, y0 + 1);
        const uint32_t c11 = textureTexel(binding, x0 + 1, y0 + 1);
        const auto channel = [&](uint32_t shift) -> uint32_t {
            const double top = static_cast<double>((c00 >> shift) & 0xFFu) * (1.0 - dx) +
                               static_cast<double>((c10 >> shift) & 0xFFu) * dx;
            const double bottom = static_cast<double>((c01 >> shift) & 0xFFu) * (1.0 - dx) +
                                  static_cast<double>((c11 >> shift) & 0xFFu) * dx;
            return static_cast<uint32_t>(std::clamp(
                std::lround(top * (1.0 - dy) + bottom * dy), 0l, 255l));
        };
        return (channel(24u) << 24u) | (channel(16u) << 16u) |
               (channel(8u) << 8u) | channel(0u);
    }

    static bool writeTextureAtlas(const std::string& path,
                                  const std::vector<TextureCacheEntry>& entries) {
        if (path.empty() || entries.empty()) return false;
        constexpr uint32_t columns = 16u;
        constexpr uint32_t cell = 64u;
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
            const bool pcwTexture = (batch->pcw & (1u << 3u)) != 0u;
            if (pcwTexture) ++result.texturedPcwBatches;
            if (!pcwTexture || uvVertices == 0u) continue;
            ++result.textureCandidateBatches;
            const uint32_t materialParams = batch->material.valid
                ? batch->material.words[2] : 0u;
            auto it = std::find_if(states.begin(), states.end(), [&](const auto& state) {
                return state.pcw == batch->pcw && state.tsp == batch->tsp0 &&
                       state.tcw == batch->tcw0 &&
                       state.materialParams == materialParams;
            });
            if (it == states.end()) {
                states.push_back({batch->pcw, batch->tsp0, batch->tcw0,
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
        if (a.flags != b.flags || a.vertexCount != b.vertexCount ||
            a.vertices.size() != b.vertices.size()) return false;
        for (size_t i = 0; i < a.vertices.size(); ++i) {
            const auto& av = a.vertices[i];
            const auto& bv = b.vertices[i];
            if (av.header != bv.header || av.x != bv.x || av.y != bv.y || av.z != bv.z ||
                av.hasUv != bv.hasUv || (av.hasUv && (av.u != bv.u || av.v != bv.v)))
                return false;
        }
        return true;
    }

    static bool sameRenderState(const NativeElanDrawBatch& a, const NativeElanDrawBatch& b) {
        return a.pcw == b.pcw && a.ispTsp == b.ispTsp &&
               a.tsp0 == b.tsp0 && a.tcw0 == b.tcw0 &&
               a.tsp1 == b.tsp1 && a.tcw1 == b.tcw1 &&
               a.projection.valid == b.projection.valid &&
               a.projection.fx == b.projection.fx && a.projection.tx == b.projection.tx &&
               a.projection.fy == b.projection.fy && a.projection.ty == b.projection.ty &&
               a.instance.valid == b.instance.valid &&
               a.instance.nearValue == b.instance.nearValue &&
               a.instance.transform == b.instance.transform &&
               a.material.valid == b.material.valid && a.material.words == b.material.words;
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

    static uint32_t fillTriangle(std::vector<uint8_t>& rgb,
                                 std::vector<uint8_t>& alpha,
                                 std::vector<uint8_t>& secondaryRgb,
                                 std::vector<uint8_t>& secondaryAlpha,
                                 uint32_t width, uint32_t height,
                                 const Point& a, const Point& b, const Point& c,
                                 std::vector<float>* depth,
                                 const TextureBinding* texture,
                                 const RasterState& rasterState,
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
        const int minX = static_cast<int>(std::floor(std::clamp(
            std::min({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        const int maxX = static_cast<int>(std::ceil(std::clamp(
            std::max({a.x, b.x, c.x}), 0.0f, viewportMaxX)));
        const int minY = static_cast<int>(std::floor(std::clamp(
            std::min({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        const int maxY = static_cast<int>(std::ceil(std::clamp(
            std::max({a.y, b.y, c.y}), 0.0f, viewportMaxY)));
        const bool positive = area > 0.0f;
        uint32_t written = 0u;
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                const float px = static_cast<float>(x) + 0.5f;
                const float py = static_cast<float>(y) + 0.5f;
                const float e0 = edge(a, b, px, py);
                const float e1 = edge(b, c, px, py);
                const float e2 = edge(c, a, px, py);
                const bool inside = positive ? (e0 >= 0.0f && e1 >= 0.0f && e2 >= 0.0f)
                                             : (e0 <= 0.0f && e1 <= 0.0f && e2 <= 0.0f);
                if (!inside) continue;
                const float wa = e1 / area;
                const float wb = e2 / area;
                const float wc = e0 / area;
                const float z = wa * a.depth + wb * b.depth + wc * c.depth;
                const size_t index = static_cast<size_t>(y) * width +
                                     static_cast<uint32_t>(x);
                if (depth) {
                    if (!std::isfinite(z) ||
                        !passesDepth(rasterState.depthMode, z, (*depth)[index]))
                        continue;
                }

                const auto vertexChannel = [&](unsigned shift) {
                    return static_cast<int>(std::clamp(
                        wa * static_cast<float>((a.argb >> shift) & 0xFFu) +
                        wb * static_cast<float>((b.argb >> shift) & 0xFFu) +
                        wc * static_cast<float>((c.argb >> shift) & 0xFFu),
                        0.0f, 255.0f) + 0.5f);
                };
                const int vertexA = vertexChannel(24u);
                const int vertexR = vertexChannel(16u);
                const int vertexG = vertexChannel(8u);
                const int vertexB = vertexChannel(0u);
                const auto offsetChannel = [&](unsigned shift) {
                    return static_cast<int>(std::clamp(
                        wa * static_cast<float>((a.offsetArgb >> shift) & 0xFFu) +
                        wb * static_cast<float>((b.offsetArgb >> shift) & 0xFFu) +
                        wc * static_cast<float>((c.offsetArgb >> shift) & 0xFFu),
                        0.0f, 255.0f) + 0.5f);
                };
                const int offsetR = offsetChannel(16u);
                const int offsetG = offsetChannel(8u);
                const int offsetB = offsetChannel(0u);
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

                const size_t colorOffset = index * 3u;
                uint8_t shadedSource[4] = {
                    static_cast<uint8_t>((shaded >> 16u) & 0xFFu),
                    static_cast<uint8_t>((shaded >> 8u) & 0xFFu),
                    static_cast<uint8_t>(shaded & 0xFFu),
                    static_cast<uint8_t>((shaded >> 24u) & 0xFFu)
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
                uint8_t output[4]{};
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
                if (depth && rasterState.depthWrite) (*depth)[index] = z;
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
