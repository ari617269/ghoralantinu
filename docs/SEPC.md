# Spec

## Screens (single MainWindow with QStackedWidget + nav buttons "Tracker" | "History")

### Screen 1: Tracker
- Header: active task name + live elapsed (HH:MM:SS, monospace) + Stop button.
- "New task…" line edit; Enter = create + start.
- Task table: Name | Today | Total | ▶/■ toggle. Active row highlighted.
- Footer: Today total, This week total (week starts per QLocale).

### Screen 2: History
- Sessions grouped by local day (newest first), day header shows day total.
- Row: start → end (local), duration, task name.
- Date range filter (default: last 7 days).
- Actions: Edit (start/end/task), Delete (with undo), Add manual entry.
- Validation: end > start, no overlap with other sessions, cannot edit the running session's end.

## Tray / menu bar
- Icon swaps idle/running (template icons on macOS).
- Menu: status line ("Running: Design 0:42:10" / "Not running"), Stop,
  up to 5 recent tasks to start, separator, Open Tracker, Open History, Quit.
- macOS: elapsed time shown as text next to the icon (NSStatusItem shim).
- Linux: menu is the primary interaction; don't rely on left-click/tooltips.

## Robustness
- On launch, resume any running session. If it has run > 12h, ask: keep / stop at last-seen / discard.
- Long-running reminder after configurable hours (default 8).
- Survives sleep/wake and crashes (timestamp-based).

## Data (SQLite at QStandardPaths::AppDataLocation/timetracker.db)
tasks(id, name UNIQUE, created_utc, archived)
sessions(id, task_id, start_utc, end_utc NULL, tz_name)
schema_version table for migrations.
