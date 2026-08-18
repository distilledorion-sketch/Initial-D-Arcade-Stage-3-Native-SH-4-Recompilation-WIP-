# Course environment lookup recovery

Status: validated in the private native integration build on 2026-08-18.

## Symptom

Alternating environment maps contained bright rainbow/static pixels while the
companion maps were coherent dark road/environment textures. Changing the host
window cap between 30 and 60 FPS did not create or repair the corruption.

## Root cause

The original SH-4 course loader requests these logical paths:

```text
/driveA/env/k_df1_path_env.bin
/driveA/path/k_df_sdw.bin
```

The private HOSTFS did not contain them because the physical ISO9660 Level-1
names are truncated. The first lookup pointer stayed null, so the guest
compositor read unrelated low-memory bytes as selector values.

Before recovery, the affected descriptor observed a null table and the invalid
selector bytes `90,62,04,01`. After restoring the files from the same
user-owned GDS-0033/PIC pair, it observed a valid table and the course selector
record `05,00,02,00`.

## Exact source mapping

```text
/ENV/K_DF1_PA.BIN   extent=0x000002DC -> HOSTFS/env/k_df1_path_env.bin
/PATH/K_DF_SDW.BIN  extent=0x00000A56 -> HOSTFS/path/k_df_sdw.bin
```

Both physical files are 16,356 bytes. The public preparation utility verifies
their complete SHA-256 hashes before writing a new private overlay.

## Result and boundary

The corrected private run produces coherent environment maps and a stable
single-car camera sequence at both 30 and 60 FPS presentation targets. This
closes that specific corruption; it does not claim that the complete renderer,
intro, or game is finished.

No disc image, BIOS, PIC, extracted asset, generated game translation, memory
snapshot, reference capture, or rendered game frame is included here.
