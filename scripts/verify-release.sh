#!/usr/bin/env bash
# Verify a release-artifact directory contains the complete, correct set.
#
# Usage: scripts/verify-release.sh <dir> <version>
#
# Requires all of:
#   goshpad-v<ver>-linux-x86_64.tar.gz
#   goshpad-v<ver>-linux-aarch64.tar.gz
#   goshpad-v<ver>-linux-x86_64.flatpak
#   goshpad-v<ver>-linux-aarch64.flatpak
#   SHA256SUMS
set -euo pipefail

dir="${1:?usage: verify-release.sh <dir> <version>}"
version="${2:?usage: verify-release.sh <dir> <version>}"
fail() { echo "verify-release: $*" >&2; exit 1; }

cd "$dir"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

for arch in x86_64 aarch64; do
    tb="goshpad-v${version}-linux-${arch}.tar.gz"
    fp="goshpad-v${version}-linux-${arch}.flatpak"
    case "$arch" in x86_64) elf='x86-64' ;; *) elf='aarch64' ;; esac

    [ -s "$tb" ] || fail "missing or empty: $tb"
    [ -s "$fp" ] || fail "missing or empty: $fp"

    tar -xzf "$tb" -C "$tmp" ||
        fail "$tb is not a valid gzip tarball"
    bin="$tmp/goshpad-v${version}-linux-${arch}/bin/goshpad"
    [ -x "$bin" ] || fail "$tb lacks executable bin/goshpad"
    file "$bin" | grep -q "$elf" ||
        fail "$tb contains wrong-arch binary: $(file "$bin")"

    grep -aqm1 "app/com.goshapps.GoshPad/$arch/stable" "$fp" ||
        fail "$fp is not arch $arch"
    echo "ok: $tb ($arch binary verified)"
    echo "ok: $fp (arch $arch verified)"
done

[ -s SHA256SUMS ] || fail "missing or empty: SHA256SUMS"
sha256sum -c SHA256SUMS ||
    fail "SHA256SUMS does not match the artifacts"

count="$(find . -maxdepth 1 -type f | wc -l)"
[ "$count" -eq 5 ] || fail "expected 5 files, found $count"

echo "release artifact set complete: v$version (x86_64 + aarch64)"
