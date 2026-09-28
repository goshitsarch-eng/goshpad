# Packaging

Version comes from the git tag (`vX.Y.Z` → `X.Y.Z`), verified against
`CMakeLists.txt` by `scripts/check-version.sh`.

## Tarball — `scripts/package-tarball.sh <version> <arch>`

Configures a Release build, compiles `goshpad`, checks the ELF architecture,
and stages:

```
goshpad-v4.0.0-linux-x86_64/
├── bin/goshpad
├── share/applications/com.goshapps.GoshPad.desktop
├── share/metainfo/com.goshapps.GoshPad.metainfo.xml
├── share/icons/hicolor/scalable/apps/com.goshapps.GoshPad.svg
├── share/licenses/com.goshapps.GoshPad/{LICENSE,COPYRIGHT}
├── README.md
└── install.sh          # copies into $PREFIX (default ~/.local)
```

Output: `dist/goshpad-v<ver>-linux-<arch>.tar.gz`.

## Flatpak — `scripts/package-flatpak.sh <version> <arch>`

1. `flatpak install --user` of `org.kde.Platform//6.9` and `org.kde.Sdk//6.9`
   from Flathub.
2. `flatpak-builder` of `com.goshapps.GoshPad.json` (cmake-ninja, install
   prefix `/app`).
3. `flatpak build-bundle` of `com.goshapps.GoshPad` branch `stable`.
4. The bundle must contain `app/com.goshapps.GoshPad/<arch>/stable`.

Finish-args, locked by `tests/packaging.sh`:

- `--share=ipc`
- `--socket=fallback-x11`
- `--socket=wayland`
- `--talk-name=org.freedesktop.portal.Desktop`

No host filesystem permission. File dialogs go through the portal. Settings
use the app's own XDG config file inside the sandbox.

## Native install

`just install` runs `cmake --install` with prefix `/usr`. It installs the
same binary, desktop file, metainfo, scalable icon, `LICENSE`, and
`COPYRIGHT`.
