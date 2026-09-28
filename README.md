# GoshPad

A clone of classic Microsoft Notepad (Windows XP era), written in C++ and
QML with Qt 6 and [Kirigami](https://develop.kde.org/frameworks/kirigami/).
One window, one plain-text file, and the File/Edit/Format/View/Help menus
you remember.

Current release: **4.0.0** — a ground-up rewrite on Qt 6 and Kirigami.
3.0.0 was libcosmic, 2.x was an earlier Qt 6 build, and 1.0.0 was GTK4.
Those trees are gone.

GoshPad is an independent implementation and is not affiliated with or
endorsed by Microsoft. Microsoft and Windows are trademarks of the Microsoft
group of companies.

## AI-assisted development

I use AI tools to speed up development, but I work architecture-first. I
define the architecture, review and refactor the implementation, and repeat
as the project evolves.

I treat AI as a junior developer: useful for implementation and exploration,
but not the final authority. I remain responsible for the architecture,
technical decisions, and code quality.

I'm including this notice so you can make an informed choice about whether
AI-assisted software is something you're comfortable using.

## Features

- Classic File / Edit / Format / View / Help menu bar
- Open, edit, and save UTF-8 plain-text files (New, Open, Save, Save As)
- Undo / Redo (caret and selection restored), Cut / Copy / Paste / Delete /
  Select All — on the Edit menu and the editor's right-click menu. Delete
  removes the selection or the next character
- Find bar (Ctrl+F) with a Match case checkbox and Find Next (F3, wraps
  around). Replace (Ctrl+H) adds Replace and Replace All
- Go To line (Ctrl+G) — unavailable while Word Wrap is on, like Windows
  Notepad
- Time/Date stamp (F5), Windows-style
- Word Wrap toggle and an optional status bar showing `Ln X, Col Y` and the
  wrap state
- Font picker (Format ▸ Font…): font family + size
- Color scheme: System (follows the desktop), Light, or Dark — from View ▸
  Color Scheme. The header button toggles Light/Dark
- Unsaved-changes prompt on New, Open, Exit, and the window close button;
  choosing Save continues what you were doing after Save As
- Single instance: a second `goshpad file.txt` opens the file in the running
  window; `goshpad` alone just focuses it
- Word wrap, status bar, font, and color scheme persist across restarts
- Line endings are preserved: CRLF and mixed-ending files save
  byte-identically when you have not edited those breaks
- Find, Go To, and the Ln/Col readout handle UTF-8 text correctly

## Install

### Flatpak

GoshPad 4.0.0 is a new app id, `com.goshapps.GoshPad`. It does not upgrade
the older `com.goshapps.Notepad` Flatpak. Build it from this tree:

```bash
flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install --user -y flathub org.kde.Platform//6.9 org.kde.Sdk//6.9
flatpak-builder --user --install-deps-from=flathub --install --force-clean \
    build-flatpak com.goshapps.GoshPad.json
flatpak run com.goshapps.GoshPad
```

Open and Save use the desktop portal, so the sandbox does not need a host
filesystem permission.

### Native install

Requires CMake, Qt 6 (Quick, Quick Controls, Quick Dialogs), and the KDE
Frameworks Kirigami, KConfig, KI18n, KCoreAddons, and KDBusAddons, plus
Extra CMake Modules.

```bash
just build-release
sudo just install   # installs to /usr: binary, desktop file, metainfo, icon, licenses
```

`sudo just uninstall` removes the same files.

## Usage

Launch GoshPad from your app menu, run `goshpad file.txt`, or use "Open With"
on a `.txt` file. Command-line paths must be absolute or relative to the
current directory; anything starting with `-` is ignored, and only the first
file opens — there are no flags (`--help` included).

Menus work like Windows Notepad, and there's a right-click menu in the editor
with Undo, Redo, Cut, Copy, Paste, Delete, and Select All. The shortcut list:

| Shortcut | Action |
|---|---|
| Ctrl+N / Ctrl+O / Ctrl+S / Ctrl+Shift+S | New / Open / Save / Save As |
| Ctrl+Q | Exit |
| Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z | Undo / Redo / Redo |
| Ctrl+X / Ctrl+C / Ctrl+V | Cut / Copy / Paste (focused text control) |
| Shift+Delete, Ctrl+Insert, Shift+Insert | Cut, Copy, Paste |
| Delete | Delete selection or next character |
| Ctrl+F / F3 / Ctrl+H / Ctrl+G | Find / Find Next / Replace / Go To |
| Ctrl+A | Select All |
| F5 | Insert time/date |
| Esc | Close the open dialog, About window, or Find bar |

Menu commands such as Ctrl+S, Ctrl+F, F3, and F5 work while a Find field is
focused. They do nothing while a modal dialog is open. Ctrl+A/X/C/V, Delete,
and the arrow keys act on the control that has focus. Enter in the Find
field runs Find Next; Enter in the Replace field runs Replace.

The Find bar does not take focus when it opens — click or Tab into the field
to type a search.

Settings live in `~/.config/com.goshapps.GoshPadrc` (KConfig). Nothing is
read from the old COSMIC config directory.

## Limitations

- One document per window, plain text only. No tabs, printing, recent-files
  list, or syntax highlighting — by design
- UTF-8 files only. A file in another encoding (Latin-1, CP1252, …) gets an
  error dialog rather than opening with mangled characters
- Undo history is capped at 100 steps
- The UI is English-only. Strings are wrapped in KI18n so translations can
  be added later
- Go To is unavailable while Word Wrap is on (deliberate Notepad parity)
- No `--help` and no man page

## Development

C++20, Qt 6, Kirigami, and CMake. `just` wraps configure, build, test, and
install. Flatpak uses `org.kde.Platform` / `org.kde.Sdk` **6.9**.

```bash
just build-debug
just test
just run
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for prerequisites,
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how the code is organized,
and [docs/RELEASING.md](docs/RELEASING.md) for version bumps and packages.

## Project layout

```
├── com.goshapps.GoshPad.json   # Flatpak manifest (KDE 6.9 runtime)
├── CMakeLists.txt              # Qt 6 + KF6 build and install rules
├── justfile                    # configure, build, test, install
├── scripts/                    # package-*.sh, verify-release.sh, check-version.sh
├── src/
│   ├── main.cpp                # application, about data, single instance
│   ├── controller.cpp          # menus' behavior, dialogs, find, settings
│   ├── document.cpp            # buffer, line endings, undo
│   ├── textops.cpp             # find / replace / go-to / caret math
│   ├── settings.cpp            # KConfig load and save
│   └── qml/                    # Kirigami window, menu bar, dialogs
├── tests/                      # Qt Test plus tests/packaging.sh
└── data/                       # desktop entry, AppStream metainfo, icon
```

## License

GPL-3.0-or-later — see [LICENSE](LICENSE) and [COPYRIGHT](COPYRIGHT).
