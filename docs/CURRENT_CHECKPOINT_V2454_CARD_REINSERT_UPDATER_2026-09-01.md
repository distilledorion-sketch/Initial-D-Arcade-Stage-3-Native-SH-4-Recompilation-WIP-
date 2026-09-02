# v2454 card-reinsert and verified-updater checkpoint

Date: 2026-09-01
Scope: Windows x64 public early demo, post-result card lifecycle, and
launcher-time updates

## Product boundary

The executable remains a BIOS-free static SH-4 recompilation. The public ZIP
contains no game data, NAOMI firmware, security PIC, extracted assets, card
save, custom music, user log, memory snapshot, generated guest translation, or
private host path. Users must provide their own matching `gds-0033.chd` and
`317-0384-com.pic`; setup verifies and extracts them locally without Python.

## Card lifecycle correction

The reader previously allowed a host reinsert request to replace the physical
front-slot state while the guest was still completing an eject screen. The
v2454 state machine keeps those events separate:

1. A normal card write persists exactly 207 bytes.
2. Eject moves that card to the reader's front sensor.
3. The next ordinary status lets the guest observe that the card was removed.
4. Only after that observation is the selected saved card queued again.
5. The card becomes visible at the next genuine frontend/capture request, not
   during the old result screen.

The exact offline card suite now covers 27 protocol transactions, including an
early queued insert request, the empty post-eject status, delayed capture,
automatic second reinsert, and initialization cleanup.

A bounded 60 Hz Bunta result run used a disposable card and the exact v2454
candidate. It recorded `save=ok bytes=207`, followed by
`reinsert=queued source=post-eject-removal`, zero recognized errors/runtime
faults, cooperative exit code 0, and no remaining game process. The resulting
card was 207 bytes with SHA-256
`230B7502BA64637F065DF87035B336696D80AD6BE2897421DFAF790CCAAE8A72`.

## Launcher update behavior

Before game setup or launch, the Windows launcher queries this repository's
release list and selects only the highest release whose tag begins with a
higher `vNNNN` product version and contains a non-empty ZIP asset. A prompt
shows the new version and local release date, offers Yes and No, links to the
GitHub release notes, and includes an optional "Automatically install future
updates" checkbox.

The updater:

- does not block game launch when GitHub or the network is unavailable;
- downloads only the selected GitHub release asset;
- requires and verifies GitHub's `sha256:` asset digest;
- rejects unsafe archive paths, ambiguous package roots, and a mismatched
  `PRODUCT_VERSION.txt` marker;
- installs from an external PowerShell process before the game starts;
- preserves `game files`, `card data`, `custom music`, `logs`,
  `idas3_user_settings.ini`, and the updater preference;
- backs up replaced product files and rolls them back if installation fails;
- relaunches the packaged frontend after a successful install.

Automatic updates use the same download, digest, archive, version, preservation,
and rollback checks; they only skip the Yes/No prompt.

## Acceptance

- PowerShell parsing: PASS for launcher, update support, installer, and updater
  test.
- Update selection/no-downgrade/preference tests: PASS.
- Digest, safe extraction, and version-root tests: PASS.
- In-place install, product replacement, card/settings preservation, and
  rollback policy tests: PASS.
- Isolated moved-launcher fast-to-full integrity retry: PASS; no game process
  started.
- Full v2454 offline controller/music/card/ELAN/Vulkan/translation/link and
  no-firmware product suite: PASS.
- Bunta loss continuation: 240 seconds, two 207-byte saves/eject cycles, zero
  recognized faults, cooperative exit.
- Bunta win continuation: 220 seconds, 207-byte save/eject/reinsert, zero
  recognized faults, cooperative exit.
- No-input attract run: 180 seconds at 60 Hz, no repeats in sampled output,
  zero recognized faults, cooperative exit.

These bounded paths do not prove every replay, card-error, campaign, hardware,
or high-refresh permutation.

## Published artifacts

- Native executable SHA-256:
  `4E9D607E2F475E38AF8A40BAF6DAE4D8F79CB62C57172D1BB0D1E6FCA065C013`
- Public ZIP SHA-256:
  `041CBA8943BF7DF43F61B9843FC81C93C5C8107E2FAAF16E16CFA45B69B317AD`
- Public ZIP size: 18,575,765 bytes
- Public ZIP file count: 17

The clean ZIP was extracted into a fresh temporary directory and rechecked for
its exact runtime hash, version marker, file count, forbidden extensions, and
private paths before publication.
