# Current public checkpoint — v2445

Date: 2026-09-01

Scope: persisted Direct/Safe Vulkan presentation selection

Native executable SHA-256: `49BEFEBCD2D23D97CF017EDB7F9F4E5E3685E23AA3994CA774EFE333EBCC4C5D`

Public ZIP SHA-256: `301741FEF79AC86CF56F85E9BC19E30D6DC4290833AE3CDF55596C4042F0BB0E`

## What changed

The F1 menu already saved `VulkanPresentation=Safe` or `Direct`, but the
Windows launcher did not consume that setting after restart. v2445 makes the
choice a validated launcher parameter, loads the saved value unless an
explicit command-line value overrides it, and maps both modes through one
shared helper:

- `Direct` maps to `IDAS3_NATIVE_VULKAN_OFFSCREEN=0`, selecting the optimized
  Vulkan swapchain path.
- `Safe` maps to `IDAS3_NATIVE_VULKAN_OFFSCREEN=1`, retaining GPU rendering
  while presenting its bounded readback through Win32.

The read-only `-ValidateOnly` output uses the same helper as the real runtime
environment assignment. This gives the package a no-launch acceptance path
for the exact mapping.

## Accepted evidence

- All four maintained launcher copies parse with zero PowerShell errors.
- A private-data-ready `-ValidateOnly` run resolved the saved Direct selection
  to offscreen flag `0`.
- An explicit Safe override in the same dry run resolved to offscreen flag `1`.
- Static launcher policy tests cover parameter validation, saved-setting
  precedence, the shared mapping, the Safe public default, and absence of
  private absolute paths.
- No game, Vulkan window, or audio process was started for this checkpoint.
- The native runtime is unchanged from the fully accepted v2444 executable,
  including its zero-firmware standalone audit.

## Public package audit

`Public Early Demo v2445.zip` is 18,565,971 bytes and contains 14 files. The
archive contains no game image, BIOS, security PIC, CHD, extracted asset, card
save, custom music, log, frame capture, generated guest translation,
credential, or private filesystem path.
