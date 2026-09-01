# Current integration checkpoint — v2442

Date: 2026-09-01  
Scope: controller discovery/remapping acceptance on the v2441 renderer base  
Native executable SHA-256: `B56F179C53D3238A6BF311F4E351F2AEE674C8E58397776445DC6F11120C046C`

The executable is intentionally not published in the source tree. This page
records source-safe engineering evidence only.

## What changed

- XInput DLL loading, device discovery, and normalization now live in one
  shared component used by both the native product and a no-window hardware
  probe. A probe success therefore exercises the product's real discovery and
  conversion code rather than a separate approximation.
- The F1 Controls page accepts D-pad or left-stick navigation, A to activate a
  row, and B to back out. A baseline sample on menu entry prevents a control
  already held during F1 opening from immediately activating a setting.
- Axis remapping now recognizes deliberate movement after 35% normalized
  travel. The previous greater-than-50% requirement could make large steering
  wheels and partial trigger movement appear undetected.
- Binding capture logs its selected provider, action, baseline availability,
  and threshold before waiting for movement, making future user logs useful
  when hardware-specific failures occur.
- XInput diagnostics now identify the loaded system DLL as well as connected
  slots.

## Accepted evidence

On the current Windows host, with no game or Vulkan window launched, the
product-shared probe reported:

- XInput runtime available through `xinput1_4.dll`;
- one connected controller in slot 0;
- a successful state packet read;
- normalized neutral left/right sticks and independent triggers;
- cabinet steering 32768, accelerator 0, and brake 0; and
- `product-shared-path=1` with exit code 0.

The controller binding regression also passed button capture, full positive and
negative steering, independent trigger normalization, steering smoothing, and
the new partial-travel axis capture contract.

The complete v2442 off-screen suite then passed custom music, interpolation,
Direct-session marker lifecycle, ELAN/card behavior, offscreen Vulkan, guest
timing, controller/music policies, lifecycle/shutdown policies, link freshness,
translation integrity, and the standalone product audit. The standalone audit
found zero firmware callbacks, firmware AOT objects, firmware input contracts,
or cached firmware translations.

No game process remained after acceptance and this checkpoint did not create a
Win32 Vulkan presentation surface.

## Important limits

This result proves that this host's attached Xbox controller is found and read
through the exact product code. It does not yet prove every live movement and
binding inside a running game, multiple controller models, wireless reconnects
on physical hardware, DirectInput wheel behavior, or force feedback. Those
remain explicit physical acceptance work.

This checkpoint also does not add new course, graphics, audio, or uncapped-rate
coverage. The v2441 2560x1080/120 Hz evidence remains the latest live renderer
acceptance result.

## Public-data boundary

No game image, BIOS, security PIC, CHD, extracted asset, generated guest
translation, card save, custom music, log, frame capture, credential, or local
filesystem path is included in this checkpoint.
