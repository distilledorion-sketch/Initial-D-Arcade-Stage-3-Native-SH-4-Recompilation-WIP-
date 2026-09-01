#include "native_controller_bindings.h"

#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    using namespace idas3input;
    const ControllerBinding left{ControllerBindingKind::axisNegative, 0u};
    const ControllerBinding right{ControllerBindingKind::axisPositive, 0u};
    const ControllerBinding trigger{ControllerBindingKind::axisPositive, 5u};
    const ControllerBinding button{ControllerBindingKind::button, 4u};

    assert(packControllerBinding(left) == 0x0200u);
    assert(packControllerBinding(right) == 0x0300u);
    assert(unpackControllerBinding(packControllerBinding(trigger)).index == 5u);
    assert(unpackControllerBinding(0xFF01u).kind ==
           ControllerBindingKind::none);

    assert(normalizeSignedAxis(0, 7849) == 0.0f);
    assert(normalizeSignedAxis(32767, 7849) > 0.999f);
    assert(normalizeSignedAxis(-32768, 7849) < -0.999f);
    assert(normalizeUnsignedAxis(0u, 30u) == 0.0f);
    assert(normalizeUnsignedAxis(255u, 30u) > 0.999f);
    assert(normalizeDirectInputAxis(0) < -0.999f);
    assert(std::fabs(normalizeDirectInputAxis(32768)) < 0.001f);
    assert(normalizeDirectInputAxis(65535) > 0.999f);

    ControllerSnapshot state{};
    assert(arcadeSteeringFromBindings(state, left, right) == 32768u);
    state.axes[0] = -1.0f;
    assert(arcadeSteeringFromBindings(state, left, right) == 0u);
    state.axes[0] = 1.0f;
    assert(arcadeSteeringFromBindings(state, left, right) == 65535u);
    assert(smoothArcadeSteering(32768u, 65535u, 0u) == 65535u);
    const uint16_t mediumSmoothing =
        smoothArcadeSteering(32768u, 65535u, 50u);
    const uint16_t maximumSmoothing =
        smoothArcadeSteering(32768u, 65535u, 100u);
    assert(mediumSmoothing > maximumSmoothing);
    assert(maximumSmoothing > 32768u && maximumSmoothing < 65535u);
    assert(smoothArcadeSteering(32768u, 32768u, 100u) == 32768u);
    uint16_t converging = 32768u;
    for (unsigned poll = 0u; poll < 120u; ++poll)
        converging = smoothArcadeSteering(converging, 65535u, 100u);
    assert(converging > 65520u);
    state.axes[5] = 0.5f;
    assert(arcadePedalFromBinding(state, trigger) >= 32767u);
    state.buttons[4] = 1u;
    assert(controllerBindingStrength(state, button) == 1.0f);

    ControllerSnapshot baseline{};
    ControllerSnapshot moved{};
    moved.axes[5] = 0.8f;
    auto captured = detectControllerBinding(baseline, moved);
    assert(captured.found);
    assert(captured.binding.kind == ControllerBindingKind::axisPositive);
    assert(captured.binding.index == 5u);
    moved.buttons[2] = 1u;
    captured = detectControllerBinding(baseline, moved);
    assert(captured.binding.kind == ControllerBindingKind::button);
    assert(captured.binding.index == 2u);

    // A deliberate partial stick/wheel movement must be enough to remap an
    // axis. Requiring more than half travel made the F1 control capture look
    // unresponsive, especially on large steering wheels.
    moved = {};
    moved.axes[0] = 0.40f;
    captured = detectControllerBinding(baseline, moved);
    assert(captured.found);
    assert(captured.binding.kind == ControllerBindingKind::axisPositive);
    assert(captured.binding.index == 0u);

    std::puts("controller binding normalization/capture tests passed");
    return 0;
}
