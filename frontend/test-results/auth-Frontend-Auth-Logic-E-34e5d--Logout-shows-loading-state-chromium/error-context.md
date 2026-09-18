# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth.spec.ts >> Frontend Auth Logic E2E Tests >> Logout Logic - Token Invalidation & State Clearing >> TC3.2: Logout shows loading state
- Location: tests/e2e/auth.spec.ts:185:5

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
  184 | 
  185 |     test('TC3.2: Logout shows loading state', async ({ page }) => {
  186 |       // Login
  187 |       await page.goto('/login');
  188 |       await page.fill('input[id="username"]', 'testuser');
  189 |       await page.fill('input[id="password"]', 'password123');
  190 |       await page.click('button[type="submit"]');
> 191 |       await page.waitForURL('**/');
      |                  ^ Error: page.waitForURL: Test timeout of 30000ms exceeded.
  192 | 
  193 |       // Slow down logout API
  194 |       await page.route('**/api/user/logout', async (route) => {
  195 |         await new Promise((resolve) => setTimeout(resolve, 1000));
  196 |         await route.continue();
  197 |       });
  198 | 
  199 |       const logoutButton = page.locator('button:has-text("Logout")');
  200 | 
  201 |       // Click logout
  202 |       await logoutButton.click();
  203 | 
  204 |       // Check loading state
  205 |       await expect(logoutButton).toContainText('Logging out...');
  206 |       await expect(logoutButton).toBeDisabled();
  207 | 
  208 |       // Wait for completion
  209 |       await page.waitForURL('**/login');
  210 |     });
  211 | 
  212 |     test('TC3.3: Cannot use token after logout', async ({ page }) => {
  213 |       // Login and capture token
  214 |       await page.goto('/login');
  215 |       await page.fill('input[id="username"]', 'testuser');
  216 |       await page.fill('input[id="password"]', 'password123');
  217 | 
  218 |       let tokenFromLogin: string | null = null;
  219 | 
  220 |       // Capture token from login response
  221 |       page.on('response', async (response) => {
  222 |         if (response.url().includes('/api/user/login') && response.status() === 200) {
  223 |           const data = await response.json();
  224 |           tokenFromLogin = data.token;
  225 |         }
  226 |       });
  227 | 
  228 |       await page.click('button[type="submit"]');
  229 |       await page.waitForURL('**/');
  230 | 
  231 |       // Wait for token to be captured
  232 |       await page.waitForTimeout(500);
  233 | 
  234 |       // Logout
  235 |       await page.click('button:has-text("Logout")');
  236 |       await page.waitForURL('**/login');
  237 | 
  238 |       // Try to use old token via direct API call
  239 |       if (tokenFromLogin) {
  240 |         const response = await page.context().request.get('/api/user/valid', {
  241 |           headers: { Authorization: `Bearer ${tokenFromLogin}` },
  242 |         });
  243 | 
  244 |         // Should be rejected
  245 |         expect(response.status()).toBe(401);
  246 |       }
  247 |     });
  248 |   });
  249 | 
  250 |   test.describe('In-Memory Token Storage Logic', () => {
  251 |     test('TC4.1: Token is not in localStorage', async ({ page }) => {
  252 |       // Login
  253 |       await page.goto('/login');
  254 |       await page.fill('input[id="username"]', 'testuser');
  255 |       await page.fill('input[id="password"]', 'password123');
  256 |       await page.click('button[type="submit"]');
  257 |       await page.waitForURL('**/');
  258 | 
  259 |       // Check localStorage
  260 |       const storage = await page.evaluate(() => ({
  261 |         token: window.localStorage.getItem('token'),
  262 |         auth: window.localStorage.getItem('auth'),
  263 |       }));
  264 | 
  265 |       expect(storage.token).toBeNull();
  266 |       expect(storage.auth).toBeNull();
  267 |     });
  268 | 
  269 |     test('TC4.2: Token is not in sessionStorage', async ({ page }) => {
  270 |       // Login
  271 |       await page.goto('/login');
  272 |       await page.fill('input[id="username"]', 'testuser');
  273 |       await page.fill('input[id="password"]', 'password123');
  274 |       await page.click('button[type="submit"]');
  275 |       await page.waitForURL('**/');
  276 | 
  277 |       // Check sessionStorage
  278 |       const storage = await page.evaluate(() => ({
  279 |         token: window.sessionStorage.getItem('token'),
  280 |         auth: window.sessionStorage.getItem('auth'),
  281 |       }));
  282 | 
  283 |       expect(storage.token).toBeNull();
  284 |       expect(storage.auth).toBeNull();
  285 |     });
  286 | 
  287 |     test('TC4.3: Refresh clears token and redirects to login', async ({
  288 |       page,
  289 |     }) => {
  290 |       // Login
  291 |       await page.goto('/login');
```