#!/usr/bin/env bash
set -euo pipefail
repository=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)
source_root="$repository/minijvm/c"
output=$(mktemp -d)
trap 'rm -rf -- "$output"' EXIT
mapfile -t sources < <(find "$source_root/jvm" "$source_root/utils" "$source_root/utils/https" "$source_root/utils/https/mbedtls/library" -maxdepth 1 -name '*.c')
cc -O2 -std=gnu11 -w -ffunction-sections -fdata-sections \
  -I "$source_root/jvm" -I "$source_root/utils" -I "$source_root/utils/sljit" \
  -I "$source_root/utils/https" -I "$source_root/utils/https/mbedtls/include" -I "$source_root/utils/mimalloc/include" \
  "${sources[@]}" "$source_root/utils/sljit/sljitLir.c" "$source_root/utils/mimalloc/src/static.c" \
  "$repository/minijvm/test/native/io_safepoint_test.c" \
  -Wl,--gc-sections -Wl,--wrap=fopen -pthread -lm -ldl -o "$output/io-test"
timeout 10s "$output/io-test"
