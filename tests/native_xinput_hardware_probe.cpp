#include "native_windows_xinput.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    bool allowNone = false;
    for (int index = 1; index < argc; ++index) {
        if (std::strcmp(argv[index], "--allow-none") == 0)
            allowNone = true;
    }

    idas3input::NativeXInputRuntime runtime;
    std::printf("[XINPUT-PROBE] runtime=%u dll='%s'\n",
        runtime.available() ? 1u : 0u, runtime.moduleName());
    if (!runtime.available()) {
        std::fprintf(stderr,
            "[XINPUT-PROBE] result=FAIL reason=xinput-runtime-unavailable\n");
        return allowNone ? 0 : 2;
    }

    const std::vector<DWORD> users = runtime.connectedUsers();
    std::printf("[XINPUT-PROBE] devices=%zu", users.size());
    for (DWORD user : users)
        std::printf(" user=%lu", static_cast<unsigned long>(user));
    std::printf("\n");
    if (users.empty()) {
        std::fprintf(stderr,
            "[XINPUT-PROBE] result=%s reason=no-connected-controller\n",
            allowNone ? "SKIP" : "FAIL");
        return allowNone ? 0 : 3;
    }

    const DWORD selected = users.front();
    XINPUT_STATE state{};
    const DWORD status = runtime.getState(selected, &state);
    if (status != ERROR_SUCCESS) {
        std::fprintf(stderr,
            "[XINPUT-PROBE] result=FAIL reason=selected-read status=%lu\n",
            static_cast<unsigned long>(status));
        return 4;
    }

    const idas3input::ControllerSnapshot snapshot =
        idas3input::nativeXInputSnapshot(state);
    std::printf(
        "[XINPUT-PROBE] selected=%lu packet=%lu buttons=0x%04X "
        "lx=%.3f ly=%.3f rx=%.3f ry=%.3f lt=%.3f rt=%.3f\n",
        static_cast<unsigned long>(selected),
        static_cast<unsigned long>(state.dwPacketNumber),
        static_cast<unsigned>(state.Gamepad.wButtons),
        snapshot.axes[0], snapshot.axes[1], snapshot.axes[2],
        snapshot.axes[3], snapshot.axes[4], snapshot.axes[5]);

    const idas3input::ControllerBinding left{
        idas3input::ControllerBindingKind::axisNegative, 0u};
    const idas3input::ControllerBinding right{
        idas3input::ControllerBindingKind::axisPositive, 0u};
    const idas3input::ControllerBinding accelerator{
        idas3input::ControllerBindingKind::axisPositive, 5u};
    const idas3input::ControllerBinding brake{
        idas3input::ControllerBindingKind::axisPositive, 4u};
    std::printf(
        "[XINPUT-PROBE] cabinet steering=%u accelerator=%u brake=%u\n",
        static_cast<unsigned>(idas3input::arcadeSteeringFromBindings(
            snapshot, left, right)),
        static_cast<unsigned>(idas3input::arcadePedalFromBinding(
            snapshot, accelerator)),
        static_cast<unsigned>(idas3input::arcadePedalFromBinding(
            snapshot, brake)));
    std::printf("[XINPUT-PROBE] result=PASS product-shared-path=1\n");
    return 0;
}
