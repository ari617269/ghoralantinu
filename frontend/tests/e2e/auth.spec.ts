import { test, expect } from '@playwright/test';

/**
 * Frontend E2E Tests - Testing Application Logic with Playwright
 * These tests verify the actual state machine, Redux integration, and auth flow
 */

test.describe('Frontend Auth Logic E2E Tests', () => {
  test.describe('Login Logic - Form Submission & State Update', () => {
    test('TC1.1: Valid credentials trigger login API and update Redux state', async ({
      page,
    }) => {
      // Navigate to login
      await page.goto('/login');
      await expect(page.locator('h1')).toContainText('Login');

      // Intercept login API
      const loginPromise = page.waitForResponse(
        (response) =>
          response.url().includes('/api/user/login') && response.status() === 200
      );

      // Submit form
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Verify API response
      const response = await loginPromise;
      const data = await response.json();

      expect(data.token).toBeTruthy();
      expect(data.user.username).toBe('testuser');

      // Verify redirect to home (implies Redux state was updated)
      await page.waitForURL('**/');
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC1.2: Invalid credentials show error without redirecting', async ({
      page,
    }) => {
      await page.goto('/login');

      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'wrongpassword');
      await page.click('button[type="submit"]');

      // Error message should appear
      await expect(page.locator('text=Invalid username or password')).toBeVisible();

      // Should remain on login page (not redirected)
      expect(page.url()).toContain('/login');

      // Form should still be visible and functional
      await expect(page.locator('input[id="username"]')).toBeVisible();
    });

    test('TC1.3: Loading state shown during submission', async ({ page }) => {
      await page.goto('/login');

      // Slow down the API
      await page.route('**/api/user/login', async (route) => {
        await new Promise((resolve) => setTimeout(resolve, 1000));
        await route.continue();
      });

      const button = page.locator('button[type="submit"]');

      // Verify initial state
      await expect(button).toContainText('Login');

      // Submit form
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await button.click();

      // Check loading state immediately
      await expect(button).toContainText('Logging in...');
      await expect(button).toBeDisabled();

      // Wait for completion
      await page.waitForURL('**/');
    });

    test('TC1.4: Session validation called on home mount', async ({ page }) => {
      await page.goto('/login');

      // Intercept session validation
      const validationPromise = page.waitForResponse(
        (response) =>
          response.url().includes('/api/user/valid') && response.status() === 200
      );

      // Login
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Session validation should be called
      await validationPromise;

      // Should be on home page
      await page.waitForURL('**/');
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });
  });

  test.describe('Protected Route Logic - Access Control', () => {
    test('TC2.1: Unauthenticated user cannot access home', async ({ page }) => {
      // Try to access home without auth
      await page.goto('/', { waitUntil: 'networkidle' });

      // Should redirect to login
      await page.waitForURL('**/login');
      await expect(page.locator('h1')).toContainText('Login');
    });

    test('TC2.2: Authenticated user can access home', async ({ page }) => {
      // First login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Should be on home
      await page.waitForURL('**/');
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC2.3: Cannot access login when already authenticated', async ({ page }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Try to navigate to login
      await page.goto('/login');

      // Should redirect back to home
      await page.waitForURL('**/');
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC2.4: Unknown routes redirect appropriately based on auth', async ({
      page,
    }) => {
      // Try unknown route without auth
      await page.goto('/unknown-page', { waitUntil: 'networkidle' });

      // Should redirect to login
      await page.waitForURL('**/login');
    });
  });

  test.describe('Logout Logic - Token Invalidation & State Clearing', () => {
    test('TC3.1: Logout calls API and clears Redux state', async ({ page }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Intercept logout API
      const logoutPromise = page.waitForResponse(
        (response) =>
          response.url().includes('/api/user/logout') && response.status() === 200
      );

      // Click logout
      await page.click('button:has-text("Logout")');

      // Verify API was called
      const response = await logoutPromise;
      expect(response.status()).toBe(200);

      // Verify redirect to login (implies Redux state was cleared)
      await page.waitForURL('**/login');
      await expect(page.locator('h1')).toContainText('Login');
    });

    test('TC3.2: Logout shows loading state', async ({ page }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Slow down logout API
      await page.route('**/api/user/logout', async (route) => {
        await new Promise((resolve) => setTimeout(resolve, 1000));
        await route.continue();
      });

      const logoutButton = page.locator('button:has-text("Logout")');

      // Click logout
      await logoutButton.click();

      // Check loading state
      await expect(logoutButton).toContainText('Logging out...');
      await expect(logoutButton).toBeDisabled();

      // Wait for completion
      await page.waitForURL('**/login');
    });

    test('TC3.3: Cannot use token after logout', async ({ page }) => {
      // Login and capture token
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');

      let tokenFromLogin: string | null = null;

      // Capture token from login response
      page.on('response', async (response) => {
        if (response.url().includes('/api/user/login') && response.status() === 200) {
          const data = await response.json();
          tokenFromLogin = data.token;
        }
      });

      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Wait for token to be captured
      await page.waitForTimeout(500);

      // Logout
      await page.click('button:has-text("Logout")');
      await page.waitForURL('**/login');

      // Try to use old token via direct API call
      if (tokenFromLogin) {
        const response = await page.context().request.get('/api/user/valid', {
          headers: { Authorization: `Bearer ${tokenFromLogin}` },
        });

        // Should be rejected
        expect(response.status()).toBe(401);
      }
    });
  });

  test.describe('In-Memory Token Storage Logic', () => {
    test('TC4.1: Token is not in localStorage', async ({ page }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Check localStorage
      const storage = await page.evaluate(() => ({
        token: window.localStorage.getItem('token'),
        auth: window.localStorage.getItem('auth'),
      }));

      expect(storage.token).toBeNull();
      expect(storage.auth).toBeNull();
    });

    test('TC4.2: Token is not in sessionStorage', async ({ page }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Check sessionStorage
      const storage = await page.evaluate(() => ({
        token: window.sessionStorage.getItem('token'),
        auth: window.sessionStorage.getItem('auth'),
      }));

      expect(storage.token).toBeNull();
      expect(storage.auth).toBeNull();
    });

    test('TC4.3: Refresh clears token and redirects to login', async ({
      page,
    }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Verify on home
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();

      // Refresh
      await page.reload();

      // Redux state cleared (in-memory), so redirected to login
      await page.waitForURL('**/login');
      await expect(page.locator('h1')).toContainText('Login');
    });
  });

  test.describe('State Consistency', () => {
    test('TC5.1: Greeting shows logged-in username', async ({ page }) => {
      // Login
      await page.goto('/login');
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      // Verify greeting uses Redux state
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC5.2: Logout button only visible when authenticated', async ({
      page,
    }) => {
      // On login - no logout button
      await page.goto('/login');
      const logoutBtn1 = page.locator('button:has-text("Logout")');
      await expect(logoutBtn1).not.toBeVisible();

      // After login - logout button visible
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL('**/');

      const logoutBtn2 = page.locator('button:has-text("Logout")');
      await expect(logoutBtn2).toBeVisible();
    });

    test('TC5.3: Error message clears on successful retry', async ({
      page,
    }) => {
      await page.goto('/login');

      // Wrong password
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'wrong');
      await page.click('button[type="submit"]');

      // Error appears
      await expect(page.locator('text=Invalid username or password')).toBeVisible();

      // Correct and retry
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Should redirect to home (error cleared)
      await page.waitForURL('**/');
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });
  });

  test.describe('Greyscale Theme', () => {
    test('TC6.1: Login page uses greyscale colors only', async ({ page }) => {
      await page.goto('/login');

      // Check body background
      const bodyColor = await page.evaluate(() => {
        return window.getComputedStyle(document.body).backgroundColor;
      });

      // Should be white/light grey (greyscale only)
      expect(bodyColor).toMatch(/white|rgb\(2[4-5][0-9], 2[4-5][0-9], 2[4-5][0-9]\)|rgb\(255/);
    });

    test('TC6.2: Responsive design works', async ({ page }) => {
      // Mobile
      await page.setViewportSize({ width: 375, height: 667 });
      await page.goto('/login');

      const form = page.locator('form, div').first();
      const box = await form.boundingBox();

      expect(box).toBeTruthy();
      expect(box!.width).toBeGreaterThan(0);
      expect(box!.width).toBeLessThan(375); // Should fit mobile

      // Desktop
      await page.setViewportSize({ width: 1920, height: 1080 });
      await page.goto('/login');

      const formDesktop = page.locator('form, div').first();
      const boxDesktop = await formDesktop.boundingBox();

      expect(boxDesktop).toBeTruthy();
      // Form should be centered, not full width
      expect(boxDesktop!.width).toBeLessThan(500);
    });
  });
});
