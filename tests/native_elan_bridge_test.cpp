#include "native_elan_bridge.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static void expect(bool value, const char* message) {
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

static void writeCommand(std::vector<uint8_t>& ram, uint32_t offset,
                         const std::array<uint32_t, 8>& words) {
    expect(offset + sizeof(words) <= ram.size(), "synthetic command range");
    std::memcpy(ram.data() + offset, words.data(), sizeof(words));
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

    std::vector<uint8_t> ram(0x1000u, 0u);
    const std::array<uint32_t, 8> wait{
        0x08000E00u, 0x005F6903u, 0u, 0x80u, 0u, 0u, 0u, 0u,
    };
    const std::array<uint32_t, 8> link{
        0x08000F00u, 0x00000200u, 0u, 0x20u, 0u, 0u, 0u, 0u,
    };
    writeCommand(ram, 0x100u, link);
    writeCommand(ram, 0x200u, wait);

    const NativeElanControlWalkResult nested =
        NativeElanControlWalker::walk(ram.data(), ram.size(), 0x100u, 0x20u);
    expect(nested.ok, "nested Link walk accepted");
    expect(nested.streams == 2u, "nested Link stream count");
    expect(nested.records == 2u, "nested Link record count");
    expect(nested.recursiveLinks == 1u, "nested Link recursion count");
    expect(nested.events.size() == 1u, "nested wait event count");
    expect(nested.events[0].command.kind() ==
               NativeElanKind::RegisterWaitPunchthrough,
           "nested wait event kind");

    const std::array<uint32_t, 8> selfLink{
        0x08000F00u, 0x00000400u, 0u, 0x20u, 0u, 0u, 0u, 0u,
    };
    writeCommand(ram, 0x400u, selfLink);
    const NativeElanControlWalkResult cycle =
        NativeElanControlWalker::walk(ram.data(), ram.size(), 0x400u, 0x20u);
    expect(!cycle.ok, "recursive cycle rejected");
    expect(cycle.failure == NativeElanWalkFailure::StreamCycle,
           "recursive cycle failure kind");
    expect(cycle.events.empty(), "cycle emits no side effects");

    const std::array<uint32_t, 8> badWait{
        0x08000E00u, 0x005F6903u, 0u, 1u, 0u, 0u, 0u, 0u,
    };
    writeCommand(ram, 0x600u, badWait);
    const NativeElanControlWalkResult invalid =
        NativeElanControlWalker::walk(ram.data(), ram.size(), 0x600u, 0x20u);
    expect(!invalid.ok, "unsupported active wait rejected");
    expect(invalid.failure == NativeElanWalkFailure::InvalidRegisterWait,
           "unsupported wait failure kind");
    expect(invalid.events.empty(), "unsupported wait emits no side effects");

    std::puts("native ELAN command classifier/walker: PASS");
}
