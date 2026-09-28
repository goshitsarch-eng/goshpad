#!/usr/bin/env bash
# Packaging identity checks for GoshPad 4.0.0.
set -euo pipefail
cd "$(dirname "$0")/.."

fail() { echo "packaging check failed: $*" >&2; exit 1; }

cmake_text="$(<CMakeLists.txt)"
metainfo="$(<data/com.goshapps.GoshPad.metainfo.xml)"
desktop="$(<data/com.goshapps.GoshPad.desktop)"
manifest="$(<com.goshapps.GoshPad.json)"
readme="$(<README.md)"
copyright="$(<COPYRIGHT)"
justfile="$(<justfile)"

grep -q 'project(goshpad VERSION 4.0.0' <<<"$cmake_text" || fail "CMake version is not 4.0.0"
grep -q '<release version="4.0.0"' <<<"$metainfo" || fail "metainfo has no 4.0.0 release"
grep -q 'Current release: \*\*4.0.0\*\*' <<<"$readme" || fail "README current release is not 4.0.0"
grep -q '<id>com.goshapps.GoshPad</id>' <<<"$metainfo" || fail "metainfo id"
grep -q 'Exec=goshpad %F' <<<"$desktop" || fail "desktop Exec"
grep -q 'Icon=com.goshapps.GoshPad' <<<"$desktop" || fail "desktop icon"
grep -q 'GoshPad' <<<"$copyright" || fail "COPYRIGHT name"
grep -q 'Copyright © 2026 Gosh and GoshPad contributors.' <<<"$copyright" || fail "COPYRIGHT line"
grep -q 'share' <<<"$justfile" && grep -q 'licenses' <<<"$justfile" || fail "justfile license install"
grep -q 'LICENSE' <<<"$justfile" && grep -q 'COPYRIGHT' <<<"$justfile" || fail "justfile license files"

python3 - "$manifest" <<'PY' || fail "manifest invariants"
import json, sys
value = json.loads(sys.argv[1])
assert value["app-id"] == "com.goshapps.GoshPad"
assert value["runtime"] == "org.kde.Platform"
assert value["runtime-version"] == "6.9"
assert value["sdk"] == "org.kde.Sdk"
assert value["command"] == "goshpad"
assert value["branch"] == "stable"
finish = value["finish-args"]
assert finish == [
    "--share=ipc",
    "--socket=fallback-x11",
    "--socket=wayland",
    "--talk-name=org.freedesktop.portal.Desktop",
], finish
assert not any("filesystem=" in arg for arg in finish)
assert "com.system76.Cosmic.BaseApp" not in json.dumps(value)
assert "cargo" not in json.dumps(value)
PY

grep -q 'install(FILES LICENSE COPYRIGHT' <<<"$cmake_text" || fail "CMake does not install LICENSE and COPYRIGHT"
! grep -q 'com.goshapps.Notepad' <<<"$desktop$manifest$justfile$cmake_text" || fail "old app id survived packaging files"
! grep -q 'libcosmic\|cosmic-config\|i18n-embed' <<<"$cmake_text$manifest$justfile" || fail "old toolkit reference"

echo "packaging checks passed"
