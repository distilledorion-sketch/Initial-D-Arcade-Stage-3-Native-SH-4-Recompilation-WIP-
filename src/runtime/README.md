# Runtime source snapshots

These files are selected from the private integration tree to show the real clean-room implementation work:

- `native_elan_bridge.h` classifies validated NAOMI 2 link and wait commands.
- `native_elan_decode.h` tracks persistent ELAN state and decodes linked scene structures.
- `native_elan_framebuffer.h` contains the diagnostic ELAN renderer used for the progress captures.
- `jvs_837_13551.h` models the cabinet-facing JVS/Maple protocol boundary.
- `native_controller_bindings.h` contains provider-neutral controller binding,
  axis normalization, capture, arcade-range conversion, and the v2413
  presentation-rate-independent steering filter.
- `native_windows_xinput.h` is the product-shared Windows XInput loader,
  connected-slot discovery path, and state-to-provider normalization used by
  both the presenter and the public hardware probe.
- `native_windows_audio_endpoint.h` defines the product's 44.1 kHz stereo PCM
  output format and its Windows endpoint identity/volume query.

`native_elan_bridge.h` and `native_controller_bindings.h` are compiled by the
portable public checks. On Windows, the same check set also builds no-window
XInput and silent endpoint probes from the product-shared headers. The
decoder/renderer snapshots refer to support
headers in the private integration tree and are included for review, not as a
claim that this repository is a complete runtime.

The private v2458 integration tree has advanced beyond these selected snapshots
with targeted menu-to-race/result/save execution, corrected HUD and mirror
projection, additional PVR list features, native AICA/Maple/JVS/card tests,
persistent renderer workers, Vulkan pipeline prewarming, fixed 60 Hz retained
presentation during guest loads, distinct 120 Hz presentation accounting,
exact authored-cinematic-matte widescreen handling, and asset-lineage
validation. It also includes developer-only persisted Time Attack input-path
QA, ordered learned-path controller selection across adjacent switchbacks, and
a Direct-authoritative renderer policy that never silently changes a
saved Direct choice to Safe because of diagnostic marker state; no learned path
data is published. These components will be moved into this public tree only when they
can be separated cleanly from generated game code and private captures without
weakening the legal boundary.

Generated game translations, memory/device integration, private replay code, asset inputs, and compatibility-only experiments are intentionally excluded.
