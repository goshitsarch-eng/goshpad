# 4.0.0 packaging report

GoshPad 4.0.0 replaces the libcosmic application. The Rust crate, Fluent
catalogs, cosmic-config files, and `com.goshapps.Notepad` packaging id are
not part of this tree.

| Item | Value |
|---|---|
| Display name | GoshPad |
| Binary | `goshpad` |
| App id | `com.goshapps.GoshPad` |
| Version | 4.0.0 |
| License | GPL-3.0-or-later |
| UI | Qt 6, Kirigami, classic menu bar |
| Settings | `~/.config/com.goshapps.GoshPadrc` |
| Single instance | KDBusAddons `KDBusService` |
| Flatpak runtime | `org.kde.Platform//6.9` |
| Flatpak SDK | `org.kde.Sdk//6.9` |
| Host filesystem | not requested |

Checked by `tests/packaging.sh` and `scripts/check-version.sh`. The feature
behavior those packages wrap is covered by the Qt Test targets `textops`,
`document`, and `controller`.
