# Contributing

## Setup

You need CMake, a C++20 compiler, Qt 6.5 or newer (Qt Quick, Qt Quick
Controls, and Qt Quick Dialogs), and these KDE Frameworks 6 modules:
Kirigami, KConfig, KI18n, KCoreAddons, and KDBusAddons. Extra CMake Modules
must be on `CMAKE_PREFIX_PATH` (usually the same prefix as the frameworks).

On Fedora:

```bash
sudo dnf install cmake ninja-build gcc-c++ extra-cmake-modules \
    qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtquickcontrols2-devel \
    kf6-kirigami-devel kf6-kconfig-devel kf6-ki18n-devel \
    kf6-kcoreaddons-devel kf6-kdbusaddons-devel
```

On Arch Linux the same pieces are the `cmake`, `extra-cmake-modules`,
`qt6-base`, `qt6-declarative`, `kirigami`, `kconfig`, `ki18n`,
`kcoreaddons`, and `kdbusaddons` packages.

`just` is optional. The recipes wrap CMake.

## Build, run, test

```bash
just build-debug
just test          # CTest (textops, document, controller) and tests/packaging.sh
just run           # release build, then goshpad
sudo just install  # default prefix /usr
```

`tests/packaging.sh` checks that CMake, the metainfo, the README release
line, the desktop file, and `com.goshapps.GoshPad.json` agree on version
4.0.0, the app id, the binary name, the KDE 6.9 runtime, and the Flatpak
finish-args. Update that script in the same change if you intentionally
change any of those.

There is no Rust toolchain, Fluent catalog, or vendored crate tree.

## Flatpak development build

```bash
flatpak install --user -y flathub org.kde.Platform//6.9 org.kde.Sdk//6.9
flatpak-builder --user --install-deps-from=flathub --install --force-clean \
    build-flatpak com.goshapps.GoshPad.json
flatpak run com.goshapps.GoshPad
```

## Style notes

- Search, replace, go-to, and caret math live in `src/textops.cpp` and are
  tested without a window.
- The document buffer in `src/document.cpp` keeps the on-disk line endings.
  The QML text area sees `\n` only. Do not normalize a file to `\n` on save
  if the user did not edit that break.
- User-visible strings go through KI18n: `i18n()` in C++ and QML. English
  is the only shipped locale. Do not add a translation catalog unless you
  are also adding a real translation.
- Controller tests must call `QStandardPaths::setTestModeEnabled(true)` so
  they do not write `~/.config/com.goshapps.GoshPadrc`.
- No open PR process is formalized — this is a personal project. Filing an
  issue first is a good idea for anything beyond a small fix.

## Translations

Strings are marked with `i18n()` / `i18nc()`. No `po/` files ship. A future
translation would be a KI18n gettext catalog for the `goshpad` domain.
