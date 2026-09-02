# v2462 Direct Vulkan and Unicode music checkpoint — 2026-09-02

v2462 keeps the public early-demo boundary: the archive contains the host
launcher/runtime only and requires matching user-owned game inputs. It does
not contain a BIOS, game data, card saves, custom music, logs, captures,
learned course paths, generated guest-code source, or private paths.

## Changes

- Fresh installations factory-default to normal Direct Vulkan presentation.
  Existing users keep their saved Direct/Safe selection during merge updates,
  and Safe remains available as a manual troubleshooting option.
- Custom-music directory discovery uses native Windows Unicode filenames and
  UTF-8 internal paths, so Japanese and other non-ANSI titles are accepted.
- Two exact, binary-backed local BRAF jump tables were restored inside their
  owning translated functions. The fail-closed verifier accepts both repairs.

## Verification

- Native runtime SHA-256:
  `554621C84D0A6E4EA5BD7E331A3A15AEE8183337DBC5FEC73C26337247769FD8`
- Public ZIP SHA-256:
  `F00123E09C5B3A77A65D81A63BD363BE6404ADFEFB6F03F8913CE0E72AEBF1F1`
- Public ZIP size: 24,078,735 bytes; 18 files.
- The Unicode custom-music regression, controller/music policies, 34 shutdown
  policies, 20 route-parser policies, fixed-point RTS audit, two-repair BRAF
  verifier, translated-source check, 116-object freshness check, and
  standalone no-firmware audit pass.
- A muted, one-worker, BIOS-free Safe-presentation boot ran for 60 seconds at
  strict 60 Hz, averaged 59.9 FPS output, generated zero repeated frames,
  emitted no untranslated/runtime/Vulkan fault, and shut down cooperatively.
- A digest-verified updater rehearsal preserved game files, card data, custom
  music, logs, settings, and destination-only files. An existing Safe choice
  remained Safe; the archive's fresh-install template remained Direct.

## Limits

This remains unfinished playtest software. Higher-refresh presentation,
remaining campaign permutations, controller/wheel variety, and full visual
comparison across every content combination still require additional work.
