# Rewrite as GoshPad 4.0.0 (Qt 6 + Kirigami)

## Context

The tree is NotePad 3.0.0: a single-window classic Microsoft Notepad clone in Rust/libcosmic. 1.0.0 was GTK4 and 2.x was Qt 6; 3.0.0 moved to COSMIC. The Git remote is already `goshitsarch-eng/goshpad`, but the shipped identity is still `com.goshapps.Notepad` / binary `notepad` / display name NotePad.

Version 4 is a ground-up rewrite on C++20, QML, Qt 6, and Kirigami, **renamed GoshPad**, with full behavioral parity and fresh settings. Remove the Rust, libcosmic, Fluent, and COSMIC packaging path. Docs describe only the new app.

Locked decisions:

- C++20 + QML, CMake, and KF6: Kirigami, KConfig, KI18n, KCoreAddons, KDBusAddons.
- Classic File / Edit / Format / View / Help menu bar inside the window, as in Windows Notepad. Not a Kirigami global drawer.
- No import of `~/.config/cosmic/com.goshapps.Notepad/`.
- Version **4.0.0**. Display name **GoshPad**. Binary **`goshpad`**. App id **`com.goshapps.GoshPad`**. This is a new Flatpak id, not an upgrade of `com.goshapps.Notepad`.
- Flatpak on `org.kde.Platform` / `org.kde.Sdk` **6.9**.
- English UI only. Strings go through KI18n; no other locales ship. Fluent is deleted.
- Full parity with 3.0.0 behavior, including its limits.

Still absent, on purpose: tabs, printing, recent files, syntax highlighting, drag-and-drop, non-UTF-8 encodings, notifications, `--help`, and a man page.

## Approach

One desktop process. `Kirigami.ApplicationWindow` holds a Qt Quick Controls `MenuBar` (File, Edit, Format, View, Help) and a header-end control that toggles Light/Dark. The document is a Qt Quick text area whose on-disk bytes are owned by C++, not by the control's normalized `\n` buffer.

**Identity.** `KAboutData` uses `com.goshapps.GoshPad`, display name GoshPad, version 4.0.0, GPL-3.0-or-later, and `https://github.com/goshitsarch-eng/goshpad`. Window title is `•  {name} — GoshPad` when dirty, otherwise `{name} — GoshPad`, with `Untitled` when unnamed. Desktop categories become `Utility;TextEditor;Qt;KDE;`. Keywords keep `notepad` and add `goshpad`. Drop `X-COSMIC` and the `com.system76.CosmicApplication` provide. Rename the SVG to `com.goshapps.GoshPad.svg` (same artwork). `COPYRIGHT` names GoshPad.

**Document model** (`Document`, C++):

- Port `src/commands.rs` into `textops` and keep its tests as the spec: find next with wrap, case-insensitive by default (per-code-point lowercase, not full Unicode casefold), replace-current-then-find, replace-all, go-to-line, and UTF-8 line/column math (`line_col_at`, `caret_line_col`, `offset_at_line_col`).
- Load with strict UTF-8. Invalid bytes show "Could not open file" naming the path and leave the open document untouched.
- Preserve line endings. Load and save round-trip CRLF and mixed endings byte-identically. The QML control may use `\n` internally; the C++ model converts on the way in and out using the per-line endings captured at load (new files and pasted text use `\n`).
- Dirty when current text differs from the last saved text.
- Undo/redo are snapshot stacks of `(text, cursor position, selection)`, capped at **100**. Any new edit clears redo. Cut, copy, paste, delete, and the context menu feed that stack. Delete removes the selection or the next character.
- Default size 820×600, minimum 360×180.

**Editing UI.**

- Menus and shortcuts match 3.0.0: Ctrl+N/O/S/Shift+S/Q, Ctrl+Z/Y/Shift+Z, Ctrl+X/C/V, Shift+Delete / Ctrl+Insert / Shift+Insert, Delete, Ctrl+F, F3, Ctrl+H, Ctrl+G, Ctrl+A, F5. Context menu: Undo, Redo, Cut, Copy, Paste, Delete, Select All.
- Menu shortcuts are application-wide, including while a Find field is focused, and do nothing while a modal dialog is open. Ctrl+A/X/C/V, Delete, and arrows act on the focused control.
- Find/Replace is an in-window bar: close button, find field, replace field (replace mode), Match case, Find Next, Replace, Replace All. Enter in Find runs Find Next; Enter in Replace runs Replace. A single-line selection prefills the find field. A miss opens "Cannot find …".
- Go To is a dialog, prefilled with the caret line, with inline errors for non-numeric and out-of-range input. The menu item is disabled and Ctrl+G is inert while Word Wrap is on.
- F5 inserts local time `h:MM AM/PM M/D/YYYY`.
- Status bar shows `Ln X, Col Y` and whether wrap is on. View ▸ Status Bar toggles it (default on).
- Format ▸ Word Wrap (default off). Format ▸ Font… is family + size, the current curated list plus a free-text family, sizes 8–36 pt, applied on OK, no live preview.
- Esc closes the topmost dialog, then About, then the Find bar.

**Color scheme.** View ▸ Color Scheme ▸ System / Light / Dark, persisted. The header button toggles between Light and Dark and its label names the mode it will switch to. System clears the override and follows `QStyleHints::colorSchemeChanged`. Light and Dark call `QGuiApplication::styleHints()->setColorScheme()` so Kirigami and Qt Quick Controls share one scheme. No hardcoded widget colors.

**Files and close.** `QFileDialog` (portal platform theme under Flatpak) filters Text files `*.txt` and All files `*`. Open starts in the current file's directory. Save As prefills the name or `Untitled.txt`. Unsaved-changes prompt on New, Open, Exit, window close, and a second-instance open. Save continues the original action after a successful write, including when Save As was required. Discard proceeds. Cancel aborts. A failed save names the OS error; dismissing that error re-raises the prompt when a continuation was pending. Only the first non-option CLI path is opened; arguments starting with `-` are ignored.

**Settings.** `KSharedConfig` for `com.goshapps.GoshPad`. Defaults: `color_scheme=system`, `word_wrap=false`, `show_status_bar=true`, `font_family=monospace`, `font_size=14`. Malformed values fall back to those defaults.

**Single instance.** `KDBusService` in `Unique` mode. Activation focuses the window. A forwarded path runs the unsaved-changes guard, then loads. No path only focuses. If the bus cannot be claimed, a second window may start (same accepted failure as today).

**i18n.** `i18n()` / `i18nc()` around user-visible strings. Ship English only; delete `i18n/` and `i18n.toml`. Do not add a translation catalog.

**Flatpak.** New manifest `com.goshapps.GoshPad.json`: `org.kde.Platform//6.9` and `org.kde.Sdk//6.9`, command `goshpad`, finish-args limited to ipc, wayland, fallback-x11, and portal talk. No host filesystem and no cosmic-config mount. Drop Rust SDK extensions, `cargo vendor`, and `scripts/vendor.sh`.

**Tests and tooling.** Qt Test for `textops` and `Document` with no display: the current `commands.rs` cases, invalid UTF-8 rejection, CRLF and mixed-ending byte identity, undo cap, and caret restore. A small packaging check asserts version 4.0.0 across CMake, metainfo, and About; manifest command `goshpad`; app id `com.goshapps.GoshPad`; license install lines; no host filesystem permission. `justfile` becomes CMake configure/build, `run`, `test`, `install`, and `uninstall`.

## Files to modify

Remove:

- `Cargo.toml`, `Cargo.lock`, `rust-toolchain.toml`, `src/**/*.rs`, `tests/packaging.rs`
- `i18n/`, `i18n.toml`, `scripts/vendor.sh`
- `com.goshapps.Notepad.json`
- `data/com.goshapps.Notepad.desktop`, `data/com.goshapps.Notepad.metainfo.xml`
- `data/icons/hicolor/scalable/apps/com.goshapps.Notepad.svg` (after the renamed copy exists)

Add:

- `CMakeLists.txt` — Qt 6 Quick/QuickControls2, KF6 Kirigami, Config, I18n, CoreAddons, DBusAddons; install binary, desktop, metainfo, icon, LICENSE, COPYRIGHT
- `src/main.cpp` — application, `KAboutData`, `KDBusService`, QML engine, first-path CLI
- `src/document.h`, `src/document.cpp` — buffer, endings, dirty flag, undo/redo, load/save
- `src/textops.h`, `src/textops.cpp` — port of `src/commands.rs`
- `src/settings.h`, `src/settings.cpp` — KConfig
- `src/qml/Main.qml` — window, menu bar, editor, find bar, status bar, scheme button
- `src/qml/` dialogs — save-changes, go-to, font, error; Kirigami about page
- `tests/tst_textops.cpp`, `tests/tst_document.cpp`, `tests/packaging.sh`
- `com.goshapps.GoshPad.json`
- `data/com.goshapps.GoshPad.desktop`, `data/com.goshapps.GoshPad.metainfo.xml`
- `data/icons/hicolor/scalable/apps/com.goshapps.GoshPad.svg`

Update:

- `README.md`, `CONTRIBUTING.md`, `COPYRIGHT`
- `docs/ARCHITECTURE.md`, `docs/RELEASING.md`
- `docs/documentation/APP-INVENTORY.md`, `AUDIT.md`, `PLAN.md` — describe GoshPad 4.0.0; mark the libcosmic write-up as superseded
- `docs/release/CI-ARCHITECTURE.md`, `PACKAGING.md`, `RELEASES.md`, `REPORT.md`
- `justfile`, `scripts/check-version.sh`, `scripts/package-flatpak.sh`, `scripts/package-tarball.sh`, `scripts/verify-release.sh`
- Release artifact names become `goshpad-v4.0.0-linux-<arch>.flatpak` and `.tar.gz`, with `bin/goshpad` inside the tarball

## Reuse

Port behavior, then delete the Rust sources:

- `src/commands.rs` — search, replace, go-to, and line/column functions, plus their unit tests.
- `src/app.rs` — menu set, shortcuts, `AfterSave` / `PendingDialog` continuation, title rules, font list, find-bar keys, escape order.
- `src/config.rs` — the five settings and their defaults. Storage changes from cosmic-config RON to KConfig.
- `src/single_instance.rs` — user-visible contract only (focus, or focus and open the first path). The socket protocol is not kept.
- `src/app_file_tests.rs` — UTF-8 rejection, load errors that name the file, CRLF and mixed-ending round-trips.
- `tests/packaging.rs` — invariant list, retargeted at CMake and `com.goshapps.GoshPad.json`.
- The existing scalable icon artwork, copied to the new app-id filename.

## Steps

- [x] Add the CMake + KF6 skeleton that configures, builds, and installs `goshpad` with the new desktop file, metainfo, and icon.
- [x] Port `textops` and `Document` (strict UTF-8, byte-identical endings, 100-deep undo with caret restore) and land Qt Test coverage before the QML UI.
- [x] Build `Main.qml`: Notepad menu bar, editor, status bar, find/replace bar, Go To, font, error, and save-changes dialogs, plus About.
- [x] Wire app-wide shortcuts, the editor context menu, escape priority, and the unsaved-changes continuation state machine.
- [x] Persist the five settings with KConfig. Implement System / Light / Dark through `QStyleHints`, including the header toggle.
- [x] Claim a unique `KDBusService` and route activation through the unsaved-changes guard.
- [x] Add `com.goshapps.GoshPad.json` on the KDE 6.9 runtime. Rewrite `justfile` and the release scripts. Delete Cargo, Rust sources, Fluent, vendor script, and the old app-id files.
- [x] Rewrite README, CONTRIBUTING, COPYRIGHT, architecture, inventory, and release docs so they name GoshPad, Qt 6, Kirigami, and the KDE runtime only.

## Verification

- `just test`: Qt Test covers the old `commands.rs` cases and the file tests (invalid UTF-8 leaves the buffer untouched; CRLF and mixed endings survive load→save as the same bytes; undo cap and caret restore). `tests/packaging.sh` checks app id, binary name, version 4.0.0, license install lines, and Flatpak finish-args.
- `just build-release && just run`: walk every menu, System/Light/Dark and the header toggle, font and status-bar persistence across restart, Find/Replace/Go To, F5, and unsaved-changes on New, Open, close, and a second launch with a path.
- A second `goshpad file.txt` focuses the running instance and does not create another window. `goshpad` with no path only focuses it.
- Flatpak build from `org.kde.Platform//6.9` launches with no host filesystem permission; Open and Save still work through the portal.
- Search of the tree finds no `libcosmic`, `cargo`, `i18n-embed`, `cosmic-config`, Fluent, `com.goshapps.Notepad`, or binary `notepad`, except release notes that explicitly say those belonged to 3.0.0 and earlier.
