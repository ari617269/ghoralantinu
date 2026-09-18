# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth-logic.spec.ts >> Frontend Auth Logic E2E Tests >> Logout Logic >> TC4: Logout button shows loading state
- Location: tests/e2e/auth-logic.spec.ts:309:5

# Error details

```
Test timeout of 30000ms exceeded.
```

```
Error: page.waitForURL: Test timeout of 30000ms exceeded.
=========================== logs ===========================
waiting for navigation to "http://localhost:5173/" until "load"
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
  215 |   // ========================================================================
  216 | 
  217 |   test.describe('Logout Logic', () => {
  218 |     test('TC1: Logout invalidates token and redirects to login', async ({ page }) => {
  219 |       // Login
  220 |       await page.goto(`${BASE_URL}/login`);
  221 |       await page.fill('input[id="username"]', 'testuser');
  222 |       await page.fill('input[id="password"]', 'password123');
  223 |       await page.click('button[type="submit"]');
  224 |       await page.waitForURL(`${BASE_URL}/`);
  225 | 
  226 |       // Get initial token (from network inspection)
  227 |       let tokenBeforeLogout: string | null = null;
  228 |       page.on('response', async (response) => {
  229 |         if (response.url().includes('/api/user/login')) {
  230 |           const data = await response.json();
  231 |           tokenBeforeLogout = data.token;
  232 |         }
  233 |       });
  234 | 
  235 |       // Logout
  236 |       const logoutRequest = page.waitForResponse(
  237 |         response => response.url().includes('/api/user/logout')
  238 |       );
  239 | 
  240 |       await page.click('button:has-text("Logout")');
  241 | 
  242 |       // Verify logout API was called
  243 |       const response = await logoutRequest;
  244 |       expect(response.status()).toBe(200);
  245 | 
  246 |       // Verify redirected to login
  247 |       await page.waitForURL(`${BASE_URL}/login`);
  248 |       await expect(page.locator('h1')).toContainText('Login');
  249 | 
  250 |       // Verify form is cleared
  251 |       await expect(page.locator('input[id="username"]')).toHaveValue('');
  252 |       await expect(page.locator('input[id="password"]')).toHaveValue('');
  253 |     });
  254 | 
  255 |     test('TC2: Logout clears Redux auth state', async ({ page }) => {
  256 |       // Login
  257 |       await page.goto(`${BASE_URL}/login`);
  258 |       await page.fill('input[id="username"]', 'testuser');
  259 |       await page.fill('input[id="password"]', 'password123');
  260 |       await page.click('button[type="submit"]');
  261 |       await page.waitForURL(`${BASE_URL}/`);
  262 | 
  263 |       // Logout
  264 |       await page.click('button:has-text("Logout")');
  265 |       await page.waitForURL(`${BASE_URL}/login`);
  266 | 
  267 |       // Verify accessing home redirects to login (because state is cleared)
  268 |       await page.goto(`${BASE_URL}/`);
  269 |       await page.waitForURL(`${BASE_URL}/login`);
  270 |     });
  271 | 
  272 |     test('TC3: Cannot use token after logout', async ({ page }) => {
  273 |       // Login
  274 |       await page.goto(`${BASE_URL}/login`);
  275 |       await page.fill('input[id="username"]', 'testuser');
  276 |       await page.fill('input[id="password"]', 'password123');
  277 | 
  278 |       let tokenFromLogin: string | null = null;
  279 | 
  280 |       page.on('response', async (response) => {
  281 |         if (response.url().includes('/api/user/login') && response.status() === 200) {
  282 |           const data = await response.json();
  283 |           tokenFromLogin = data.token;
  284 |         }
  285 |       });
  286 | 
  287 |       await page.click('button[type="submit"]');
  288 |       await page.waitForURL(`${BASE_URL}/`);
  289 | 
  290 |       // Wait a moment for token to be captured
  291 |       await page.waitForTimeout(500);
  292 | 
  293 |       // Logout
  294 |       await page.click('button:has-text("Logout")');
  295 |       await page.waitForURL(`${BASE_URL}/login`);
  296 | 
  297 |       // Try to use old token to validate - should fail
  298 |       if (tokenFromLogin) {
  299 |         const validationResponse = await fetch(`${API_URL}/api/user/valid`, {
  300 |           headers: { Authorization: `Bearer ${tokenFromLogin}` },
  301 |         });
  302 | 
  303 |         expect(validationResponse.status).toBe(401);
  304 |         const data = await validationResponse.json();
  305 |         expect(data.error).toContain('revoked');
  306 |       }
  307 |     });
  308 | 
  309 |     test('TC4: Logout button shows loading state', async ({ page }) => {
  310 |       // Login
  311 |       await page.goto(`${BASE_URL}/login`);
  312 |       await page.fill('input[id="username"]', 'testuser');
  313 |       await page.fill('input[id="password"]', 'password123');
  314 |       await page.click('button[type="submit"]');
> 315 |       await page.waitForURL(`${BASE_URL}/`);
      |                  ^ Error: page.waitForURL: Test timeout of 30000ms exceeded.
  316 | 
  317 |       // Slow down logout API
  318 |       await page.route('**/api/user/logout', async (route) => {
  319 |         await new Promise(resolve => setTimeout(resolve, 1000));
  320 |         await route.continue();
  321 |       });
  322 | 
  323 |       const logoutButton = page.locator('button:has-text("Logout")');
  324 | 
  325 |       // Click logout
  326 |       await logoutButton.click();
  327 | 
  328 |       // Verify loading state
  329 |       await expect(logoutButton).toContainText('Logging out...');
  330 |       await expect(logoutButton).toBeDisabled();
  331 | 
  332 |       // Wait for completion
  333 |       await page.waitForURL(`${BASE_URL}/login`);
  334 |     });
  335 |   });
  336 | 
  337 |   // ========================================================================
  338 |   // PROTECTED ROUTE LOGIC TESTS
  339 |   // ========================================================================
  340 | 
  341 |   test.describe('Protected Route Logic', () => {
  342 |     test('TC1: Unauthenticated user cannot access home', async ({ page }) => {
  343 |       await page.goto(`${BASE_URL}/`);
  344 |       await page.waitForURL(`${BASE_URL}/login`);
  345 |       await expect(page.locator('h1')).toContainText('Login');
  346 |     });
  347 | 
  348 |     test('TC2: Direct URL to home redirects to login when not authenticated', async ({ page }) => {
  349 |       // Don't login, just try to access home
  350 |       await page.goto(`${BASE_URL}/`, { waitUntil: 'networkidle' });
  351 | 
  352 |       // Should redirect immediately
  353 |       expect(page.url()).toContain('/login');
  354 |     });
  355 | 
  356 |     test('TC3: Unknown routes redirect to home (or login if not authenticated)', async ({ page }) => {
  357 |       // Try unknown route
  358 |       await page.goto(`${BASE_URL}/unknown-page`);
  359 | 
  360 |       // Should end up at login (because not authenticated)
  361 |       await page.waitForURL(`${BASE_URL}/login`);
  362 |     });
  363 | 
  364 |     test('TC4: Authenticated user can access home', async ({ page }) => {
  365 |       // Login
  366 |       await page.goto(`${BASE_URL}/login`);
  367 |       await page.fill('input[id="username"]', 'testuser');
  368 |       await page.fill('input[id="password"]', 'password123');
  369 |       await page.click('button[type="submit"]');
  370 | 
  371 |       // Should be on home
  372 |       await page.waitForURL(`${BASE_URL}/`);
  373 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  374 |     });
  375 | 
  376 |     test('TC5: Cannot access login when already authenticated', async ({ page }) => {
  377 |       // Login
  378 |       await page.goto(`${BASE_URL}/login`);
  379 |       await page.fill('input[id="username"]', 'testuser');
  380 |       await page.fill('input[id="password"]', 'password123');
  381 |       await page.click('button[type="submit"]');
  382 |       await page.waitForURL(`${BASE_URL}/`);
  383 | 
  384 |       // Try to go back to login
  385 |       await page.goto(`${BASE_URL}/login`);
  386 | 
  387 |       // Should redirect back to home
  388 |       await page.waitForURL(`${BASE_URL}/`);
  389 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  390 |     });
  391 |   });
  392 | 
  393 |   // ========================================================================
  394 |   // IN-MEMORY TOKEN STORAGE LOGIC TESTS
  395 |   // ========================================================================
  396 | 
  397 |   test.describe('In-Memory Token Storage Logic', () => {
  398 |     test('TC1: Token is not stored in localStorage', async ({ page }) => {
  399 |       // Login
  400 |       await page.goto(`${BASE_URL}/login`);
  401 |       await page.fill('input[id="username"]', 'testuser');
  402 |       await page.fill('input[id="password"]', 'password123');
  403 |       await page.click('button[type="submit"]');
  404 |       await page.waitForURL(`${BASE_URL}/`);
  405 | 
  406 |       // Check localStorage
  407 |       const localStorage = await page.evaluate(() => {
  408 |         return {
  409 |           token: window.localStorage.getItem('token'),
  410 |           auth: window.localStorage.getItem('auth'),
  411 |           jwt: window.localStorage.getItem('jwt'),
  412 |         };
  413 |       });
  414 | 
  415 |       expect(localStorage.token).toBeNull();
```