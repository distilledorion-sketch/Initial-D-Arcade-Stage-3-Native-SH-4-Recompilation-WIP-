# Status and evidence ledger

Status date: 2026-09-02
Integration checkpoint: v2462 WIP
Public checkpoint: v2462 Windows x64 early-demo prerelease

This file separates demonstrated behavior from hypotheses. Percentages are deliberately avoided: a technically advanced attract-mode path is not the same thing as a playable game.

| Area | Demonstrated | Current limitation |
|---|---|---|
| Static SH-4 translation | Broad instruction decoding/code generation; direct translated calls; FPSCR-aware FPU regression tests | Whole-program coverage and every indirect target are not complete |
| Native boot/runtime | BIOS-free static translated execution cold-boots and advances tested menu, loading, live-race, result, and save paths; 48/48 route/branch targets load | Remaining long campaign permutations and error branches are not complete |
| Deterministic replay | N70 accepted 70/70 with `unimplemented=0`, `FPSCR=0`, no fault/watchdog, and cross-machine identical frame hash | Uses private user-owned replay/input state that cannot be published |
| ELAN 3D path | Submitted links, CH2 DMA, completion interrupts, persistent state, materials, instances, lighting, textures, culling, depth, and retained presentation scenes advance continuously through tested races | Untested cars, courses, weather, and scene combinations still need systematic comparison |
| PVR rendering | Native handling exists for observed opaque/translucent lists, fog, modifier volumes, punch alpha, blend modes, texture layouts, autosort, tile clipping, tested HUD placement, mirrors, and exact widescreen expansion of authored attract cinematic mattes | Remaining combinations and presentation edge cases need scene-by-scene validation |
| Course environment maps | Missing logical lookup paths were recovered from two exact user-owned ISO files; rainbow/static maps are corrected | Other courses and weather conditions are not yet audited |
| JVS/input | 837-13551 identity/features, EEPROM, Maple VBlank/reset, keyboard routes, analog driving input, digital shifter routes, XInput-first device selection, hotplug rescan, paused-menu remapping, controller-operated F1 navigation, adjustable FFB, and saved 0–100% steering smoothing pass focused tests. A product-shared no-window probe found a real attached Xbox controller through `xinput1_4.dll` | Live physical axis/button movement, multiple controller models, wheels, and FFB acceptance remain open |
| Audio | Flycast-derived AICA SGC/mailbox tests pass; the ARM7/AICA path produces sustained non-silent samples; the product's 44.1 kHz stereo PCM format opens on the preferred Windows endpoint; 13 custom music mappings follow authentic stream commands; native Windows Unicode music filenames pass the v2462 regression | Audible end-user acceptance and exhaustive music/voice/effect coverage remain open |
| Card | No-card operation works; a disposable 207-byte card passes insert/load/save/eject/removal/reinsert/reload; selected managed cards use a move-safe path under the local `card data` folder. The post-Bunta result path now waits for the required physical-removal status before queuing the saved card for the next prompt | Real user card data is never used for QA; damaged-card and remaining error branches need coverage |
| Performance | Authentic 60 Hz guest timing with distinct 120 Hz presentation on measured routes; GPU projection, ELAN lighting, depth normalization, mapped geometry/uniform streams, static topology summaries, and Direct Vulkan display are active. A v2448 2560x1080 Akagi run held 120.0 FPS minimum/average across 21 moving-race samples, produced 240 distinct frames per two-second sample, and added zero repeats; v2441 also reduced the controlled warmed static-reuse frame from 0.696 ms to 0.629 ms median | 120 Hz is experimental; exhaustive content coverage and uncapped presentation are not complete |
| Host shutdown | Cooperative Vulkan stop request, raster-worker exit handshake, message pumping, join-before-window-destroy order, bounded acquire/fence/startup-upload waits, cooperative QA watchdog, single-instance enforcement, and Direct-session diagnostics pass source/offscreen checks. Fresh v2462 installs default to Direct Vulkan while a saved Direct/Safe choice remains authoritative and Safe remains manually selectable | Repeated live close/restart stress across more GPUs/drivers remains open |
| Distribution | The v2462 BIOS-free, Python-free Windows x64 public early demo accepts only matching user-owned inputs, performs verification/extraction locally, defaults fresh installs to Direct Vulkan, persists the selected Direct/Safe path, and checks GitHub for digest-verified merge updates. The archive has one canonically named top-level folder; the main executable and native runtime have stable product names | Clean-machine coverage is limited and the release remains unfinished |

## Strongest accepted evidence

- v2462 native executable SHA-256:
  `554621C84D0A6E4EA5BD7E331A3A15AEE8183337DBC5FEC73C26337247769FD8`.
  The build passes the Unicode music regression, 34 shutdown policies, 20
  route-parser policies, 15 controller/music checks, both binary-backed BRAF
  repair checks, the zero-proven-RTS-omissions audit, 116-object freshness,
  translated-source validation, and standalone no-firmware verification.
- A bounded BIOS-free v2462 boot used muted Safe presentation, one raster
  worker, Below Normal priority, and strict 60 Hz simulation/presentation for
  60 seconds. It averaged 59.9 FPS output with zero repeated frames, reported
  no untranslated/runtime/Vulkan faults, and exited cooperatively with no game
  process remaining.
- v2462 public ZIP SHA-256:
  `F00123E09C5B3A77A65D81A63BD363BE6404ADFEFB6F03F8913CE0E72AEBF1F1`;
  audited size is 24,078,735 bytes. Its 18 files contain no game data, BIOS,
  PIC, cards, music, logs, captures, learned paths, generated guest-code
  source, or private paths. A real updater rehearsal preserved mock user and
  destination-only data, retained an existing Safe preference, and confirmed
  the Direct default for a fresh install.

- v2461 native executable SHA-256:
  `104B7A357D18450A890EBBE7549FF2B180ECDA84C6AD131E63B8849A07B4D12A`.
  An offline fixed-point audit reports zero analyzer-proven non-NOP RTS
  delay-slot omissions after repairing 67 instructions across 60 functions.
  The complete private build, path-cache, route, controller/music, shutdown,
  translation, 116-object freshness, and standalone no-firmware suites pass.
- A bounded no-BIOS v2461 Akina Time Attack run used one raster worker, muted
  offscreen Vulkan, Below Normal priority, and strict 60 Hz simulation plus
  presentation. It followed the verified private course path for 1,488.022 m,
  reached one natural result with no forced result, averaged 60.0 FPS game
  output, reported zero untranslated calls or runtime faults, exited
  cooperatively, and left no game process.
- v2461 public ZIP SHA-256:
  `1B0ABCFDEFAF3F2A3E4CAC46D738259FF09AA4747AB825266A681097104AFAC5`;
  audited size is 24,067,516 bytes. Its 18 files contain no game data, BIOS,
  PIC, cards, music, logs, captures, learned paths, generated guest-code
  source, or private paths. Current-installer and exact-v2457-updater
  acceptances preserved user and destination-only data, used the canonical
  folder/runtime, and launched no game process.

- v2460 native executable SHA-256:
  `FCFBD471638FA7E7FB1643A9EFE5EF1FB95FC3847897434D8CF32EADA8037EE0`.
  Four retained-log targets were added only after disassembly/CFG review found
  bounded control flow and zero unknown instructions. The largest has 1,044
  reachable instructions, one normal return, no BRAF/BSRF edge, no interior
  translated entry, and a 32-byte boundary before the next function.
  Compilation, 34 restart/shutdown policies, 20 route-parser policies, 15
  controller/music policies, learned-path regression, translation-source and
  116-object freshness checks, and the standalone no-firmware audit pass.
- Separate no-BIOS, muted, offscreen 60 Hz v2460 Akina and Usui runs reached
  49.037 m and 53.181 m of real movement. Both reported zero untranslated
  calls, zero runtime faults, zero repeated presentation frames, cooperative
  exit code 0, and no remaining game process.
- v2460 public ZIP SHA-256:
  `7FAE695DA3449FB261B95E1256A60E75F3A12FA9BFAE44FF3B09BFF221D71F03`;
  audited size is 23,826,056 bytes. Its 18 files contain no game data, BIOS,
  PIC, cards, music, logs, captures, learned paths, generated guest-code
  source, or private paths. Current-installer and exact-v2457-updater
  acceptances preserved user and destination-only data, used the canonical
  folder/runtime, and launched no game process.

- v2459 native executable SHA-256:
  `C31ACF51EFB3DE210381E9754B4D3C702C770EC1CD7258DA2CD663E607CF0C5E`.
  Its analyzer-verified post-result helper contains 28 reachable SH-4
  instructions, one normal return, and no unknown opcode or unsafe control
  edge. Compilation, 34 restart/shutdown policies, 20 route-parser policies,
  15 controller/music policies, learned-path regression, link freshness, and
  the standalone no-firmware audit pass.
- A bounded offscreen v2459 Akina Time Attack run passed the earlier 663.071 m
  learned-path stall, reached 1,488.022 m and a natural result, and reported
  race display minimum/average of 59.8/60.0 FPS. It generated no repeated
  presentation frames, used no forced result, recorded zero recognized errors
  or runtime faults, exited cooperatively, and left no game process.
- v2459 public ZIP SHA-256:
  `B42F76F6D15D6CDE1014B94390755F336B263997ADF7AC73ED5F69C9DB39D5B2`;
  audited size is 23,813,057 bytes. Its 18 entries contain no game data, BIOS,
  PIC, cards, music, logs, captures, learned paths, generated guest-code
  source, or private paths. Current-installer and exact-v2457-updater
  acceptances both preserved user data, selected the canonical folder/runtime,
  migrated the byte-identical legacy name, and launched no game process.

- v2458 native executable SHA-256:
  `5636197D7CA5D3DEDEAA2FA7F016A2C6899516D5C8D30A74AD150636E53C49A2`.
  Compilation, the ordered-vs-spatial-switchback regression, 34
  restart/shutdown policies, 20 route-parser policies, 15 controller/music
  policies, link freshness, and the standalone no-firmware audit pass. No live
  v2458 race acceptance is claimed by this package checkpoint.
- v2458 public ZIP SHA-256:
  `D2DA3CFA1113E9DDA7DE88C224C4E92DC4E73540C4F44868FBB10E4A6B89CD0F`;
  audited size is 23,812,401 bytes. Its 18 entries include one byte-identical
  `demo.exe` transition copy required by the updater already shipped in v2457.
  On first v2458 launch it is retired only if it matches the canonical runtime.
  No game data, BIOS, PIC, cards, music, logs, captures, learned paths,
  generated guest code, or private paths are present.
- The actual v2458 archive passed two isolated no-game-launch acceptances. The
  current installer merged files, preserved protected and destination-only
  data, migrated the name, and verified the runtime hash. Separately, the exact
  updater and installer shipped in v2457 located the nested
  `Initial D Arcade Stage 3 Recompiled` folder, installed v2458, preserved user
  data, and handed off to the renamed runtime. Both left zero game processes.
- v2457 native executable SHA-256:
  `CCDF9A0C3E195C8845CBFF0D8224D07854855D47CAC9DE4452F6598BB1581F94`.
  Compilation, 34 restart/shutdown policies, 20 route-parser policies, 15
  controller/music policies, an exact synthetic learned-path round trip, link
  freshness, and the standalone no-firmware audit pass. The audit reports zero
  firmware callbacks, objects, input contracts, or cached translations.
- v2457 public ZIP SHA-256:
  `5E6785CAC48C1BFEA6509E0AAADF28C9B2F1639969806A85093D103081346A2C`;
  audited size is 18,576,649 bytes. Its 17 entries contain no game data, BIOS,
  PIC, cards, custom music, logs, captures, learned racing paths, generated
  guest code, or private paths.
- The v2457 Direct-session policy automatically clears a stale diagnostic
  marker when possible and proceeds with the saved Direct choice. If marker
  maintenance is unavailable it still proceeds with Direct; there is no
  automatic Safe fallback or retry prompt. Safe remains the first-run default
  and is used when selected. This policy passed source/offline checks; no live
  Direct-renderer acceptance run is claimed for v2457.
- A live isolated updater acceptance queried GitHub Releases, selected v2456
  from an older installation, downloaded 18,577,242 bytes, matched GitHub's
  published SHA-256, installed it, preserved game files/cards/music/logs/
  settings/update preference, and confirmed that the current version is not
  offered itself. The acceptance did not launch the game.
- v2454 native executable SHA-256:
  `4E9D607E2F475E38AF8A40BAF6DAE4D8F79CB62C57172D1BB0D1E6FCA065C013`.
  Its exact card-device suite covers 27 transactions, including an early
  reinsert request, authentic post-eject empty status, delayed selected-card
  queue, and a second capture/read cycle.
- A bounded v2454 Bunta result run wrote exactly 207 bytes, reported
  `reinsert=queued source=post-eject-removal`, logged zero recognized faults,
  and exited cooperatively with no remaining game process. Separate Bunta win
  and loss continuations and a three-minute no-input attract run likewise
  completed without a recognized crash or untranslated-runtime fault.
- v2454 public ZIP SHA-256:
  `041CBA8943BF7DF43F61B9843FC81C93C5C8107E2FAAF16E16CFA45B69B317AD`;
  audited size is 18,575,765 bytes. Its 17 files include the version marker
  and source-audited update helpers but no game data, BIOS, PIC, cards, custom
  music, logs, captures, generated guest code, or private paths.
- The updater selects the highest newer `vNNNN` prerelease, shows its version
  and release date, supports optional automatic installs, requires GitHub's
  SHA-256 asset digest, rejects traversal paths and mismatched version markers,
  preserves user-owned folders/settings, and rolls back a failed install.
  Offline update/install tests and the isolated moved-launcher test pass.
- v2449 identifies the attract sequence's authored black cinematic mattes by
  exact four-vertex mesh/material state and then requires projected native
  x=0..640 coverage before expanding them about the output centre. This keeps
  the fix out of HUD, cars, roads, and other presentation panels.
- A BIOS-free 2560x1080 attract acceptance captured twelve points from scene
  3300 through 3630. The initial and changing-zoom matte families reached both
  output edges while the Hor+ world and UI placement remained unchanged. The
  process exited cooperatively with code 0; no game process or Direct-session
  marker remained.
- Six matched native-resolution scenes from 3300 through 3600 have identical
  whole-BMP SHA-256 values between v2448 and v2449, demonstrating byte-for-byte
  640x480 preservation despite routing the two mask quads through CPU
  projection.
- A normal visible 2560x1080 Direct-Vulkan v2449 attract run without diagnostic
  readback held 119.5–120.5 FPS after warm-up (120.0 Vulkan average). During
  moving attract scenes, two-second buckets produced approximately 239–241
  distinct frames; the run logged no recognized faults and closed cleanly.
- The final v2449 executable SHA-256 is
  `1B1E1990A588DE804CCB6FF19D0D920F60E3EAC038655EA79D39780E5270F479`.
  It passed the complete controller, attached-Xbox, audio endpoint, AICA,
  custom-music, interpolation, Direct-session, card, ELAN, offscreen Vulkan,
  guest-timing, lifecycle, link-freshness, translation-integrity, and
  standalone no-firmware suite.
- v2448 distinguishes swapchain output (`OUT`) from genuinely distinct output
  (`NEW`) in the in-game FPS overlay. Both counters are sampled on the
  presentation thread and published atomically; authentic 60 Hz endpoints and
  interpolated midpoints count as distinct, while cadence repeats do not.
- The v2448 native executable SHA-256 is
  `AA068705DB38295967802DBEDAFC2E643A83119F763755D97428567C0DE1DC64`.
  It passed the complete controller, attached-Xbox, audio endpoint, AICA,
  custom-music, interpolation, Direct-session, card, ELAN, offscreen Vulkan,
  guest-timing, lifecycle, link-freshness, translation-integrity, and
  standalone no-firmware suite.
- A normal visible Direct-Vulkan v2448 run used a 2560x1080 internal render,
  authentic 60 Hz game timing, and 120 Hz presentation. Across 21 steady
  moving-race samples, display and Vulkan minimum/average were all 120.0 FPS,
  each two-second bucket contained at least 240 distinct frames, the repeat
  counter delta was zero, movement reached 129.204 m, faults were zero, exit
  code was 0, and the Direct-session marker was removed.
- The 120 Hz analyzer now anchors current logs to explicit player-motion and
  positive-travel telemetry instead of mistaking distinct animated menu frames
  for race activity. Its focused contract passes 12 cases and retains a
  generated-frame fallback for historical logs without motion telemetry.
- v2446 extends the developer-only course-selector snapshot to segmented
  primary tables. Its native executable SHA-256 is
  `BB453D859F3D7DFD505CAC24501E16B7D17DEBC44ED0B2A7D557574923CB58C8`.
  Product behavior is unchanged unless QA explicitly enables the diagnostic.
- The corrected evidence policy recognizes `_etc_f` as dry and `_etc_r` as
  wet/snow; neither token proves direction. Time Attack direction requires
  three independent guest selector fields matching the manifest. All 20
  focused contracts pass, including missing, partial, and contradictory
  fail-closed cases.
- Exact final-v2449/schema-5 evidence accepts all 62 defined Time Attack rows
  across all nine course families, including the full dry/wet, day/night, and
  left/right combinations exposed by the retained matrix plus both Akina Snow
  directions. Every row has the expected course/condition/time/weather assets,
  three agreeing direction fields, real player movement, zero recognized
  faults, and cooperative exit code 0.
- All eight Bunta routes also pass on the same final v2449 executable. Each
  loaded its game-fixed night/dry course state, applied 480 AI-driving polls,
  reached measurable player movement, reported zero recognized faults, and
  exited cooperatively.
- v2445 public ZIP SHA-256: `301741FEF79AC86CF56F85E9BC19E30D6DC4290833AE3CDF55596C4042F0BB0E`; audited size is 18,565,971 bytes.
- The versioned launcher maps Direct to the optimized native Vulkan swapchain
  path and Safe to the bounded GPU-readback/Win32 path. Both modes passed the
  same-helper dry-run contract, and all four deployed launchers parsed with
  zero PowerShell errors.
- v2445 reuses the accepted v2444 native executable byte-for-byte; no game or
  Vulkan process was launched for the launcher-only acceptance.
- v2444 native executable SHA-256: `49BEFEBCD2D23D97CF017EDB7F9F4E5E3685E23AA3994CA774EFE333EBCC4C5D`.
- v2444 public ZIP SHA-256: `BEFE5D96AD8467C63E2A767B920AC523447ED42C3C849A3B817E051456E24515`; audited size is 18,565,999 bytes.
- The ZIP contains 14 files and zero CHD, PIC, BIOS, extracted game asset,
  card, custom music, log, capture, generated guest translation, credential,
  or private filesystem path entries.
- Managed F1 card paths are persisted by filename when stored in the product's
  local `card data` folder. All three launcher variants repair old absolute
  selections after a demo folder move; their PowerShell parsers report zero
  errors.
- v2444 passed the complete controller, attached-Xbox, AICA mailbox/SGC,
  Windows endpoint, exact custom-music, interpolation, Direct-session marker,
  ELAN/card/offscreen Vulkan, guest timing, policy, translation-integrity,
  link-freshness, and standalone product suite. The standalone audit found
  zero firmware callbacks, firmware AOT objects, firmware input contracts, or
  cached firmware translations.
- The v2443 180-second null-sink cold run produced 7,080,718 non-silent
  post-mix frames, peak magnitude 32768, mean absolute magnitude 5125.22, and
  zero AICA dropped samples before cooperative exit code 0.
- v2442 native executable SHA-256: `B56F179C53D3238A6BF311F4E351F2AEE674C8E58397776445DC6F11120C046C` (binary intentionally not published).
- The product and hardware probe now share one XInput runtime loader and state
  normalizer. On the current Windows host the probe loaded `xinput1_4.dll`,
  found one real controller in slot 0, and mapped its neutral sample to steering
  32768, accelerator 0, and brake 0.
- F1 controller navigation and binding capture continue to run while guest
  execution is paused. Axis capture now accepts deliberate 35% travel instead
  of requiring more than half travel, while button edges retain priority.
- v2442 passed controller normalization/capture, product-shared hardware probe,
  custom music, interpolation, Direct-session marker, ELAN/card/offscreen
  Vulkan, guest timing, 14 controller/music policies, 31 lifecycle policies,
  link freshness, translation integrity, and standalone no-firmware checks.
  The standalone audit again found zero firmware callbacks, AOT objects, input
  contracts, or cached firmware translations.
- v2441 native executable SHA-256: `78FFA018D058744A5AA5AAF5F5580C09F313CB8F5725A744AD0BFBCD6E30C6DD` (binary intentionally not published).
- Seven fresh-process samples of the warmed 16,384-vertex reuse case reduced
  median batch-loop time from 0.089 ms to 0.066 ms, topology time from 0.064 ms
  to 0.041 ms, and complete synthetic frame time from 0.696 ms to 0.629 ms.
- v2441 passed the complete v2440 offline suite plus an explicit BGR24/BGRA32
  presenter channel-accuracy contract. Its standalone audit remains at zero
  firmware callbacks, AOT objects, input contracts, and cached translations.
- v2441 CPU-only cold-boot probes passed Myogi left/dry/day Time Attack with
  9.548 m of player movement and Shomaru dry/night Bunta with 5.097 m. Both
  identified the expected course/condition/environment assets and exited 0
  through cooperative close without creating a Vulkan WSI surface.
- A complementary Usui right/wet/night Time Attack probe loaded the expected
  course, reverse condition geometry, direction identity `1,1,1`, night
  environment, and rain; it advanced 9.008 m and exited cooperatively with 0.
- Tsuchisaka dry/night Bunta likewise loaded `k_tu2`, its forward condition
  geometry, night/no-rain environment, and the Bunta rival package; it advanced
  6.302 m and exited cooperatively with 0.
- Four fresh v2441 Direct-Vulkan processes covered Myogi, Akina, and Akagi at
  640x480 plus Akagi at 2560x1080. Every steady race interval recorded zero
  exact fallback presentations and zero faults. The ultrawide run held 120.0
  FPS minimum/average for both display and Vulkan across 35 complete samples,
  with at least 120 generated motion phases in every sample. All four processes
  exited cooperatively with code 0 and removed their Direct-session markers.
- v2440 native executable SHA-256: `7FB61B45A0137F8FCE4A9A6CD36200B212F7DFDC6717D851739AD9CBB4D798BC` (binary intentionally not published).
- The complete offline suite passed CPU/Vulkan pixel comparison, topology,
  projection, ELAN lighting, environment mapping, homogeneous near clipping,
  16,384-static-vertex reuse, controllers, custom music, interpolation, cards,
  Direct-session marker lifecycle, 31 shutdown/presentation policies, 13
  controller/music policies, guest timing, link freshness, and the no-firmware
  standalone audit.
- A live 2560x1080 RX 9070 XT Direct Vulkan run sustained 119.7–120.3 FPS
  across roughly 75,000–127,000 vertices per authentic course frame. During
  moving content the generated-frame count advanced at the required cadence
  and the repeated-frame counter stayed effectively flat.
- The v2440 process crossed the result/continue transition, stabilized near a
  1.34 GiB working set with zero handle growth in the steady sample, exited 0,
  completed renderer/audio/DirectInput/window teardown, and removed its
  Direct-session marker.

- v2415 native executable SHA-256: `3185FF67F00EE5E8EF17E63F5B10ECDF66494B3B604358ED7DA863C7AB56B8D4`.
- Public ZIP SHA-256: `8CA9D017EA5BCFE8EB58245D162D89A22D75267BC6D3BABAD76D922E283B2E8E`; local audited size is 17,877,033 bytes.
- The exact public ZIP was audited as 14 files with no CHD, PIC, BIOS, extracted HOSTFS, card save, custom song, user log, or personal filesystem path.
- v2415 linked 116 objects, passed freshness checks for 108 source owners, and passed 13 controller/music plus 16 lifecycle policy tests, controller-binding/smoothing, exact custom-music, interpolation, ELAN/card/AICA/offscreen Vulkan, card-eject, translation, freshness, and no-firmware checks.
- v2415 CPU-only same-build probes passed Myogi left/dry/day Time Attack and the Shomaru and Tsuchisaka Bunta routes with correct course/time/weather assets, real motion, zero fault markers, one process, exit code 0, and complete shutdown markers.
- Current broader coverage is 48/48 route/branch loads, 32/32 rival movement, 16/16 natural Time Attack results, and 32/32 natural rival results. The separate 70-row Time Attack/Bunta load-and-movement ledger mixes checkpoints and is not a full same-build v2415 completion matrix.

- v2396 Windows x64 checkpoint SHA-256: `966F218E54CC2C3A112D174163C85B141DDA25FE96836729D310E8773C555E91` (binary intentionally not published).
- One BIOS-free 640×480/60 bounded route proved the opponent-menu label, stream override, Media Foundation decode, and AICA replacement activation in a single run while Vulkan held 60.0 Hz and live movement advanced.
- The build verifier now rejects a translated COMDAT owner that predates the inline host headers it owns. The historical stale build fails with the exact owner name; v2396 passes fresh.
- The v2396 checkpoint closed 48/48 route/branch loads, 32/32 rival movement, 16/16 natural Time Trial results, and 32/32 natural rival results.
- The accepted four-course high-refresh matrix measured 119.8–120.0 FPS minimum presentation and no repeated endpoints in accepted moving intervals.

- v2374 Windows x64 checkpoint SHA-256: `81F18BBEA04D42B692BE295630A3F6A6DE8DF6F479ECF4881E38FCAC100FD0C4` (binary intentionally not published).
- On the identical resolved-input/card `k_ez` route, v2373 measured 100.0 minimum / 116.609 average generated FPS across 22 full race samples. The exact main-RAM object-clear repair raised v2374 to 120.0 minimum / 120.0 average across 23 full samples.
- The moving-race repeat-counter delta was zero in both runs; v2374's 120 FPS result is distinct interpolated motion, not repeated 60 Hz endpoints.
- The exact linked memset regression passes zero, one-byte, unaligned 257-byte, and P1-alias 64 KiB cases with preserved registers, T flag, stack, guard bytes, and `length + 2` memory-access cycles.
- The standalone audit reports zero firmware callbacks, firmware AOT objects, firmware input contracts, or cached firmware translations in v2374.

- v2217 Windows x64 checkpoint SHA-256: `0F225655BF9F32C1FAEE76CF559EABD1D7D5C19A4960BD7A02B512582A4A08E6` (binary intentionally not published).
- At 3840×2160, 43 samples from takeover onward measured 59.8–60.1 FPS while genuine JVS/physics-driven race motion advanced 668.687 m.
- The v2216 29.3/51.2 transition samples were traced to the 60 Hz presenter waiting for new guest scenes during synchronous loads. v2217 retains the last completed image at fixed wall-clock cadence without advancing guest code.
- At 1920×1080, all eight sampled race intervals from takeover through 160 m measured 119.8–120.1 FPS using presentation interpolation over authentic guest timing.
- Vulkan prewarming moved seven observed race-only pipelines to startup. The two heaviest entry frames dropped from 20.97/19.01 ms to 11.36/5.69 ms and created zero pipelines in-frame.
- The final acceptance runs recorded no fatal, exception, access-violation, unimplemented-target, NSEQ, or Vulkan fault markers.
- N70 frame SHA-256: `34D6B91C0550A5CC2A60D4B1F8930812908CEA6DCD6C354749A0881BE426D9E2`.
- Accepted N70 state: 667 batches, 61,498 vertices, 17,490 triangles, 990,196 textured pixels, and 53,996 lit vertices.
- Full-intro ladder reached N3200 with each recorded run accepted N/N, `unimplemented=0`, `FPSCR=0`, and no recorded memory fault, crash, or watchdog.
- Current public classifier smoke test covers recursive links, SH-4/ERAM texture DMA links, all five known list-complete wait bits, empty/unknown-mask rejection, and false command-header aliases.

## Latest WIP truth

The earlier linked-stream/frame-lifecycle and attract-mode-only blockers are no
longer the active frontier. The v2454 package uses the v2454 native build,
which advances targeted
menu-to-race and result/save paths, presents retained NAOMI 2 scenes
continuously, and is available as a public early-demo prerelease that prepares
data from matching user-owned inputs without a BIOS or Python installation.
It also repairs delayed card reuse after result-screen ejection and introduces
a fail-safe GitHub release updater whose network failure cannot block launch.

Recent visual work separated two different causes. Missing ISO9660-truncated
course lookup names were restored from the user's own dump, removing the
alternating rainbow/static environment maps. A later BIOS-free attract capture
proved that two authored black cinematic mattes were still fixed to the
centred 640x480 viewport at 2560x1080. v2449 expands only the exact full-native-
width matte mesh to the output edges and passed both fixed-zoom and changing-
zoom captures. Earlier work also corrected tested HUD projection, mirror
placement, RX-7 geometry/texture failures, and renderer lifecycle defects. This
does not imply that all graphics or gameplay paths are complete.

The current priority frontier is isolating remaining terrain/HUD edge artifacts,
reducing remaining CPU command preparation, broad physical-input acceptance,
repeated cross-driver shutdown validation, whole-game same-build coverage,
remaining visual/audio defects, synchronous transition latency, broader
clean-machine packaging, and eventually uncapped presentation that stays
independent from guest gameplay timing. The measured high-refresh routes do
not make 120 Hz a whole-game validated mode. The entire 70-row condition
ledger is now exact final-v2449/schema-5 evidence: 62 Time Attack rows and all
eight Bunta routes. The remaining coverage frontier is full race completion,
results/persistence, cars/opponents, visual and audible correctness, campaign
branches, hardware breadth, and repeated live-renderer stress rather than
route-state renewal.

Exact-v2453 full-race renewal now has 9/32 retained rival profiles: Evo 5,
Evo 6, Keisuke rematch, Kyoko, Ryosuke rematch, Sakamoto, Smiley, Sudo rematch,
and Wataru.
Each moved through ordinary guest physics/input and reached a natural
game-owned result, with zero target outcome requests after the target gate
armed, zero recognized fault signals, 60.0 FPS average display/Direct Vulkan
presentation, and cooperative clean shutdown. The exact executable SHA-256 is
`4297DD4906E5A12CD474BC4496E7CF76F907E17CBBB19F92CD6178A09FB3FBD8`.
The older accepted 32/32 ledger remains useful historical evidence, but it is
not being presented as a same-build v2453 pass.

## Definition of release-ready

The downloadable build is an early playtest, not a finished release. The
project will not call itself release-ready until users can cold boot from
legally owned inputs, reach and control every supported mode with working
audio/video/input, complete meaningful gameplay and persistence branches, and
do so repeatedly through the static native path without an interpreter/JIT
fallback or host-level failure.
