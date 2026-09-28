# Release scripts

This repository does not ship a GitHub Actions workflow. Releases are cut
with the scripts below on a machine that has the Qt 6 / KDE Frameworks 6
development packages, or the KDE 6.9 Flatpak SDK.

```
scripts/check-version.sh vX.Y.Z
        tag ↔ CMakeLists.txt ↔ metainfo ↔ README
   │
   ├── scripts/package-tarball.sh X.Y.Z <arch>
   │        cmake Release build, ELF arch check, dist tarball
   │
   └── scripts/package-flatpak.sh X.Y.Z <arch>
            org.kde.Platform//6.9 + Sdk, flatpak-builder, build-bundle
   │
scripts/verify-release.sh <dir> X.Y.Z
        both architectures' tarballs and Flatpaks, plus SHA256SUMS
```

Build both `x86_64` and `aarch64` before publishing. `verify-release.sh`
refuses a directory that is missing either architecture or has extra files.

`just test` is the local gate: Qt Test for text operations, the document
buffer, and the controller, then `tests/packaging.sh`.
