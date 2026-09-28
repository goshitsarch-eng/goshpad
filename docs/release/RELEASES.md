# Releases

| Version | Date | Toolkit |
|---|---|---|
| 4.0.0 | 2026-09-14 | Qt 6 + Kirigami. New app id `com.goshapps.GoshPad`, binary `goshpad`. |
| 3.0.0 | 2026-09-06 | libcosmic. Removed in 4.0.0. |
| 2.0.x | 2026-08 | Earlier Qt 6 / Kirigami series, app id `com.goshapps.Notepad`. |
| 1.0.0 | 2026-08-19 | GTK4. Removed. |

4.0.0 does not read `~/.config/cosmic/com.goshapps.Notepad/`. Settings start
fresh in `~/.config/com.goshapps.GoshPadrc`.

Flatpak users of `com.goshapps.Notepad` need a new install of
`com.goshapps.GoshPad`. The ids are not interchangeable.

Artifact names from 4.0.0 onward:

- `goshpad-v<version>-linux-<arch>.tar.gz`
- `goshpad-v<version>-linux-<arch>.flatpak`

Older NotePad artifact names are not produced anymore.
