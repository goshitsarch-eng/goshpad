# Documentation plan

Superseded for the product itself by the Qt 6 rewrite. The libcosmic
documentation plan described NotePad 3.0.0 and is not a backlog.

Current documents and why they exist:

| File | Role |
|---|---|
| `README.md` | Users: features, install, shortcuts, limits |
| `CONTRIBUTING.md` | Build prerequisites and test command |
| `docs/ARCHITECTURE.md` | Map of the C++/QML split |
| `docs/RELEASING.md` | How to cut a version |
| `docs/release/PACKAGING.md` | Tarball and Flatpak layout |
| `docs/release/CI-ARCHITECTURE.md` | The package scripts; no checked-in CI workflow |
| `docs/release/RELEASES.md` | Version history across toolkits |
| `docs/release/REPORT.md` | 4.0.0 identity card |
| `docs/documentation/APP-INVENTORY.md` | Feature list tied to this tree |
| `docs/documentation/AUDIT.md` | Which docs are allowed to make which claims |

Do not document libcosmic, cosmic-config, Fluent, or `com.goshapps.Notepad`
as the current application. Mention them only as history that 4.0.0 removed.
