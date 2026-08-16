# Temporary vendor patches (do not upstream)

Shelved for when `ALLOLIB_INSTALL=ON` (install / `find_package(AlloLib)`).
Everyday source-tree builds leave install **off** and do **not** need these.

AlloLib can **bundle** vendored libraries into `AlloLibBundledTargets` so
`find_package(AlloLib)` + `al::app` works without system packages.

CMake forbids a target appearing in more than one `install(EXPORT …)` set.
Gamma, RtAudio, RtMidi, and cpptoml each register their own export. These
patches wrap that in `if(NOT ALLOLIB_BUNDLE_DEPS)` so AlloLib can own the
export while `ALLOLIB_BUNDLE_DEPS=ON`.

This is **temporary**. When deps move to vcpkg/Conan (or similar), delete
this directory and resolve those libraries with `find_dependency()` in
`cmake/AlloLibDependencies.cmake`. Do not commit the patched submodule
trees.

## When to use

```bash
# After submodule init, only if you want the install package:
./cmake/vendor-patches/apply.sh
cmake -S . -B build -DALLOLIB_INSTALL=ON
```

`apply.sh` is idempotent (`git apply --check` / `--reverse --check`).
`revert.sh` restores the recorded submodule SHAs (clean upstream).

Without patches, `-DALLOLIB_INSTALL=ON` will fail at generate time with
“exported multiple times” for rtaudio/rtmidi/cpptoml/Gamma.
