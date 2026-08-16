# Contributing

The project is currently an engineering-progress showcase rather than an open contribution target. Focused technical reports are welcome as GitHub issues.

Do not attach or link to game images, BIOS/firmware dumps, PIC/CHD data, extracted assets, memory snapshots, generated game translation units, or reference video.

Useful reports include:

- A reproducible failure in one of the public tests.
- A bounded correctness issue in the general SH-4 decoder/code generator.
- Clean-room documentation of a hardware behavior, with a primary public source where possible.
- A proposed acceptance test that fails closed and requires no proprietary input.

All code changes should preserve the native static-AOT direction. Interpreter/JIT fallbacks, fake-success behavior, or broad guest-logic bypasses are out of scope.
