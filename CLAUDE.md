# Time Tracker (Qt 6 / C++17)

Cross-platform (macOS arm64 + Linux) menu bar/tray time tracker.
Read `docs/SPEC.md` for requirements and `docs/PLAN.md` for current progress.

## Commands
- Configure: `cmake --preset debug`
- Build:     `cmake --build --preset debug`
- Test:      `QT_QPA_PLATFORM=offscreen ctest --preset debug --output-on-failure`
- Run:       `./build/debug/TimeTracker` (macOS: `open build/debug/TimeTracker.app`)

## Architecture rules
- `src/core/` must NOT include QtWidgets/QtGui. Only QtCore + QtSql.
- All time logic goes through `IClock`. Never call `QDateTime::currentSecsSinceEpoch()`
  directly outside `SystemClock`.
- UI classes contain no business logic; they call `TrackerService` and react to its signals.
- `TrackerService` is the single source of truth for the active session.

## Time handling (critical)
- Store timestamps as UTC epoch seconds (`qint64`) in SQLite. Never store local strings.
- Convert to local time ONLY for display, via `QDateTime::fromSecsSinceEpoch(x).toLocalTime()`.
- Elapsed time = `clock.now() - start_utc`. Never accumulate ticks.
- The QTimer exists only to refresh the display (1s).
- "Today"/day grouping = local midnight → local midnight. Sessions crossing midnight
  are split for per-day totals.
- Persist start/stop to the DB immediately. A session with `end_utc IS NULL` = running.

## Behavior rules
- Only one session may run at a time. Starting a task stops the active one.
- Closing the main window hides it; the app keeps running (`setQuitOnLastWindowClosed(false)`).
- Use `QLocale` for all user-facing date/time formatting.

## Code style
- C++17, Qt 6.5+, CMake. Run clang-format (hook does this automatically).
- Use `std::optional` for nullable values; `Q_OBJECT` classes in their own files.
- Use signals/slots with the new function-pointer syntax only.

## Workflow rules
- Work one phase at a time from `docs/PLAN.md`. Do not start the next phase unprompted.
- Write/update tests BEFORE implementing core logic.
- A phase is done only when build + all tests pass. Then tick its boxes in PLAN.md.
- If something requires human visual verification, add it to `docs/MANUAL_TEST.md`.
- Never weaken or delete a test to make it pass without explaining why.
