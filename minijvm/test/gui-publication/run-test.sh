#!/usr/bin/env bash
set -euo pipefail

test_root=$(cd "$(dirname "$0")" && pwd)
lib_root=$(cd "${1:?Usage: run-test.sh <built runtime lib directory>}" && pwd)
classes=$(mktemp -d)
trap 'rm -rf "$classes"' EXIT

# Use the actual built GApplication; only the callback records constructor escape.
javac -source 8 -target 8 -encoding UTF-8 -cp "$lib_root/*" -d "$classes" \
  "$test_root/org/mini/gui/callback/GCallBack.java" "$test_root/GuiPublicationTest.java"
java -cp "$classes:$lib_root/*" GuiPublicationTest
