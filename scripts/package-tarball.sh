#!/usr/bin/env bash
# Build the release binary and package dist/goshpad-v<ver>-linux-<arch>.tar.gz
#
# Usage: scripts/package-tarball.sh <version> <x86_64|aarch64>
set -euo pipefail
cd "$(dirname "$0")/.."

version="${1:?usage: package-tarball.sh <version> <arch>}"
arch="${2:?usage: package-tarball.sh <version> <arch>}"

case "$arch" in
    x86_64)  elf_arch='x86-64' ;;
    aarch64) elf_arch='aarch64' ;;
    *) echo "unknown arch: $arch" >&2; exit 1 ;;
esac

name="goshpad-v${version}-linux-${arch}"
stage="dist/stage/$name"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-$HOME/.local:/usr}"
cmake --build build -j"$(nproc)"

file build/bin/goshpad | grep -q "$elf_arch" ||
    { file build/bin/goshpad; echo "binary is not $arch ($elf_arch)" >&2; exit 1; }

rm -rf "$stage"
cmake --install build --prefix "$PWD/$stage"

# cmake --install keeps the prefix layout (bin/, share/). Add the installer.
cat > "$stage/install.sh" <<'EOF'
#!/bin/sh
# Install into $PREFIX (default ~/.local); use PREFIX=/usr for system-wide.
set -eu
prefix="${PREFIX:-$HOME/.local}"
self="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
install -Dm0755 "$self/bin/goshpad" "$prefix/bin/goshpad"
install -Dm0644 "$self/share/applications/com.goshapps.GoshPad.desktop" \
    "$prefix/share/applications/com.goshapps.GoshPad.desktop"
install -Dm0644 "$self/share/metainfo/com.goshapps.GoshPad.metainfo.xml" \
    "$prefix/share/metainfo/com.goshapps.GoshPad.metainfo.xml"
install -Dm0644 "$self/share/icons/hicolor/scalable/apps/com.goshapps.GoshPad.svg" \
    "$prefix/share/icons/hicolor/scalable/apps/com.goshapps.GoshPad.svg"
install -Dm0644 "$self/share/licenses/com.goshapps.GoshPad/LICENSE" \
    "$prefix/share/licenses/com.goshapps.GoshPad/LICENSE"
install -Dm0644 "$self/share/licenses/com.goshapps.GoshPad/COPYRIGHT" \
    "$prefix/share/licenses/com.goshapps.GoshPad/COPYRIGHT"
echo "installed goshpad under $prefix"
EOF
chmod +x "$stage/install.sh"
install -Dm0644 README.md "$stage/README.md"

mkdir -p dist
tar -C dist/stage -czf "dist/$name.tar.gz" "$name"
rm -rf dist/stage
echo "wrote dist/$name.tar.gz"
