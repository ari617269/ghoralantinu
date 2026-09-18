# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth-logic.spec.ts >> Frontend Auth Logic E2E Tests >> In-Memory Token Storage Logic >> TC2: Token is not stored in sessionStorage
- Location: tests/e2e/auth-logic.spec.ts:420:5

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
  416 |       expect(localStorage.auth).toBeNull();
  417 |       expect(localStorage.jwt).toBeNull();
  418 |     });
  419 | 
  420 |     test('TC2: Token is not stored in sessionStorage', async ({ page }) => {
  421 |       // Login
  422 |       await page.goto(`${BASE_URL}/login`);
  423 |       await page.fill('input[id="username"]', 'testuser');
  424 |       await page.fill('input[id="password"]', 'password123');
  425 |       await page.click('button[type="submit"]');
> 426 |       await page.waitForURL(`${BASE_URL}/`);
      |                  ^ Error: page.waitForURL: Test timeout of 30000ms exceeded.
  427 | 
  428 |       // Check sessionStorage
  429 |       const sessionStorage = await page.evaluate(() => {
  430 |         return {
  431 |           token: window.sessionStorage.getItem('token'),
  432 |           auth: window.sessionStorage.getItem('auth'),
  433 |           jwt: window.sessionStorage.getItem('jwt'),
  434 |         };
  435 |       });
  436 | 
  437 |       expect(sessionStorage.token).toBeNull();
  438 |       expect(sessionStorage.auth).toBeNull();
  439 |       expect(sessionStorage.jwt).toBeNull();
  440 |     });
  441 | 
  442 |     test('TC3: Token is not in cookies', async ({ page, context }) => {
  443 |       // Login
  444 |       await page.goto(`${BASE_URL}/login`);
  445 |       await page.fill('input[id="username"]', 'testuser');
  446 |       await page.fill('input[id="password"]', 'password123');
  447 |       await page.click('button[type="submit"]');
  448 |       await page.waitForURL(`${BASE_URL}/`);
  449 | 
  450 |       // Check cookies
  451 |       const cookies = await context.cookies();
  452 |       const authCookies = cookies.filter(c => c.name.toLowerCase().includes('token') || c.name.toLowerCase().includes('auth'));
  453 | 
  454 |       expect(authCookies.length).toBe(0);
  455 |     });
  456 | 
  457 |     test('TC4: Page refresh clears token (in-memory limitation)', async ({ page }) => {
  458 |       // Login
  459 |       await page.goto(`${BASE_URL}/login`);
  460 |       await page.fill('input[id="username"]', 'testuser');
  461 |       await page.fill('input[id="password"]', 'password123');
  462 |       await page.click('button[type="submit"]');
  463 |       await page.waitForURL(`${BASE_URL}/`);
  464 | 
  465 |       // Verify on home
  466 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  467 | 
  468 |       // Refresh page
  469 |       await page.reload();
  470 | 
  471 |       // Redux state is cleared (in-memory), so user redirected to login
  472 |       await page.waitForURL(`${BASE_URL}/login`);
  473 | 
  474 |       // This is EXPECTED behavior for in-memory tokens
  475 |       await expect(page.locator('h1')).toContainText('Login');
  476 |     });
  477 | 
  478 |     test('TC5: Each tab has independent session (no cross-tab sync)', async ({ browser }) => {
  479 |       const context = await browser.newContext();
  480 |       const page1 = await context.newPage();
  481 |       const page2 = await context.newPage();
  482 | 
  483 |       // Tab 1: Login
  484 |       await page1.goto(`${BASE_URL}/login`);
  485 |       await page1.fill('input[id="username"]', 'testuser');
  486 |       await page1.fill('input[id="password"]', 'password123');
  487 |       await page1.click('button[type="submit"]');
  488 |       await page1.waitForURL(`${BASE_URL}/`);
  489 | 
  490 |       // Tab 2: Login separately
  491 |       await page2.goto(`${BASE_URL}/login`);
  492 |       await page2.fill('input[id="username"]', 'testuser');
  493 |       await page2.fill('input[id="password"]', 'password123');
  494 |       await page2.click('button[type="submit"]');
  495 |       await page2.waitForURL(`${BASE_URL}/`);
  496 | 
  497 |       // Tab 1: Logout
  498 |       await page1.click('button:has-text("Logout")');
  499 |       await page1.waitForURL(`${BASE_URL}/login`);
  500 | 
  501 |       // Tab 2: Should still be logged in (different Redux instance)
  502 |       await expect(page2.locator('text=Welcome, testuser')).toBeVisible();
  503 | 
  504 |       await context.close();
  505 |     });
  506 |   });
  507 | 
  508 |   // ========================================================================
  509 |   // STATE CONSISTENCY LOGIC TESTS
  510 |   // ========================================================================
  511 | 
  512 |   test.describe('State Consistency', () => {
  513 |     test('TC1: Username in greeting matches login username', async ({ page }) => {
  514 |       await page.goto(`${BASE_URL}/login`);
  515 |       await page.fill('input[id="username"]', 'testuser');
  516 |       await page.fill('input[id="password"]', 'password123');
  517 |       await page.click('button[type="submit"]');
  518 |       await page.waitForURL(`${BASE_URL}/`);
  519 | 
  520 |       // Verify greeting uses correct username from Redux state
  521 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  522 |     });
  523 | 
  524 |     test('TC2: Logout button only visible when authenticated', async ({ page }) => {
  525 |       // On login page - no logout button
  526 |       await page.goto(`${BASE_URL}/login`);
```