#!/usr/bin/env bash
set -euo pipefail
repository=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)
libraries=${1:?built Java library directory required}
test_classes=${2:?compiled test classes required}
output=$(mktemp -d)
trap 'rm -rf -- "$output"' EXIT
source_root="$repository/minijvm/c"
mapfile -t sources < <(find "$source_root/jvm" "$source_root/utils" "$source_root/utils/https" "$source_root/utils/https/mbedtls/library" -maxdepth 1 -name '*.c')
cc -O2 -std=gnu11 -w \
  -I "$source_root/jvm" -I "$source_root/utils" -I "$source_root/utils/sljit" \
  -I "$source_root/utils/https" -I "$source_root/utils/https/mbedtls/include" -I "$source_root/utils/mimalloc/include" \
  "${sources[@]}" "$source_root/utils/sljit/sljitLir.c" "$source_root/utils/mimalloc/src/static.c" "$source_root/main.c" \
  -pthread -lm -ldl -o "$output/mini_jvm"
run_test() {
  local class=$1 marker=$2
  local result=0
  timeout 20s "$output/mini_jvm" -Xmx128M -bootclasspath "$libraries/minijvm_rt.jar" \
    -cp "$test_classes:$libraries/freej2meonminijvm.jar:$libraries/glfw_gui.jar:$libraries/xgui.jar" "$class" > "$output/result.log" 2>&1 || result=$?
  cat "$output/result.log"
  if [[ "$result" != 0 ]]; then echo "VM test $class exited $result" >&2; return 1; fi
  # miniJVM historically exits zero for an uncaught Java exception.
  rg_or_grep=${RG:-grep}
  "$rg_or_grep" -q "$marker" "$output/result.log"
}
run_test ClassInitializationTest CLASS_INITIALIZATION_VERIFIED
run_test ClassInitializationReentryTest CLASS_REENTRY_DISPATCH_VERIFIED
run_test com.ebsee.emu.RectangleCompatibilityTest RECTANGLE_INTERSECTION_VERIFIED
