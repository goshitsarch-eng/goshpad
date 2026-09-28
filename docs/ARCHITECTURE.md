# Architecture

GoshPad is a single-window Qt 6 application. C++ owns the document, the
settings, and the File/Edit command state. QML (`Kirigami.ApplicationWindow`)
draws a classic menu bar, the editor, the find bar, and the dialogs.

```
src/main.cpp         QGuiApplication, KAboutData, KDBusService, QML engine
src/controller.h/cpp menus, shortcuts' behavior, dialogs, find/replace, scheme
src/document.h/cpp   canonical text, display text, undo/redo, load/save
src/textops.h/cpp    find / replace / go-to / line-column math
src/settings.h/cpp   KConfig file com.goshapps.GoshPadrc
src/qml/Main.qml     window, menu bar, editor, find bar, status bar
src/qml/*Dialog.qml  save-changes, go-to, font, error, about
```

## How the pieces fit

- **Editing state.** `Document` stores the canonical `QString`, including
  original `\r\n` and lone `\r` breaks. The text area only sees `\n`.
  Edits are merged back by line: unchanged breaks stay, and a newly inserted
  line break is `\n`. Dirtiness is canonical text versus the last saved
  canonical text.
- **Undo.** Snapshots of `(canonical text, caret, selection)`, capped at 100.
  A new edit clears the redo stack. The caret restored is the one committed
  before the edit.
- **Unsaved changes.** `Controller::guard` opens the save-changes dialog and
  remembers New, Open, Open-path, or Close. Save writes the file (via Save
  As when the document has no path) and then continues that action. Discard
  marks the buffer clean and continues. Cancel drops the pending action. A
  failed save shows the OS error and, when the save was part of that
  continuation, puts the save-changes dialog back when the error is dismissed.
- **Close path.** The window's `onClosing` handler rejects the close until
  `Controller` emits `quitRequested`.
- **Shortcuts.** App-wide menu shortcuts are Qt Quick `Action` shortcuts, and
  they are disabled while a dialog or file chooser is open. Ctrl+X/C/V/A and
  the insert/delete alternates are window shortcuts enabled only while the
  editor has focus, so a Find field keeps those keys. Esc closes a dialog
  (or About), then the Find bar.
- **Config.** `KSharedConfig` group `Editor` in
  `~/.config/com.goshapps.GoshPadrc`. Keys: `color_scheme`, `word_wrap`,
  `show_status_bar`, `font_family`, `font_size`. There is no import from
  `~/.config/cosmic/`.
- **Theme.** System calls `QStyleHints::unsetColorScheme()`. Light and Dark
  call `setColorScheme()`. Kirigami and Qt Quick Controls follow that hint.
  The header button toggles between Light and Dark and labels the mode it
  will switch to.
- **Single instance.** `KDBusService` in unique mode. The service name comes
  from the organization domain `goshapps.com` and the component name
  `goshpad` (`com.goshapps.goshpad`). A second launch emits
  `activateRequested` on the running process and exits. The handler focuses
  the window and, if a path was passed, runs the unsaved-changes guard. If
  the bus cannot be claimed, `NoExitOnFailure` lets another window start.
- **File IO.** Load uses `QStringDecoder` in UTF-8 mode. Invalid bytes show
  "Could not open file" and leave the document untouched. Save writes the
  canonical string as UTF-8. File dialogs are Qt Quick `FileDialog`s, which
  use the XDG portal inside Flatpak.
- **i18n.** `KLocalization::setupLocalizedContext` exposes `i18n()` to QML.
  The domain is `goshpad`. Only English source strings ship.

## Testing

`textops` and `Document` tests do not need a display. `tst_controller` uses
an offscreen Qt Quick platform and `QStandardPaths::setTestModeEnabled` so
settings stay out of the home directory. `tests/packaging.sh` locks the app
id, version, runtime, and finish-args.

## Packaging

`com.goshapps.GoshPad.json` builds with `cmake-ninja` on
`org.kde.Platform//6.9` and installs into `/app`. Finish-args are ipc,
Wayland, fallback X11, and the desktop portal. There is no host filesystem
permission and no COSMIC config mount. `justfile` configures CMake with
prefix `/usr` and installs the binary, desktop file, metainfo, icon, and
license files.
