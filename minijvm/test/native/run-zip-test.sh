#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)
output=$(mktemp -d)
trap 'rm -rf -- "$output"' EXIT
utils="$repo/minijvm/c/utils"
cc -std=gnu11 -O2 -D__EMSCRIPTEN__ -ffunction-sections -fdata-sections \
  -I "$utils" "$repo/minijvm/test/native/zip_case_test.c" \
  "$utils/miniz_wrapper.c" "$utils/miniz.c" "$utils/bytebuf.c" "$utils/tinycthread.c" \
  -Wl,--gc-sections -pthread -lm -o "$output/zip-case-test"
"$output/zip-case-test" "$output/sample.zip"
