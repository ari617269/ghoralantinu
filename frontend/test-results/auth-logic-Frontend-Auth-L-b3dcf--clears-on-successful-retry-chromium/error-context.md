# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth-logic.spec.ts >> Frontend Auth Logic E2E Tests >> State Consistency >> TC3: Error message clears on successful retry
- Location: tests/e2e/auth-logic.spec.ts:540:5

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
  527 |       const logoutButton1 = page.locator('button:has-text("Logout")');
  528 |       await expect(logoutButton1).not.toBeVisible();
  529 | 
  530 |       // After login - logout button visible
  531 |       await page.fill('input[id="username"]', 'testuser');
  532 |       await page.fill('input[id="password"]', 'password123');
  533 |       await page.click('button[type="submit"]');
  534 |       await page.waitForURL(`${BASE_URL}/`);
  535 | 
  536 |       const logoutButton2 = page.locator('button:has-text("Logout")');
  537 |       await expect(logoutButton2).toBeVisible();
  538 |     });
  539 | 
  540 |     test('TC3: Error message clears on successful retry', async ({ page }) => {
  541 |       await page.goto(`${BASE_URL}/login`);
  542 | 
  543 |       // First attempt - wrong password
  544 |       await page.fill('input[id="username"]', 'testuser');
  545 |       await page.fill('input[id="password"]', 'wrong');
  546 |       await page.click('button[type="submit"]');
  547 | 
  548 |       // Error should appear
  549 |       await expect(page.locator('text=Invalid username or password')).toBeVisible();
  550 | 
  551 |       // Correct the password
  552 |       await page.fill('input[id="password"]', 'password123');
  553 |       await page.click('button[type="submit"]');
  554 | 
  555 |       // Should redirect to home (error is cleared by successful login)
> 556 |       await page.waitForURL(`${BASE_URL}/`);
      |                  ^ Error: page.waitForURL: Test timeout of 30000ms exceeded.
  557 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  558 |     });
  559 |   });
  560 | });
  561 | 
```