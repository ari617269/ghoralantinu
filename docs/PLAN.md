# Plan

## Phase 0: Scaffold
- [x] CMakeLists with targets: `ttcore` (static lib), `TimeTracker` (app), tests
- [x] CMakePresets (debug/release), .clang-format
- [x] Empty MainWindow shows; one passing dummy test
**Done when:** configure/build/test all succeed.

## Phase 1: Core time utilities (TDD)
- [x] `IClock`, `SystemClock`, `FakeClock`
- [x] `TimeUtils::localDayStartUtc(epoch)`, `localDayEndUtc`, `weekStartUtc`
- [x] `TimeUtils::splitByLocalDay(start, end)` → per-day segments
- [x] `TimeUtils::formatDuration(secs)` → "H:MM:SS"
- [x] Tests incl. DST transitions (TZ=America/New_York, Europe/Berlin) and midnight crossing
**Done when:** tst_timeutils passes under multiple TZ values.

## Phase 2: Persistence
- [x] Database open/migrate (schema_version), in-memory mode for tests
- [x] TaskRepository: create, list, rename, archive, findByName
- [x] SessionRepository: start, stop, findRunning, listInRange, update, delete, overlap check
- [x] Totals queries: per task today/total, day totals in range
**Done when:** tst_repositories passes using `:memory:` DB.

## Phase 3: TrackerService
- [x] start(taskId) auto-stops active; stop(); activeSession(); elapsed()
- [x] Signals: sessionStarted, sessionStopped, dataChanged
- [x] Recovery on startup (running session detection, >12h flag)
- [x] Edit/delete/manual-add with validation (no overlaps, end > start)
**Done when:** tst_trackerservice passes with FakeClock.

## Phase 4: Screen 1 (Tracker)
- [x] MainWindow with QStackedWidget + nav
- [x] TrackerPage: header, new-task field, task table model (QAbstractTableModel), toggles, footer
- [x] 1s QTimer refreshes visible elapsed only
- [x] Shortcuts: Space toggle selected, Ctrl/Cmd+N new task, Ctrl/Cmd+. stop
- [x] UI smoke test (offscreen): create task, toggle, assert service state
**Done when:** tests pass + MANUAL_TEST.md items added.

## Phase 5: Screen 2 (History)
- [x] HistoryPage: grouped-by-day view (QTreeView + model), range filter
- [x] SessionEditDialog (QDateTimeEdit in local time → converted to UTC)
- [x] Delete with undo (QUndoStack or simple last-action undo)
- [x] Manual entry
- [x] UI smoke test for edit validation
**Done when:** tests pass + manual items added.

## Phase 6: Tray (cross-platform)
- [ ] TrayController: icon swap, menu (status, stop, recent tasks, open screens, quit)
- [ ] Close window = hide; Quit only from menu / Cmd+Q
- [ ] Graceful fallback if !isSystemTrayAvailable()
**Done when:** build passes on both platforms; manual checklist written.

## Phase 7: macOS menu bar text
- [ ] `StatusItem` interface; `StatusItem_mac.mm` (NSStatusItem button.title); Linux stub
- [ ] CMake: enable OBJCXX + link `-framework AppKit` on APPLE only
- [ ] Decide: replace QSystemTrayIcon on macOS or coexist (document choice)
  - Decision: coexist. QSystemTrayIcon keeps the icon and menu. A separate NSStatusItem shows only the elapsed time while a task runs; clicking it opens the Tracker.
**Done when:** builds on macOS + Linux.

## Phase 8: Robustness & polish
- [x] Startup recovery dialog, long-running reminder
- [x] Settings (reminder hours; launch at login not implemented, optional per spec)
- [x] CSV export from History

## Phase 9: Packaging
- [ ] macOS: Info.plist (LSUIElement optional), macdeployqt, icns, signing notes
- [ ] Linux: .desktop file, AppImage via linuxdeploy
