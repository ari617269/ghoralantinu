# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth.spec.ts >> Frontend Auth Logic E2E Tests >> Login Logic - Form Submission & State Update >> TC1.3: Loading state shown during submission
- Location: tests/e2e/auth.spec.ts:59:5

# Error details

```
Test timeout of 30000ms exceeded.
```

```
Error: page.waitForURL: Test timeout of 30000ms exceeded.
=========================== logs ===========================
waiting for navigation to "**/" until "load"
============================================================
```

# Page snapshot

```yaml
- generic [ref=e4]:
  - heading "Login" [level=1] [ref=e5]
  - generic [ref=e6]:
    - generic [ref=e7]: Username
    - textbox "Username" [ref=e8]:
      - /placeholder: testuser
      - text: testuser
    - generic [ref=e9]: Password
    - textbox "Password" [ref=e10]:
      - /placeholder: password123
      - text: password123
    - generic [ref=e11]: Invalid username or password
    - button "Login" [ref=e12] [cursor=pointer]
```

# Test source

```ts
  1   | import { test, expect } from '@playwright/test';
  2   | 
  3   | /**
  4   |  * Frontend E2E Tests - Testing Application Logic with Playwright
  5   |  * These tests verify the actual state machine, Redux integration, and auth flow
  6   |  */
  7   | 
  8   | test.describe('Frontend Auth Logic E2E Tests', () => {
  9   |   test.describe('Login Logic - Form Submission & State Update', () => {
  10  |     test('TC1.1: Valid credentials trigger login API and update Redux state', async ({
  11  |       page,
  12  |     }) => {
  13  |       // Navigate to login
  14  |       await page.goto('/login');
  15  |       await expect(page.locator('h1')).toContainText('Login');
  16  | 
  17  |       // Intercept login API
  18  |       const loginPromise = page.waitForResponse(
  19  |         (response) =>
  20  |           response.url().includes('/api/user/login') && response.status() === 200
  21  |       );
  22  | 
  23  |       // Submit form
  24  |       await page.fill('input[id="username"]', 'testuser');
  25  |       await page.fill('input[id="password"]', 'password123');
  26  |       await page.click('button[type="submit"]');
  27  | 
  28  |       // Verify API response
  29  |       const response = await loginPromise;
  30  |       const data = await response.json();
  31  | 
  32  |       expect(data.token).toBeTruthy();
  33  |       expect(data.user.username).toBe('testuser');
  34  | 
  35  |       // Verify redirect to home (implies Redux state was updated)
  36  |       await page.waitForURL('**/');
  37  |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  38  |     });
  39  | 
  40  |     test('TC1.2: Invalid credentials show error without redirecting', async ({
  41  |       page,
  42  |     }) => {
  43  |       await page.goto('/login');
  44  | 
  45  |       await page.fill('input[id="username"]', 'testuser');
  46  |       await page.fill('input[id="password"]', 'wrongpassword');
  47  |       await page.click('button[type="submit"]');
  48  | 
  49  |       // Error message should appear
  50  |       await expect(page.locator('text=Invalid username or password')).toBeVisible();
  51  | 
  52  |       // Should remain on login page (not redirected)
  53  |       expect(page.url()).toContain('/login');
  54  | 
  55  |       // Form should still be visible and functional
  56  |       await expect(page.locator('input[id="username"]')).toBeVisible();
  57  |     });
  58  | 
  59  |     test('TC1.3: Loading state shown during submission', async ({ page }) => {
  60  |       await page.goto('/login');
  61  | 
  62  |       // Slow down the API
  63  |       await page.route('**/api/user/login', async (route) => {
  64  |         await new Promise((resolve) => setTimeout(resolve, 1000));
  65  |         await route.continue();
  66  |       });
  67  | 
  68  |       const button = page.locator('button[type="submit"]');
  69  | 
  70  |       // Verify initial state
  71  |       await expect(button).toContainText('Login');
  72  | 
  73  |       // Submit form
  74  |       await page.fill('input[id="username"]', 'testuser');
  75  |       await page.fill('input[id="password"]', 'password123');
  76  |       await button.click();
  77  | 
  78  |       // Check loading state immediately
  79  |       await expect(button).toContainText('Logging in...');
  80  |       await expect(button).toBeDisabled();
  81  | 
  82  |       // Wait for completion
> 83  |       await page.waitForURL('**/');
      |                  ^ Error: page.waitForURL: Test timeout of 30000ms exceeded.
  84  |     });
  85  | 
  86  |     test('TC1.4: Session validation called on home mount', async ({ page }) => {
  87  |       await page.goto('/login');
  88  | 
  89  |       // Intercept session validation
  90  |       const validationPromise = page.waitForResponse(
  91  |         (response) =>
  92  |           response.url().includes('/api/user/valid') && response.status() === 200
  93  |       );
  94  | 
  95  |       // Login
  96  |       await page.fill('input[id="username"]', 'testuser');
  97  |       await page.fill('input[id="password"]', 'password123');
  98  |       await page.click('button[type="submit"]');
  99  | 
  100 |       // Session validation should be called
  101 |       await validationPromise;
  102 | 
  103 |       // Should be on home page
  104 |       await page.waitForURL('**/');
  105 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  106 |     });
  107 |   });
  108 | 
  109 |   test.describe('Protected Route Logic - Access Control', () => {
  110 |     test('TC2.1: Unauthenticated user cannot access home', async ({ page }) => {
  111 |       // Try to access home without auth
  112 |       await page.goto('/', { waitUntil: 'networkidle' });
  113 | 
  114 |       // Should redirect to login
  115 |       await page.waitForURL('**/login');
  116 |       await expect(page.locator('h1')).toContainText('Login');
  117 |     });
  118 | 
  119 |     test('TC2.2: Authenticated user can access home', async ({ page }) => {
  120 |       // First login
  121 |       await page.goto('/login');
  122 |       await page.fill('input[id="username"]', 'testuser');
  123 |       await page.fill('input[id="password"]', 'password123');
  124 |       await page.click('button[type="submit"]');
  125 | 
  126 |       // Should be on home
  127 |       await page.waitForURL('**/');
  128 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  129 |     });
  130 | 
  131 |     test('TC2.3: Cannot access login when already authenticated', async ({ page }) => {
  132 |       // Login
  133 |       await page.goto('/login');
  134 |       await page.fill('input[id="username"]', 'testuser');
  135 |       await page.fill('input[id="password"]', 'password123');
  136 |       await page.click('button[type="submit"]');
  137 |       await page.waitForURL('**/');
  138 | 
  139 |       // Try to navigate to login
  140 |       await page.goto('/login');
  141 | 
  142 |       // Should redirect back to home
  143 |       await page.waitForURL('**/');
  144 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  145 |     });
  146 | 
  147 |     test('TC2.4: Unknown routes redirect appropriately based on auth', async ({
  148 |       page,
  149 |     }) => {
  150 |       // Try unknown route without auth
  151 |       await page.goto('/unknown-page', { waitUntil: 'networkidle' });
  152 | 
  153 |       // Should redirect to login
  154 |       await page.waitForURL('**/login');
  155 |     });
  156 |   });
  157 | 
  158 |   test.describe('Logout Logic - Token Invalidation & State Clearing', () => {
  159 |     test('TC3.1: Logout calls API and clears Redux state', async ({ page }) => {
  160 |       // Login
  161 |       await page.goto('/login');
  162 |       await page.fill('input[id="username"]', 'testuser');
  163 |       await page.fill('input[id="password"]', 'password123');
  164 |       await page.click('button[type="submit"]');
  165 |       await page.waitForURL('**/');
  166 | 
  167 |       // Intercept logout API
  168 |       const logoutPromise = page.waitForResponse(
  169 |         (response) =>
  170 |           response.url().includes('/api/user/logout') && response.status() === 200
  171 |       );
  172 | 
  173 |       // Click logout
  174 |       await page.click('button:has-text("Logout")');
  175 | 
  176 |       // Verify API was called
  177 |       const response = await logoutPromise;
  178 |       expect(response.status()).toBe(200);
  179 | 
  180 |       // Verify redirect to login (implies Redux state was cleared)
  181 |       await page.waitForURL('**/login');
  182 |       await expect(page.locator('h1')).toContainText('Login');
  183 |     });
```