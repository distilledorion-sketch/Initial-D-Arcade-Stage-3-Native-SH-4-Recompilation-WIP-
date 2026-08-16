# Runtime source snapshots

These files are selected from the private integration tree to show the real clean-room implementation work:

- `native_elan_bridge.h` classifies validated NAOMI 2 link and wait commands.
- `native_elan_decode.h` tracks persistent ELAN state and decodes linked scene structures.
- `native_elan_framebuffer.h` contains the diagnostic ELAN renderer used for the progress captures.
- `jvs_837_13551.h` models the cabinet-facing JVS/Maple protocol boundary.

Only `native_elan_bridge.h` is compiled by the standalone public smoke test. The decoder/renderer snapshots refer to support headers in the private integration tree and are included for review, not as a claim that this repository is a complete runtime.

Generated game translations, memory/device integration, private replay code, asset inputs, and compatibility-only experiments are intentionally excluded.
