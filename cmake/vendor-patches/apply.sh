#!/usr/bin/env bash
# Apply temporary AlloLib vendor patches so bundled install(EXPORT) works.
# Does not commit submodule repos. Revert with revert.sh.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PATCH_DIR="$(cd "$(dirname "$0")" && pwd)"

apply_one() {
  local rel="$1"
  local patch="$2"
  local expected="$3"
  local dir="${ROOT}/${rel}"
  local patch_path="${PATCH_DIR}/${patch}"

  if [[ ! -d "${dir}/.git" && ! -f "${dir}/.git" ]]; then
    echo "error: missing submodule ${rel} (run git submodule update --init --recursive)" >&2
    exit 1
  fi

  local head
  head="$(git -C "${dir}" rev-parse HEAD)"
  if [[ "${head}" != "${expected}" ]]; then
    echo "warning: ${rel} HEAD is ${head}, expected ${expected}; trying patch anyway" >&2
  fi

  if git -C "${dir}" apply --reverse --check "${patch_path}" >/dev/null 2>&1; then
    echo "already applied: ${rel}"
    return 0
  fi

  if ! git -C "${dir}" apply --check "${patch_path}"; then
    echo "error: ${patch} does not apply to ${rel}" >&2
    exit 1
  fi

  git -C "${dir}" apply "${patch_path}"
  echo "applied: ${rel} (${patch})"
}

apply_one modules/external/Gamma gamma-bundle-export.patch 615eea7467bec22c572952e200e4fa1ccd0ab337
apply_one modules/audio-backend-rtaudio/external/rtaudio rtaudio-bundle-export.patch 46b01b5b134f33d8ddc3dab76829d4b1350e0522
apply_one modules/midi/external/rtmidi rtmidi-bundle-export.patch 806e18f575b68c23b26f9398e1b6866b335b5308
apply_one modules/config/external/cpptoml cpptoml-bundle-export.patch fc4adc4a5b6038f8c36063369531677f136d2187

echo "vendor patches applied (working trees only; not committed)"
