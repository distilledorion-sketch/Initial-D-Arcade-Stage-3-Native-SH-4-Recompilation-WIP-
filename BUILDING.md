# Building the public checks

The public repository has two independent checks. Neither requires game data.

## Python translator tests

Requirements: Python 3.10 or newer.

```bash
python -m unittest discover -s tests -v
```

## Native command-classifier/walker test

Requirements: CMake 3.20 or newer and a C++17 compiler.

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Why there is no full game build command

The complete integration build consumes private, legally owned inputs and generates derived C++ translation units that are not part of this public repository. Publishing a command that appears to produce a game from missing data would be misleading. A future release workflow will accept user-supplied inputs locally and keep them outside version control.
