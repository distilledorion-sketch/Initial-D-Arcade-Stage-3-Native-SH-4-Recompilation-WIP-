#include "native_elan_bridge.h"

#include <array>
#include <cstdio>
#include <cstdlib>

static void expect(bool value, const char* message) {
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

int main() {
    NativeElanCommand command{};
    command.w[0] = 0x08000F00u;
    command.w[1] = 0xA0000000u;
    command.w[2] = 0x11800800u;
    command.w[3] = 0x00004000u;

    expect(command.isNaomi2(), "NAOMI 2 command bit");
    expect(command.isCanonicalControlHeader(), "canonical control header");
    expect(command.kind() == NativeElanKind::LinkTextureDmaSh4, "SH-4 DMA link");

    command.w[1] = 0x20000100u;
    expect(command.kind() == NativeElanKind::LinkTextureDmaEram, "ERAM DMA link");
    command.w[1] = 0x00000100u;
    expect(command.kind() == NativeElanKind::LinkRecursive, "recursive link");

    command.w[0] = 0x3F54AE40u;
    expect(command.isNaomi2(), "floating payload can alias command bit");
    expect(!command.isCanonicalControlHeader(), "floating payload is not a command header");

    command = {};
    command.w[0] = 0x08000E00u;
    command.w[1] = 0x005F6903u;
    constexpr std::array<uint32_t, 5> waitBits{
        0x80u, 0x100u, 0x200u, 0x400u, 0x200000u,
    };
    for (const uint32_t mask : waitBits) {
        command.w[3] = mask;
        expect(command.kind() == NativeElanKind::RegisterWaitPunchthrough,
               "known list-complete wait");
    }

    command.w[3] = 0u;
    expect(command.kind() == NativeElanKind::Unknown, "empty wait rejected");
    command.w[3] = 1u;
    expect(command.kind() == NativeElanKind::Unknown, "unknown wait rejected");

    std::puts("native ELAN command classifier: PASS");
}
