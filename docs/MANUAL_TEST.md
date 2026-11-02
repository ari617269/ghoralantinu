# Manual Test Checklist

Items that need a human to look at or use the running app. Automated tests cover the logic; these cover how it looks and feels.

## Phase 4: Tracker screen

- [ ] Launch the app. The Tracker tab shows: header (task name, elapsed time, Stop button), "New task…" field, task table, footer (Today and This week totals).
- [ ] Elapsed time in the header uses a monospace font and stays the same width as digits change.
- [ ] Type a new name in "New task…" and press Return. The task starts, the header shows its name, and the field clears.
- [ ] The running row has a blue tint and shows ■ in the toggle column. Idle rows show ▶. The tint is visible in both light and dark appearance.
- [ ] Click ▶ on a row to start it, then click ■ to stop it.
- [ ] Select a row, click the table, and press Space: the selected task toggles. Typing a space in "New task…" still types a space.
- [ ] Cmd+N (Ctrl+N on Linux) moves focus to the "New task…" field.
- [ ] Cmd+. (Ctrl+. on Linux) stops the running task.
- [ ] While a task runs, the header elapsed time and the running row's Today/Total update every second.
- [ ] Long task names are truncated or wrapped cleanly and do not push the Today/Total columns off screen.
- [ ] Resize the window narrower and wider. The table, header, and footer stay readable.
- [ ] Close the window with the red button. The app process keeps running after the window closes. There is no way to reopen the window yet; Phase 6 adds the tray menu that does this.

## Phase 5: History screen

- [ ] Open History. Sessions are grouped under day headers, newest day first, and each day header shows that day's total. Rows show start – end in local time, duration, and task name.
- [ ] The From/To date filters default to the last 7 days. Changing them updates the list.
- [ ] A session that crosses local midnight appears under its start day, and both days' totals include its portion.
- [ ] Select a closed session and click Edit…. Change the task and times and press OK. The row updates.
- [ ] Try an end time before the start time, and a time that overlaps another session. An error message appears and the dialog stays open with your values.
- [ ] A running session shows "– running". Edit… is available for it, but its end time is not editable, and Delete is disabled.
- [ ] Delete a session, then press Undo delete (or Cmd+Z). The session returns with the same times and task.
- [ ] Add entry… creates a manual entry for the last hour by default, using the time-picker dialog.
- [ ] Date and time fields use the system locale format (12h or 24h to match your settings).

## Phase 6: Tray

- [ ] macOS: a tray icon appears in the menu bar. It is a hollow circle when idle and a filled circle while a task runs. It adapts to light and dark menu bars.
- [ ] Linux (GNOME, KDE, or XFCE with a tray): the icon appears. Its colour is grey when idle and green when running.
- [ ] Open the tray menu. The first line reads "Not running" when idle, or "Running: <task> H:MM:SS" while a task runs, and the elapsed time is current when the menu opens.
- [ ] Stop is enabled only while a task runs, and it stops the task.
- [ ] Up to five recent tasks are listed. Clicking one starts it.
- [ ] Open Tracker and Open History each bring the window to the front on the matching screen.
- [ ] Clicking the window's red close button hides the window and leaves the app running in the tray. The timer keeps running.
- [ ] Quit from the tray menu exits the app. Cmd+Q (Ctrl+Q on Linux) and File > Quit also exit it.
- [ ] Without a system tray (for example, GNOME without the AppIndicator extension), closing the window quits the app, since there is no other way to reopen it.

## Phase 7: macOS menu bar text

- [ ] While a task runs, its elapsed time (H:MM:SS) appears in the menu bar next to the tray icon, and it counts up every second.
- [ ] When the task stops, the text disappears and only the icon remains.
- [ ] Clicking the elapsed-time text opens the Tracker window.
- [ ] The text uses the system menu bar font and is legible in light and dark menu bars.
- [ ] The text does not jump in position noticeably as the digits change.

## Phase 8: Robustness and polish

- [ ] Start a timer, quit the app, and wait past 12 hours (or change the system clock forward). Relaunch. A "Running timer found" dialog appears with Keep running, Stop at last seen time, and Discard.
- [ ] Choose Stop at last seen time. The session ends at the last time the app was running (no later than the time you relaunched). Check it in History.
- [ ] Choose Discard. The session is removed from History.
- [ ] Choose Keep running. The timer keeps counting.
- [ ] With a timer running past the reminder threshold (default 8 hours), a "Still tracking?" message appears once. It does not repeat for the same session.
- [ ] File > Settings… shows the reminder threshold. Changing it to a new value takes effect on the next check without restarting.
- [ ] History > Export CSV… saves the rows in the selected date range. Open the file in a spreadsheet: the columns are date, start, end, duration_seconds, task, and task names containing commas or quotes stay in one cell.
- [ ] Sleep the computer while a timer runs. After wake, the elapsed time reflects real time, not the time the app was awake.
