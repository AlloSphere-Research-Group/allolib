#!/usr/bin/env bash
# Restore vendored submodules to the recorded upstream SHAs (drops local patches).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

revert_one() {
  local rel="$1"
  local expected="$2"
  local dir="${ROOT}/${rel}"

  if [[ ! -d "${dir}/.git" && ! -f "${dir}/.git" ]]; then
    echo "error: missing submodule ${rel}" >&2
    exit 1
  fi

  git -C "${dir}" checkout --force "${expected}" -- .
  git -C "${dir}" clean -fdq
  echo "reverted: ${rel} -> ${expected}"
}

revert_one modules/external/Gamma 615eea7467bec22c572952e200e4fa1ccd0ab337
revert_one modules/audio-backend-rtaudio/external/rtaudio 46b01b5b134f33d8ddc3dab76829d4b1350e0522
revert_one modules/midi/external/rtmidi 806e18f575b68c23b26f9398e1b6866b335b5308
revert_one modules/config/external/cpptoml fc4adc4a5b6038f8c36063369531677f136d2187

echo "vendor working trees restored (submodule pointers unchanged)"
