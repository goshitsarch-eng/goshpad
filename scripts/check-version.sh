#!/usr/bin/env bash
# Fail if a release tag disagrees with the version recorded in the source.
#
# Usage: scripts/check-version.sh v4.0.0
set -euo pipefail
cd "$(dirname "$0")/.."

tag="${1:?usage: scripts/check-version.sh <tag>}"
version="${tag#v}"
fail() { echo "version check failed: $*" >&2; exit 1; }

case "$version" in
    ''|*[!0-9.]*) fail "tag $tag is not a vX.Y.Z version" ;;
esac

cmake_version="$(sed -n 's/^project(goshpad VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt | head -n1)"
[ "$version" = "$cmake_version" ] ||
    fail "tag $tag but CMakeLists.txt says $cmake_version"

grep -q "<release version=\"$version\"" data/com.goshapps.GoshPad.metainfo.xml ||
    fail "metainfo has no <release version=\"$version\"> entry"

grep -q "Current release: \*\*$version\*\*" README.md ||
    fail "README does not say \"Current release: **$version**\""

echo "version $version consistent: tag, CMakeLists.txt, metainfo, README"
