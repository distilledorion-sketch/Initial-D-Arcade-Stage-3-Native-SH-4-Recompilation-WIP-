# Building the public checks

The public repository has two independent checks. Neither requires game data.

## Python translator tests

Requirements: Python 3.10 or newer.

```bash
python -m unittest discover -s tests -v
```

## Native runtime tests

Requirements: CMake 3.20 or newer and a C++17 compiler.

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

This builds the public ELAN command-classifier check and the provider-neutral
controller normalization, binding-capture, arcade-range, and steering-smoothing
test.

## Public demo versus the source checks

The complete integration build consumes private, legally owned inputs and generates derived C++ translation units that are not part of this public repository. Publishing a command that appears to produce a game from missing data would be misleading. A future release workflow will accept user-supplied inputs locally and keep them outside version control.

The [v2445 public early demo](https://github.com/distilledorion-sketch/Initial-D-Arcade-Stage-3-Native-SH-4-Recompilation-WIP-/releases/tag/v2445-direct-persistence)
provides the Windows launcher/runtime separately. It verifies and extracts from
the user's own matching `gds-0033.chd` and `317-0384-com.pic` on that machine;
it does not make the omitted generated translation units part of this Git
source tree. The demo launcher requires no Python installation.

## Verify the course lookup recovery

The source-safe utility below checks two ISO9660-truncated course files directly
inside a user-owned GDS-0033 dump and matching security PIC. It can verify the
inputs without writing anything:

```bash
python tools/prepare_v1023_course_lookup_hostfs.py \
  --base-drivea /path/to/private/driveA_v1022 \
  --tools /path/to/private/extraction-tools \
  --bin /path/to/user-owned/gds-0033.bin \
  --pic /path/to/user-owned/317-0384-com.pic \
  --verify-only
```

To create a new private HOSTFS overlay, replace `--verify-only` with
`--output-drivea /path/to/private/driveA_v1023`. The tool refuses to overwrite
an existing output and verifies the exact source extents, sizes, and hashes.
