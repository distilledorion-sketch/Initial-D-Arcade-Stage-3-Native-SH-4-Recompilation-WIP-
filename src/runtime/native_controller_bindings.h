#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace idas3input {

enum class ControllerBindingKind : uint8_t {
    none = 0u,
    button = 1u,
    axisNegative = 2u,
    axisPositive = 3u
};

struct ControllerBinding {
    ControllerBindingKind kind = ControllerBindingKind::none;
    uint8_t index = 0u;
};

struct ControllerSnapshot {
    std::array<float, 8u> axes{};
    std::array<uint8_t, 128u> buttons{};
};

struct ControllerCaptureResult {
    bool found = false;
    ControllerBinding binding{};
};

constexpr uint16_t packControllerBinding(ControllerBinding binding) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(binding.kind) << 8u) | binding.index);
}

constexpr ControllerBinding unpackControllerBinding(uint16_t packed) {
    const uint8_t kind = static_cast<uint8_t>((packed >> 8u) & 0xFFu);
    return {
        kind <= static_cast<uint8_t>(ControllerBindingKind::axisPositive)
            ? static_cast<ControllerBindingKind>(kind)
            : ControllerBindingKind::none,
        static_cast<uint8_t>(packed & 0xFFu)};
}

inline float normalizeSignedAxis(int32_t value, int32_t deadzone,
                                 int32_t maximum = 32767) {
    const int64_t widened = value;
    const int64_t magnitude = std::llabs(widened);
    if (magnitude <= deadzone) return 0.0f;
    const double usable = std::max<int32_t>(1, maximum - deadzone);
    const double normalized = std::min(
        1.0, static_cast<double>(magnitude - deadzone) / usable);
    return static_cast<float>(widened < 0 ? -normalized : normalized);
}

inline float normalizeUnsignedAxis(uint32_t value, uint32_t threshold,
                                   uint32_t maximum = 255u) {
    if (value <= threshold) return 0.0f;
    const double usable = std::max<uint32_t>(1u, maximum - threshold);
    return static_cast<float>(std::min(
        1.0, static_cast<double>(value - threshold) / usable));
}

inline float normalizeDirectInputAxis(int32_t value) {
    const double centered =
        (static_cast<double>(std::clamp<int32_t>(value, 0, 65535)) -
         32767.5) / 32767.5;
    return static_cast<float>(std::clamp(centered, -1.0, 1.0));
}

inline float controllerBindingStrength(
        const ControllerSnapshot& snapshot, ControllerBinding binding) {
    switch (binding.kind) {
    case ControllerBindingKind::button:
        return binding.index < snapshot.buttons.size() &&
                snapshot.buttons[binding.index] != 0u
            ? 1.0f : 0.0f;
    case ControllerBindingKind::axisNegative:
        return binding.index < snapshot.axes.size()
            ? std::max(0.0f, -snapshot.axes[binding.index]) : 0.0f;
    case ControllerBindingKind::axisPositive:
        return binding.index < snapshot.axes.size()
            ? std::max(0.0f, snapshot.axes[binding.index]) : 0.0f;
    default:
        return 0.0f;
    }
}

inline ControllerCaptureResult detectControllerBinding(
        const ControllerSnapshot& baseline,
        const ControllerSnapshot& current,
        float axisThreshold = 0.35f) {
    // Buttons win over axes so pressing a face button while a noisy stick is
    // near its threshold always captures the deliberate control.
    for (uint16_t index = 0u; index < current.buttons.size(); ++index) {
        if (current.buttons[index] != 0u && baseline.buttons[index] == 0u) {
            return {true, {ControllerBindingKind::button,
                           static_cast<uint8_t>(index)}};
        }
    }
    float largestDelta = axisThreshold;
    ControllerBinding best{};
    for (uint8_t index = 0u; index < current.axes.size(); ++index) {
        const float delta = current.axes[index] - baseline.axes[index];
        if (std::fabs(delta) <= largestDelta) continue;
        largestDelta = std::fabs(delta);
        best = {delta < 0.0f
                    ? ControllerBindingKind::axisNegative
                    : ControllerBindingKind::axisPositive,
                index};
    }
    return {best.kind != ControllerBindingKind::none, best};
}

inline uint16_t arcadeSteeringFromBindings(
        const ControllerSnapshot& snapshot,
        ControllerBinding left, ControllerBinding right) {
    const float leftStrength = controllerBindingStrength(snapshot, left);
    const float rightStrength = controllerBindingStrength(snapshot, right);
    const float signedValue = std::clamp(
        rightStrength - leftStrength, -1.0f, 1.0f);
    return static_cast<uint16_t>(std::clamp<int32_t>(
        static_cast<int32_t>(std::lround(
            32767.5 + static_cast<double>(signedValue) * 32767.5)),
        0, 65535));
}

// First-order steering filter evaluated on the authentic 60 Hz cabinet input
// cadence. Zero is a true bypass. The strongest setting retains ten percent
// of each new movement per guest poll, reducing noisy wheel/stick motion
// without changing the input range or advancing gameplay at display rate.
inline uint16_t smoothArcadeSteering(uint16_t previous, uint16_t raw,
                                     uint8_t strengthPercent) {
    const uint8_t strength = std::min<uint8_t>(strengthPercent, 100u);
    if (strength == 0u || previous == raw) return raw;
    const double response = 1.0 -
        static_cast<double>(strength) * 0.009;
    const int32_t delta = static_cast<int32_t>(raw) -
        static_cast<int32_t>(previous);
    const int32_t filtered = static_cast<int32_t>(previous) +
        static_cast<int32_t>(std::lround(static_cast<double>(delta) * response));
    if (std::abs(static_cast<int32_t>(raw) - filtered) <= 1) return raw;
    return static_cast<uint16_t>(std::clamp<int32_t>(filtered, 0, 65535));
}

inline uint16_t arcadePedalFromBinding(
        const ControllerSnapshot& snapshot, ControllerBinding binding) {
    return static_cast<uint16_t>(std::clamp<int32_t>(
        static_cast<int32_t>(std::lround(
            controllerBindingStrength(snapshot, binding) * 65535.0f)),
        0, 65535));
}

}  // namespace idas3input
