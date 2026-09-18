# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth-logic.spec.ts >> Frontend Auth Logic E2E Tests >> Login Logic >> TC1: Valid credentials transition from login to home
- Location: tests/e2e/auth-logic.spec.ts:44:5

# Error details

```
Test timeout of 30000ms exceeded.
```

```
Error: page.waitForResponse: Test timeout of 30000ms exceeded.
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
  1   | import { test, expect, Page } from '@playwright/test';
  2   | 
  3   | interface AuthState {
  4   |   token: string | null;
  5   |   user: { id: number; username: string } | null;
  6   |   isAuthenticated: boolean;
  7   | }
  8   | 
  9   | /**
  10  |  * Frontend E2E Tests - Application Logic
  11  |  * Tests the state machine, Redux integration, and UI behavior based on auth state
  12  |  */
  13  | 
  14  | test.describe('Frontend Auth Logic E2E Tests', () => {
  15  |   const BASE_URL = 'http://localhost:5173';
  16  |   const API_URL = 'http://localhost:3000';
  17  | 
  18  |   // Helper: Get Redux auth state from page
  19  |   async function getAuthState(page: Page): Promise<AuthState> {
  20  |     return await page.evaluate(() => {
  21  |       // Access Redux store from window if exposed in dev
  22  |       const state = (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__?.
  23  |         (window as any).store?.getState?.()?.auth || {
  24  |         token: null,
  25  |         user: null,
  26  |         isAuthenticated: false,
  27  |       };
  28  |       return state;
  29  |     });
  30  |   }
  31  | 
  32  |   // Helper: Get Redux state via window (for tests)
  33  |   async function getReduxState(page: Page): Promise<any> {
  34  |     // Since Redux state isn't directly accessible, we'll verify via API calls
  35  |     // and UI state instead
  36  |     return null;
  37  |   }
  38  | 
  39  |   // ========================================================================
  40  |   // LOGIN LOGIC TESTS
  41  |   // ========================================================================
  42  | 
  43  |   test.describe('Login Logic', () => {
  44  |     test('TC1: Valid credentials transition from login to home', async ({ page }) => {
  45  |       // Start at login
  46  |       await page.goto(`${BASE_URL}/login`);
  47  |       await expect(page.locator('h1')).toContainText('Login');
  48  | 
  49  |       // Intercept the login API call to verify payload
> 50  |       const loginRequest = page.waitForResponse(
      |                                 ^ Error: page.waitForResponse: Test timeout of 30000ms exceeded.
  51  |         response => response.url().includes('/api/user/login') && response.status() === 200
  52  |       );
  53  | 
  54  |       // Submit form
  55  |       await page.fill('input[id="username"]', 'testuser');
  56  |       await page.fill('input[id="password"]', 'password123');
  57  |       await page.click('button[type="submit"]');
  58  | 
  59  |       // Verify API was called correctly
  60  |       const response = await loginRequest;
  61  |       const data = await response.json();
  62  | 
  63  |       expect(data.token).toBeTruthy();
  64  |       expect(data.user.username).toBe('testuser');
  65  |       expect(data.user.id).toBe(1);
  66  | 
  67  |       // Verify redirect to home
  68  |       await page.waitForURL(`${BASE_URL}/`);
  69  |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  70  |     });
  71  | 
  72  |     test('TC2: Invalid credentials show error and remain on login', async ({ page }) => {
  73  |       await page.goto(`${BASE_URL}/login`);
  74  | 
  75  |       // Submit invalid credentials
  76  |       await page.fill('input[id="username"]', 'testuser');
  77  |       await page.fill('input[id="password"]', 'wrongpassword');
  78  |       await page.click('button[type="submit"]');
  79  | 
  80  |       // Verify error message appears
  81  |       await expect(page.locator('text=Invalid username or password')).toBeVisible();
  82  | 
  83  |       // Verify URL hasn't changed
  84  |       await expect(page).toHaveURL(`${BASE_URL}/login`);
  85  | 
  86  |       // Verify form is still visible and editable
  87  |       await expect(page.locator('input[id="username"]')).toHaveValue('testuser');
  88  |       await expect(page.locator('input[id="password"]')).toHaveValue('wrongpassword');
  89  | 
  90  |       // Form is still functional - can retry
  91  |       await page.fill('input[id="password"]', 'password123');
  92  |       await page.click('button[type="submit"]');
  93  | 
  94  |       // Should now succeed
  95  |       await page.waitForURL(`${BASE_URL}/`);
  96  |     });
  97  | 
  98  |     test('TC3: Missing fields prevent submission', async ({ page }) => {
  99  |       await page.goto(`${BASE_URL}/login`);
  100 | 
  101 |       // Try to submit without username
  102 |       await page.fill('input[id="password"]', 'password123');
  103 |       await page.click('button[type="submit"]');
  104 | 
  105 |       // Should get error (client-side or server)
  106 |       const errorVisible = await page.locator('text=Invalid username or password, text=required').isVisible().catch(() => false);
  107 |       const stillOnLogin = page.url().includes('/login');
  108 | 
  109 |       expect(errorVisible || stillOnLogin).toBeTruthy();
  110 |     });
  111 | 
  112 |     test('TC4: Login button shows loading state during submission', async ({ page }) => {
  113 |       await page.goto(`${BASE_URL}/login`);
  114 | 
  115 |       // Set up slow network to observe loading state
  116 |       await page.route('**/api/user/login', async (route) => {
  117 |         await new Promise(resolve => setTimeout(resolve, 1000));
  118 |         await route.continue();
  119 |       });
  120 | 
  121 |       await page.fill('input[id="username"]', 'testuser');
  122 |       await page.fill('input[id="password"]', 'password123');
  123 | 
  124 |       const button = page.locator('button[type="submit"]');
  125 | 
  126 |       // Check initial state
  127 |       await expect(button).toContainText('Login');
  128 | 
  129 |       // Click and immediately check for loading state
  130 |       await button.click();
  131 |       await expect(button).toContainText('Logging in...');
  132 |       await expect(button).toBeDisabled();
  133 | 
  134 |       // Wait for completion
  135 |       await page.waitForURL(`${BASE_URL}/`);
  136 |     });
  137 |   });
  138 | 
  139 |   // ========================================================================
  140 |   // SESSION VALIDATION LOGIC TESTS
  141 |   // ========================================================================
  142 | 
  143 |   test.describe('Session Validation Logic', () => {
  144 |     test('TC1: Valid token allows home page access', async ({ page }) => {
  145 |       // First login to get token
  146 |       await page.goto(`${BASE_URL}/login`);
  147 |       await page.fill('input[id="username"]', 'testuser');
  148 |       await page.fill('input[id="password"]', 'password123');
  149 |       await page.click('button[type="submit"]');
  150 | 
```