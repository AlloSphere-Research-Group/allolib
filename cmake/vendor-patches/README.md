# Temporary vendor patches (do not upstream)

AlloLib currently **bundles** a few vendored libraries into
`AlloLibBundledTargets` so `find_package(AlloLib)` + `al::app` works.

CMake forbids a target appearing in more than one `install(EXPORT …)` set.
Gamma, RtAudio, RtMidi, and cpptoml each register their own export. These
patches wrap that in `if(NOT ALLOLIB_BUNDLE_DEPS)` so AlloLib can own the
export while `ALLOLIB_BUNDLE_DEPS=ON`.

This is **temporary**. When deps move to vcpkg/Conan (or similar), delete
this directory and resolve those libraries with `find_dependency()` in
`cmake/AlloLibDependencies.cmake`. Do not commit the patched submodule
trees.

## Apply / revert

From the AlloLib repo root (after `git submodule update --init --recursive`):

```bash
./cmake/vendor-patches/apply.sh
./cmake/vendor-patches/revert.sh
```

`apply.sh` is idempotent (`git apply --check` / `--reverse --check`).
`revert.sh` restores the recorded submodule SHAs (clean upstream).

Configure still succeeds without the patches; **exporting `al::app`**
(`cmake` generate of the install package) needs them applied.
