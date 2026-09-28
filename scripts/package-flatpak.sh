#!/usr/bin/env bash
# Build the Flatpak and produce dist/goshpad-v<ver>-linux-<arch>.flatpak
#
# Usage: scripts/package-flatpak.sh <version> <x86_64|aarch64>
#
# Requires flatpak, flatpak-builder, and the Flathub remote. The runtime is
# org.kde.Platform//6.9 from com.goshapps.GoshPad.json.
set -euo pipefail
cd "$(dirname "$0")/.."

version="${1:?usage: package-flatpak.sh <version> <arch>}"
arch="${2:?usage: package-flatpak.sh <version> <arch>}"

flatpak install --user -y --noninteractive flathub \
    org.kde.Platform//6.9 \
    org.kde.Sdk//6.9

flatpak-builder --user --repo=repo --install-deps-from=flathub \
    --force-clean --state-dir="$PWD/.flatpak-builder" \
    build-flatpak com.goshapps.GoshPad.json

mkdir -p dist
out="dist/goshpad-v${version}-linux-${arch}.flatpak"
flatpak build-bundle --arch="$arch" repo "$out" com.goshapps.GoshPad stable

grep -aqm1 "app/com.goshapps.GoshPad/$arch/stable" "$out" ||
    { echo "flatpak bundle is not arch $arch" >&2; exit 1; }

echo "wrote $out"
