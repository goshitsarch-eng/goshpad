# Documentation audit

The 3.0.0 audit checked a libcosmic tree that is no longer in this
repository. This note replaces it.

GoshPad 4.0.0 documentation was rewritten with the Qt 6 sources:

| Document | What it claims now |
|---|---|
| `README.md` | Features, shortcuts, KDE 6.9 Flatpak build, native `just install`, KConfig path, English-only UI |
| `CONTRIBUTING.md` | CMake / Qt 6 / KF6 prerequisites, `just test`, no Rust or Fluent |
| `docs/ARCHITECTURE.md` | Controller, document, textops, Kirigami shell, KDBusService |
| `docs/RELEASING.md` | Version bump locations and the package scripts |
| `docs/release/*` | Tarball layout, Flatpak finish-args, absence of a checked-in CI workflow |

Claims that must stay true:

- App id `com.goshapps.GoshPad`, binary `goshpad`, version 4.0.0.
- Flatpak runtime `org.kde.Platform//6.9`, no host filesystem finish-arg.
- Settings file `com.goshapps.GoshPadrc`, not the COSMIC config directory.
- UTF-8 only, 100-step undo, Go To disabled while wrapping, first CLI file only.

`tests/packaging.sh` fails if the manifest, desktop file, metainfo, CMake
version, or README release line drift from that identity.
