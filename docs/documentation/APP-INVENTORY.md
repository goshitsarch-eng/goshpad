# GoshPad application inventory

This describes GoshPad 4.0.0 as built from this tree. The 3.0.0 libcosmic
inventory was removed with that code. It is not a second source of truth.

Status: **works** means implemented in `src/`. Limits called out below are
intentional Notepad parity, not unfinished ports.

## Identity

| Item | Value |
|---|---|
| Name | GoshPad |
| App id | `com.goshapps.GoshPad` |
| Binary | `goshpad` |
| Version | 4.0.0 |
| License | GPL-3.0-or-later |
| Toolkit | Qt 6 + Kirigami + KConfig, KI18n, KCoreAddons, KDBusAddons |

## Window and menus

- Default size 820×600, minimum 360×180.
- Menu bar: File, Edit, Format, View, Help, inside the window.
- Header action toggles Light/Dark and names the mode it will switch to.
- Title: `•  {name} — GoshPad` when dirty, otherwise `{name} — GoshPad`.
  Unnamed documents use `Untitled`.
- Help ▸ About GoshPad opens a Kirigami about page (name, version, GPL-3.0-or-later, repository, bug tracker).

File: New, Open, Save, Save As, Exit.

Edit: Undo, Redo, Cut, Copy, Paste, Delete, Find, Find Next, Replace, Go To
(disabled while Word Wrap is on), Select All, Time/Date.

Format: Word Wrap (persisted), Font….

View: Status Bar (persisted), Color Scheme ▸ System / Light / Dark.

The editor context menu repeats Undo, Redo, Cut, Copy, Paste, Delete, and
Select All.

## Document behavior

| Feature | Status |
|---|---|
| Undo / redo of text plus caret and selection, 100 entries | works |
| Dirty flag from canonical text versus last save | works |
| Strict UTF-8 load; invalid bytes leave the buffer untouched and name the file | works |
| CRLF and mixed endings round-trip when those breaks are not edited | works |
| New line breaks inserted by editing use `\n` | works |
| One document; only the first non-option CLI path opens | works |
| No tabs, printing, recent files, syntax highlighting, or drag-and-drop | absent on purpose |

## Find, replace, go to

Find is case-insensitive unless Match case is checked, wraps, and reports
"Cannot find …". A single-line selection prefills the query. Replace All is
one undo step. Go To is prefilled with the caret line and rejects non-numeric
and out-of-range input. It is unavailable while Word Wrap is on.

## Settings

`~/.config/com.goshapps.GoshPadrc`, group `Editor`:

| Key | Default |
|---|---|
| `color_scheme` | `system` |
| `word_wrap` | false |
| `show_status_bar` | true |
| `font_family` | `monospace` |
| `font_size` | 14 |

System theme follows `QStyleHints`. Light and Dark pin
`Qt::ColorScheme`. No cosmic-config migration.
