# Frontend E2E Test Cases

## Setup
- Backend running on `http://localhost:3000`
- Frontend running on `http://localhost:5173`
- Database seeded with testuser / password123
- Vite proxy configured: `/api` → `localhost:3000`
- Browser: Chrome/Firefox/Safari (modern)
- Testing tool: Playwright/Cypress recommended

---

## Test Suite 1: Login Page (Route /login)

### TC1.1: Login Page Initial Load
**Steps**:
1. Open `http://localhost:5173/login`

**Expected**:
- Page loads successfully
- Title: "Login" heading visible
- Two input fields: username and password
- Submit button: "Login"
- No error message displayed
- Page background: white (greyscale theme)
- Font: system sans-serif
- Username input is empty
- Password input is empty
- Login button is enabled

---

### TC1.2: Redirect to Login When Not Authenticated
**Steps**:
1. Open `http://localhost:5173/` (home page)

**Expected**:
- Should redirect to `http://localhost:5173/login`
- Browser URL updates immediately
- ProtectedRoute prevents access to home

---

### TC1.3: Redirect Away from Login When Authenticated
**Precondition**: User is logged in (token in Redux state)

**Steps**:
1. With active session, open `http://localhost:5173/login`

**Expected**:
- Should redirect to `http://localhost:5173/` (home)
- Browser URL updates immediately

---

## Test Suite 2: Login Form Submission

### TC2.1: Valid Login - testuser / password123
**Steps**:
1. On login page, enter username: "testuser"
2. Enter password: "password123"
3. Click "Login" button
4. Wait for response

**Expected**:
- Button changes to "Logging in..." while submitting
- API call: `POST /api/user/login` with credentials
- Response: `{ token: "...", user: { id: 1, username: "testuser" } }`
- Status: `200`
- Redux state updated: `{ token: "...", user: {...}, isAuthenticated: true }`
- Redirects to home page (`/`)
- No error message visible

**Network Verification**:
- In DevTools Network tab, verify POST request was sent
- Response payload contains token and user data

---

### TC2.2: Invalid Username
**Steps**:
1. Enter username: "nonexistent"
2. Enter password: "password123"
3. Click "Login"

**Expected**:
- Button shows "Logging in..."
- API call is made
- Response: `401 { error: "Invalid credentials" }`
- Error message displayed: "Invalid username or password"
- Error text shown in red box
- Form remains visible
- Login button re-enabled
- No redirect

---

### TC2.3: Invalid Password
**Steps**:
1. Enter username: "testuser"
2. Enter password: "wrongpassword"
3. Click "Login"

**Expected**:
- API call is made
- Response: `401 { error: "Invalid credentials" }`
- Error message displayed: "Invalid username or password"
- Form remains visible
- Token NOT stored in Redux
- isAuthenticated remains false

---

### TC2.4: Empty Username
**Steps**:
1. Username field: empty
2. Password field: "password123"
3. Click "Login"

**Expected**:
- Form may validate client-side OR submit with empty values
- If submitted: API responds `400`
- If client-validated: Error message shown
- Form remains on page

---

### TC2.5: Empty Password
**Steps**:
1. Username field: "testuser"
2. Password field: empty
3. Click "Login"

**Expected**:
- Similar to TC2.4
- Form doesn't submit or shows error

---

### TC2.6: Both Fields Empty
**Steps**:
1. Both username and password empty
2. Click "Login"

**Expected**:
- Form validation or API error
- No submission to empty credentials
- Remains on login page

---

## Test Suite 3: Form State and Validation

### TC3.1: Input Field Focus and Blur
**Steps**:
1. Click on username input
2. Type "testuser"
3. Click on password input
4. Type "password123"

**Expected**:
- Both inputs accept text normally
- Values are visible as typed
- Password field masks characters (shows dots/asterisks)

---

### TC3.2: Form Clears on Error
**Steps**:
1. Login with invalid credentials
2. Error message appears
3. Close error or try again

**Expected**:
- Password field is still visible (not cleared for security review)
- Username field retains value (user can edit)
- User can re-submit

---

### TC3.3: Button Disabled During Submission
**Steps**:
1. Fill in valid credentials
2. Click "Login"
3. Before response arrives, attempt to click again

**Expected**:
- Button shows "Logging in..." text
- Button is disabled (cursor: not-allowed)
- Cannot trigger multiple submissions
- Only one API call is made

---

## Test Suite 4: Greyscale Theme

### TC4.1: Theme Colors on Login Page
**Steps**:
1. Open login page
2. Inspect element colors

**Expected**:
- Background: white (#ffffff) or very light grey
- Card/container: white with light grey border
- Text: black (#111111) or very dark grey
- Button: black background with white text
- Error text: red (#b91c1c or similar)
- No colors outside black/white/grey palette
- No shadows or gradients

---

### TC4.2: Typography
**Steps**:
1. Open login page
2. Check fonts

**Expected**:
- Font stack: system sans-serif (San Francisco, Segoe UI, etc.)
- Heading: bold, 28px
- Labels: 14px, medium weight
- Inputs: 14px
- Line height: comfortable (1.5+)
- Text is legible on white background

---

### TC4.3: Responsive Design
**Steps**:
1. Open login page
2. Resize browser to mobile (375px)
3. Check desktop (1920px)
4. Check tablet (768px)

**Expected**:
- Login form remains centered
- Form width stays reasonable (~360px max)
- Inputs stack vertically
- Button full width
- Text remains readable
- No overflow or cutoff

---

## Test Suite 5: Home Page (Route /)

### TC5.1: Home Page Load When Authenticated
**Precondition**: User logged in with valid token

**Steps**:
1. After successful login, page should show home
2. Or open `http://localhost:5173/` directly with token in Redux

**Expected**:
- URL: `http://localhost:5173/`
- Header visible with "ghoralantinu" title
- "Welcome, testuser!" message displayed
- "Logout" button visible in header
- Main content area shows greeting
- Page background: white (greyscale)
- No error messages

---

### TC5.2: Greeting Shows Correct Username
**Precondition**: Logged in as testuser

**Steps**:
1. Navigate to home page
2. Check welcome message

**Expected**:
- Message reads: "Welcome, testuser!"
- Username matches logged-in user
- Message dynamically inserted (from Redux state)

---

### TC5.3: Session Validation on Mount
**Steps**:
1. After login, open DevTools Network tab
2. Observe home page loads
3. Check Network for API calls

**Expected**:
- `GET /api/user/valid` request is sent on component mount
- Request includes `Authorization: Bearer <token>` header
- Response: `200 { valid: true, user: {...} }`
- User remains on home page

---

### TC5.4: Invalid Token on Home Page
**Precondition**: Token is expired or invalid in Redux state

**Steps**:
1. Manually set Redux state to have invalid token
2. Open home page
3. Observe session check

**Expected**:
- `GET /api/user/valid` is called
- Response: `401 { error: "Invalid or expired token" }`
- Redux state cleared (clearCredentials)
- Redirected to login page
- Error is handled silently (no crash)

---

### TC5.5: Logout Button Click
**Precondition**: User logged in on home page

**Steps**:
1. Click "Logout" button
2. Observe API call and state changes

**Expected**:
- Button text changes to "Logging out..."
- Button is disabled
- `POST /api/user/logout` is sent with Bearer token
- Response: `200 { message: "Logged out successfully" }`
- Redux state cleared (token = null, user = null, isAuthenticated = false)
- Redirected to login page (`/login`)
- Button re-enabled on new page

---

### TC5.6: Logout with Network Error
**Precondition**: User on home page, mock network failure on logout

**Steps**:
1. Set up network interceptor to fail logout
2. Click "Logout" button
3. Observe behavior

**Expected**:
- API call fails (e.g., 500 error)
- Redux state is still cleared (fail-open behavior)
- User is still redirected to login
- No error popup (error handled gracefully)

---

## Test Suite 6: Header and Navigation

### TC6.1: Header Layout
**Precondition**: User on home page

**Steps**:
1. Open DevTools and inspect header

**Expected**:
- Header background: light grey (#f7f7f7)
- Top border or bottom border (1px solid, dark grey)
- Padding: 16px
- Flexbox layout: space-between (title left, button right)
- Title "ghoralantinu" on left
- Logout button on right

---

### TC6.2: Logout Button Styling
**Steps**:
1. Home page loaded
2. Inspect logout button

**Expected**:
- Background: black (#111111)
- Text: white
- Padding: ~8px 16px
- Font size: 14px
- Border radius: 2px (minimal)
- Cursor: pointer
- Hover state: slightly different (opacity or shade)

---

## Test Suite 7: Page Refresh Behavior

### TC7.1: Refresh on Home Page
**Precondition**: User logged in, on home page

**Steps**:
1. Press F5 or Cmd+R to refresh
2. Wait for page to reload

**Expected**:
- Page reloads in browser
- Redux state is cleared (in-memory token lost)
- Session check (`GET /api/user/valid`) would fail because token is gone
- User redirected to login page
- This is expected behavior for in-memory token storage

---

### TC7.2: Refresh on Login Page
**Steps**:
1. On login page, refresh browser

**Expected**:
- Page reloads
- Remains on login page
- Form is cleared (username and password empty)
- No error message

---

### TC7.3: Direct URL Navigation to Protected Route
**Precondition**: Not logged in, token not in Redux

**Steps**:
1. Open new browser window/tab
2. Type `http://localhost:5173/`

**Expected**:
- ProtectedRoute checks isAuthenticated
- Redirects to `/login`
- User sees login page

---

## Test Suite 8: Token Storage Verification

### TC8.1: Token in Redux, Not in localStorage
**Precondition**: User logged in

**Steps**:
1. Open DevTools Console
2. Type: `localStorage.getItem('token')`
3. Type: `sessionStorage.getItem('token')`
4. Check Redux DevTools or inspect Redux state

**Expected**:
- localStorage returns null
- sessionStorage returns null
- Token exists ONLY in Redux state
- Redux DevTools shows: `auth.token = "..."`
- Token is in-memory only

---

### TC8.2: Token Not in Cookies
**Steps**:
1. Open DevTools Application tab
2. Check Cookies for localhost:5173

**Expected**:
- No auth/token/jwt cookie present
- Only session/dev cookies if any

---

## Test Suite 9: Error States

### TC9.1: Network Error on Login
**Steps**:
1. Disable network (DevTools → Offline)
2. Try to login

**Expected**:
- API call fails
- Error message displayed
- No redirect
- Form remains visible for retry

---

### TC9.2: Network Error on Logout
**Steps**:
1. Disable network
2. Click logout button

**Expected**:
- API call fails
- Redux state still cleared (fail-open)
- Redirected to login anyway
- User is logged out locally

---

### TC9.3: 500 Server Error on Login
**Steps**:
1. Mock backend to return 500 on login
2. Attempt login

**Expected**:
- Error message shown
- No redirect
- Form usable for retry

---

## Test Suite 10: Multi-Tab/Window Behavior

### TC10.1: Logout in One Tab, Other Tab Effect
**Precondition**: Logged in, two tabs open to home page

**Steps**:
1. In Tab 1: Click logout
2. Switch to Tab 2
3. Try to interact with home page

**Expected**:
- Tab 1: Logs out, redirects to login
- Tab 2: Still shows home page (no shared state across tabs)
- Tab 2 is still logged in (Redux state in memory is independent per tab)
- If Tab 2 tries to make API calls with token, they succeed (token is valid server-side until expiry)

**Note**: This is expected behavior for in-memory storage with multi-tab. Shared storage (localStorage) would sync across tabs.

---

## Test Suite 11: Routing

### TC11.1: Navigate to Unknown Route
**Steps**:
1. Open `http://localhost:5173/unknown`

**Expected**:
- Catch-all route redirects to `/`
- ProtectedRoute checks auth
- If not logged in: redirects to `/login`
- If logged in: shows home page

---

### TC11.2: Route History
**Precondition**: Logged in, on home page

**Steps**:
1. Click logout
2. Browser back button

**Expected**:
- Back button does NOT navigate back to home
- ProtectedRoute uses `replace: true`, so history is not pushed
- Browser history is clean

---

## Test Suite 12: Accessibility

### TC12.1: Form Labels
**Steps**:
1. Inspect login form HTML
2. Check that inputs have associated labels

**Expected**:
- Username input has `<label for="username">`
- Password input has `<label for="password">`
- Labels are clickable (focus input on click)

---

### TC12.2: Keyboard Navigation
**Steps**:
1. On login page
2. Press Tab to navigate fields
3. Enter to submit

**Expected**:
- Tab key focuses username → password → submit button
- Enter key in password field may submit form (or Tab to button)
- Logout button is keyboard accessible

---

### TC12.3: Focus Indicators
**Steps**:
1. Open login page
2. Tab through form elements
3. Check focus visibility

**Expected**:
- Focus indicators visible on inputs and button
- Easy to see which element is focused
- No invisible focus states

---

## Test Automation Script (Playwright)

```typescript
import { test, expect } from '@playwright/test';

test.describe('Frontend E2E Tests', () => {
  const BASE_URL = 'http://localhost:5173';

  test.beforeEach(async ({ page }) => {
    await page.goto(`${BASE_URL}/login`);
  });

  test('TC1.1: Login page loads', async ({ page }) => {
    await expect(page).toHaveTitle(/ghoralantinu/);
    await expect(page.locator('h1')).toContainText('Login');
    await expect(page.locator('input[id="username"]')).toBeVisible();
    await expect(page.locator('input[id="password"]')).toBeVisible();
    await expect(page.locator('button[type="submit"]')).toContainText('Login');
  });

  test('TC2.1: Valid login redirects to home', async ({ page }) => {
    await page.fill('input[id="username"]', 'testuser');
    await page.fill('input[id="password"]', 'password123');
    await page.click('button[type="submit"]');
    await page.waitForURL('http://localhost:5173/');
    await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  });

  test('TC2.2: Invalid credentials show error', async ({ page }) => {
    await page.fill('input[id="username"]', 'invalid');
    await page.fill('input[id="password"]', 'password123');
    await page.click('button[type="submit"]');
    await expect(page.locator('text=Invalid username or password')).toBeVisible();
    await expect(page).toHaveURL('http://localhost:5173/login');
  });

  test('TC5.5: Logout button redirects to login', async ({ page }) => {
    // First login
    await page.fill('input[id="username"]', 'testuser');
    await page.fill('input[id="password"]', 'password123');
    await page.click('button[type="submit"]');
    await page.waitForURL('http://localhost:5173/');

    // Logout
    await page.click('button:has-text("Logout")');
    await page.waitForURL('http://localhost:5173/login');
    await expect(page).toHaveURL('http://localhost:5173/login');
  });

  test('TC8.1: Token in Redux, not localStorage', async ({ page }) => {
    // Login first
    await page.fill('input[id="username"]', 'testuser');
    await page.fill('input[id="password"]', 'password123');
    await page.click('button[type="submit"]');
    await page.waitForURL('http://localhost:5173/');

    // Check localStorage
    const localStorageToken = await page.evaluate(() => localStorage.getItem('token'));
    expect(localStorageToken).toBeNull();

    // Check sessionStorage
    const sessionStorageToken = await page.evaluate(() => sessionStorage.getItem('token'));
    expect(sessionStorageToken).toBeNull();
  });

  test('TC7.1: Refresh on home redirects to login', async ({ page }) => {
    // Login
    await page.fill('input[id="username"]', 'testuser');
    await page.fill('input[id="password"]', 'password123');
    await page.click('button[type="submit"]');
    await page.waitForURL('http://localhost:5173/');

    // Refresh
    await page.reload();
    await page.waitForURL('http://localhost:5173/login');
    await expect(page).toHaveURL('http://localhost:5173/login');
  });

  test('TC4.1: Greyscale theme colors', async ({ page }) => {
    const body = page.locator('body');
    const bgColor = await body.evaluate(el => window.getComputedStyle(el).backgroundColor);
    expect(bgColor).toMatch(/white|rgb\(255|rgb\(247/);

    const button = page.locator('button[type="submit"]');
    const btnColor = await button.evaluate(el => window.getComputedStyle(el).backgroundColor);
    expect(btnColor).toMatch(/black|rgb\(17|rgb\(0/);
  });
});
```

---

## Test Coverage Summary

| Suite | Test Cases | Focus |
|-------|-----------|-------|
| 1. Login Page | 3 | Page load, redirects, auth check |
| 2. Form Submission | 6 | Valid/invalid inputs, error handling |
| 3. Form State | 3 | Input validation, button states |
| 4. Theme | 3 | Colors, typography, responsive |
| 5. Home Page | 5 | Load, greeting, session validation |
| 6. Header & Nav | 2 | Layout, button styling |
| 7. Page Refresh | 3 | Refresh behavior, in-memory token loss |
| 8. Token Storage | 2 | Redux only, no localStorage |
| 9. Error States | 3 | Network errors, server errors |
| 10. Multi-Tab | 1 | Cross-tab behavior |
| 11. Routing | 2 | Unknown routes, history |
| 12. Accessibility | 3 | Labels, keyboard nav, focus |
| **Total** | **36** | **Comprehensive UI/UX flow** |

---

## Manual Testing Checklist

```
[ ] Login page displays correctly
[ ] Form accepts username and password input
[ ] Valid credentials log in successfully
[ ] Invalid credentials show error
[ ] Home page shows welcome message with username
[ ] Logout button works
[ ] Token is not in localStorage or cookies
[ ] Refresh redirects to login
[ ] Direct home page URL redirects to login when not authenticated
[ ] Greyscale theme is applied (no colors outside palette)
[ ] Mobile responsive (375px, 768px, 1920px)
[ ] Keyboard navigation works (Tab, Enter)
[ ] Focus indicators visible
[ ] Error messages display clearly
[ ] Network errors handled gracefully
[ ] No console errors or warnings
[ ] Page performance is acceptable (<2s load time)
```
