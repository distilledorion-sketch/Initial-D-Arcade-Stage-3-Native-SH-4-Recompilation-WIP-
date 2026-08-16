# Runtime source snapshots

These files are selected from the private integration tree to show the real implementation work:

- `native_elan_bridge.h` contains the stripped, bounded NAOMI 2 ELAN command walker and classifier.
- `native_elan_decode.h` tracks persistent ELAN state and decodes linked scene structures.
- `native_elan_framebuffer.h` contains the diagnostic ELAN renderer used for the progress captures.
- `native_sq_geometry.h` observes bounded ICH submissions without changing guest state.
- `jvs_837_13551.h` models the cabinet-facing JVS/Maple protocol boundary.

Only `native_elan_bridge.h` is compiled by the standalone public smoke test. The decoder/renderer snapshots refer to support headers in the private integration tree and are included for review, not as a claim that this repository is a complete runtime.

The ELAN traversal is adapted from a pinned Flycast v2.6 source revision. See [Flycast ELAN attribution](../../docs/FLYCAST_ELAN_ATTRIBUTION.md) and the repository GPL license.

Generated game translations, private memory/device integration, replay code, asset inputs, HOSTFS overlays, and compatibility-only experiments are intentionally excluded.
