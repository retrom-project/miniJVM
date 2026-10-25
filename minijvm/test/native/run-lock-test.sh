#!/usr/bin/env bash
set -euo pipefail
repository=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)
source_root="$repository/minijvm/c"
output=$(mktemp -d)
trap 'rm -rf -- "$output"' EXIT
cc -O2 -std=gnu11 -ffunction-sections -fdata-sections -w \
  -I "$source_root/jvm" -I "$source_root/utils" \
  "$repository/minijvm/test/native/vm_lock_test.c" "$source_root/jvm/jvm_util.c" \
  "$source_root/utils/tinycthread.c" -Wl,--gc-sections -pthread -o "$output/lock-test"
timeout 5s "$output/lock-test"
