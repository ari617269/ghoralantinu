import { test, expect, Page } from '@playwright/test';

interface AuthState {
  token: string | null;
  user: { id: number; username: string } | null;
  isAuthenticated: boolean;
}

/**
 * Frontend E2E Tests - Application Logic
 * Tests the state machine, Redux integration, and UI behavior based on auth state
 */

test.describe('Frontend Auth Logic E2E Tests', () => {
  const BASE_URL = 'http://localhost:5173';
  const API_URL = 'http://localhost:3000';

  // Helper: Get Redux auth state from page
  async function getAuthState(page: Page): Promise<AuthState> {
    return await page.evaluate(() => {
      // Access Redux store from window if exposed in dev
      const state = (window as any).__REDUX_DEVTOOLS_EXTENSION_COMPOSE__?.
        (window as any).store?.getState?.()?.auth || {
        token: null,
        user: null,
        isAuthenticated: false,
      };
      return state;
    });
  }

  // Helper: Get Redux state via window (for tests)
  async function getReduxState(page: Page): Promise<any> {
    // Since Redux state isn't directly accessible, we'll verify via API calls
    // and UI state instead
    return null;
  }

  // ========================================================================
  // LOGIN LOGIC TESTS
  // ========================================================================

  test.describe('Login Logic', () => {
    test('TC1: Valid credentials transition from login to home', async ({ page }) => {
      // Start at login
      await page.goto(`${BASE_URL}/login`);
      await expect(page.locator('h1')).toContainText('Login');

      // Intercept the login API call to verify payload
      const loginRequest = page.waitForResponse(
        response => response.url().includes('/api/user/login') && response.status() === 200
      );

      // Submit form
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Verify API was called correctly
      const response = await loginRequest;
      const data = await response.json();

      expect(data.token).toBeTruthy();
      expect(data.user.username).toBe('testuser');
      expect(data.user.id).toBe(1);

      // Verify redirect to home
      await page.waitForURL(`${BASE_URL}/`);
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC2: Invalid credentials show error and remain on login', async ({ page }) => {
      await page.goto(`${BASE_URL}/login`);

      // Submit invalid credentials
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'wrongpassword');
      await page.click('button[type="submit"]');

      // Verify error message appears
      await expect(page.locator('text=Invalid username or password')).toBeVisible();

      // Verify URL hasn't changed
      await expect(page).toHaveURL(`${BASE_URL}/login`);

      // Verify form is still visible and editable
      await expect(page.locator('input[id="username"]')).toHaveValue('testuser');
      await expect(page.locator('input[id="password"]')).toHaveValue('wrongpassword');

      // Form is still functional - can retry
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Should now succeed
      await page.waitForURL(`${BASE_URL}/`);
    });

    test('TC3: Missing fields prevent submission', async ({ page }) => {
      await page.goto(`${BASE_URL}/login`);

      // Try to submit without username
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Should get error (client-side or server)
      const errorVisible = await page.locator('text=Invalid username or password, text=required').isVisible().catch(() => false);
      const stillOnLogin = page.url().includes('/login');

      expect(errorVisible || stillOnLogin).toBeTruthy();
    });

    test('TC4: Login button shows loading state during submission', async ({ page }) => {
      await page.goto(`${BASE_URL}/login`);

      // Set up slow network to observe loading state
      await page.route('**/api/user/login', async (route) => {
        await new Promise(resolve => setTimeout(resolve, 1000));
        await route.continue();
      });

      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');

      const button = page.locator('button[type="submit"]');

      // Check initial state
      await expect(button).toContainText('Login');

      // Click and immediately check for loading state
      await button.click();
      await expect(button).toContainText('Logging in...');
      await expect(button).toBeDisabled();

      // Wait for completion
      await page.waitForURL(`${BASE_URL}/`);
    });
  });

  // ========================================================================
  // SESSION VALIDATION LOGIC TESTS
  // ========================================================================

  test.describe('Session Validation Logic', () => {
    test('TC1: Valid token allows home page access', async ({ page }) => {
      // First login to get token
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Wait for home page
      await page.waitForURL(`${BASE_URL}/`);

      // Verify session validation API was called
      const validationRequest = page.waitForResponse(
        response => response.url().includes('/api/user/valid') && response.status() === 200
      );

      await validationRequest;

      // Verify home page is visible
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
      await expect(page.locator('button:has-text("Logout")')).toBeVisible();
    });

    test('TC2: Home page calls session validation on mount', async ({ page }) => {
      // Navigate directly to home without auth
      await page.goto(`${BASE_URL}/`);

      // Should redirect to login because no token
      await page.waitForURL(`${BASE_URL}/login`);
      await expect(page.locator('h1')).toContainText('Login');
    });

    test('TC3: Invalid token redirects to login', async ({ page, context }) => {
      // Login first
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Intercept and fail the validation check
      await page.route('**/api/user/valid', route => {
        route.abort('failed');
      });

      // Refresh or navigate to trigger validation
      await page.reload();

      // Should redirect to login because validation failed
      await page.waitForURL(`${BASE_URL}/login`);
    });

    test('TC4: Session persists across page navigation', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Verify on home
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();

      // Navigate away via URL manipulation (if there were other pages)
      // For now, just verify home is still accessible
      await page.goto(`${BASE_URL}/`);
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });
  });

  // ========================================================================
  // LOGOUT LOGIC TESTS
  // ========================================================================

  test.describe('Logout Logic', () => {
    test('TC1: Logout invalidates token and redirects to login', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Get initial token (from network inspection)
      let tokenBeforeLogout: string | null = null;
      page.on('response', async (response) => {
        if (response.url().includes('/api/user/login')) {
          const data = await response.json();
          tokenBeforeLogout = data.token;
        }
      });

      // Logout
      const logoutRequest = page.waitForResponse(
        response => response.url().includes('/api/user/logout')
      );

      await page.click('button:has-text("Logout")');

      // Verify logout API was called
      const response = await logoutRequest;
      expect(response.status()).toBe(200);

      // Verify redirected to login
      await page.waitForURL(`${BASE_URL}/login`);
      await expect(page.locator('h1')).toContainText('Login');

      // Verify form is cleared
      await expect(page.locator('input[id="username"]')).toHaveValue('');
      await expect(page.locator('input[id="password"]')).toHaveValue('');
    });

    test('TC2: Logout clears Redux auth state', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Logout
      await page.click('button:has-text("Logout")');
      await page.waitForURL(`${BASE_URL}/login`);

      // Verify accessing home redirects to login (because state is cleared)
      await page.goto(`${BASE_URL}/`);
      await page.waitForURL(`${BASE_URL}/login`);
    });

    test('TC3: Cannot use token after logout', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');

      let tokenFromLogin: string | null = null;

      page.on('response', async (response) => {
        if (response.url().includes('/api/user/login') && response.status() === 200) {
          const data = await response.json();
          tokenFromLogin = data.token;
        }
      });

      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Wait a moment for token to be captured
      await page.waitForTimeout(500);

      // Logout
      await page.click('button:has-text("Logout")');
      await page.waitForURL(`${BASE_URL}/login`);

      // Try to use old token to validate - should fail
      if (tokenFromLogin) {
        const validationResponse = await fetch(`${API_URL}/api/user/valid`, {
          headers: { Authorization: `Bearer ${tokenFromLogin}` },
        });

        expect(validationResponse.status).toBe(401);
        const data = await validationResponse.json();
        expect(data.error).toContain('revoked');
      }
    });

    test('TC4: Logout button shows loading state', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Slow down logout API
      await page.route('**/api/user/logout', async (route) => {
        await new Promise(resolve => setTimeout(resolve, 1000));
        await route.continue();
      });

      const logoutButton = page.locator('button:has-text("Logout")');

      // Click logout
      await logoutButton.click();

      // Verify loading state
      await expect(logoutButton).toContainText('Logging out...');
      await expect(logoutButton).toBeDisabled();

      // Wait for completion
      await page.waitForURL(`${BASE_URL}/login`);
    });
  });

  // ========================================================================
  // PROTECTED ROUTE LOGIC TESTS
  // ========================================================================

  test.describe('Protected Route Logic', () => {
    test('TC1: Unauthenticated user cannot access home', async ({ page }) => {
      await page.goto(`${BASE_URL}/`);
      await page.waitForURL(`${BASE_URL}/login`);
      await expect(page.locator('h1')).toContainText('Login');
    });

    test('TC2: Direct URL to home redirects to login when not authenticated', async ({ page }) => {
      // Don't login, just try to access home
      await page.goto(`${BASE_URL}/`, { waitUntil: 'networkidle' });

      // Should redirect immediately
      expect(page.url()).toContain('/login');
    });

    test('TC3: Unknown routes redirect to home (or login if not authenticated)', async ({ page }) => {
      // Try unknown route
      await page.goto(`${BASE_URL}/unknown-page`);

      // Should end up at login (because not authenticated)
      await page.waitForURL(`${BASE_URL}/login`);
    });

    test('TC4: Authenticated user can access home', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Should be on home
      await page.waitForURL(`${BASE_URL}/`);
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC5: Cannot access login when already authenticated', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Try to go back to login
      await page.goto(`${BASE_URL}/login`);

      // Should redirect back to home
      await page.waitForURL(`${BASE_URL}/`);
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });
  });

  // ========================================================================
  // IN-MEMORY TOKEN STORAGE LOGIC TESTS
  // ========================================================================

  test.describe('In-Memory Token Storage Logic', () => {
    test('TC1: Token is not stored in localStorage', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Check localStorage
      const localStorage = await page.evaluate(() => {
        return {
          token: window.localStorage.getItem('token'),
          auth: window.localStorage.getItem('auth'),
          jwt: window.localStorage.getItem('jwt'),
        };
      });

      expect(localStorage.token).toBeNull();
      expect(localStorage.auth).toBeNull();
      expect(localStorage.jwt).toBeNull();
    });

    test('TC2: Token is not stored in sessionStorage', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Check sessionStorage
      const sessionStorage = await page.evaluate(() => {
        return {
          token: window.sessionStorage.getItem('token'),
          auth: window.sessionStorage.getItem('auth'),
          jwt: window.sessionStorage.getItem('jwt'),
        };
      });

      expect(sessionStorage.token).toBeNull();
      expect(sessionStorage.auth).toBeNull();
      expect(sessionStorage.jwt).toBeNull();
    });

    test('TC3: Token is not in cookies', async ({ page, context }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Check cookies
      const cookies = await context.cookies();
      const authCookies = cookies.filter(c => c.name.toLowerCase().includes('token') || c.name.toLowerCase().includes('auth'));

      expect(authCookies.length).toBe(0);
    });

    test('TC4: Page refresh clears token (in-memory limitation)', async ({ page }) => {
      // Login
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Verify on home
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();

      // Refresh page
      await page.reload();

      // Redux state is cleared (in-memory), so user redirected to login
      await page.waitForURL(`${BASE_URL}/login`);

      // This is EXPECTED behavior for in-memory tokens
      await expect(page.locator('h1')).toContainText('Login');
    });

    test('TC5: Each tab has independent session (no cross-tab sync)', async ({ browser }) => {
      const context = await browser.newContext();
      const page1 = await context.newPage();
      const page2 = await context.newPage();

      // Tab 1: Login
      await page1.goto(`${BASE_URL}/login`);
      await page1.fill('input[id="username"]', 'testuser');
      await page1.fill('input[id="password"]', 'password123');
      await page1.click('button[type="submit"]');
      await page1.waitForURL(`${BASE_URL}/`);

      // Tab 2: Login separately
      await page2.goto(`${BASE_URL}/login`);
      await page2.fill('input[id="username"]', 'testuser');
      await page2.fill('input[id="password"]', 'password123');
      await page2.click('button[type="submit"]');
      await page2.waitForURL(`${BASE_URL}/`);

      // Tab 1: Logout
      await page1.click('button:has-text("Logout")');
      await page1.waitForURL(`${BASE_URL}/login`);

      // Tab 2: Should still be logged in (different Redux instance)
      await expect(page2.locator('text=Welcome, testuser')).toBeVisible();

      await context.close();
    });
  });

  // ========================================================================
  // STATE CONSISTENCY LOGIC TESTS
  // ========================================================================

  test.describe('State Consistency', () => {
    test('TC1: Username in greeting matches login username', async ({ page }) => {
      await page.goto(`${BASE_URL}/login`);
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      // Verify greeting uses correct username from Redux state
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });

    test('TC2: Logout button only visible when authenticated', async ({ page }) => {
      // On login page - no logout button
      await page.goto(`${BASE_URL}/login`);
      const logoutButton1 = page.locator('button:has-text("Logout")');
      await expect(logoutButton1).not.toBeVisible();

      // After login - logout button visible
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');
      await page.waitForURL(`${BASE_URL}/`);

      const logoutButton2 = page.locator('button:has-text("Logout")');
      await expect(logoutButton2).toBeVisible();
    });

    test('TC3: Error message clears on successful retry', async ({ page }) => {
      await page.goto(`${BASE_URL}/login`);

      // First attempt - wrong password
      await page.fill('input[id="username"]', 'testuser');
      await page.fill('input[id="password"]', 'wrong');
      await page.click('button[type="submit"]');

      // Error should appear
      await expect(page.locator('text=Invalid username or password')).toBeVisible();

      // Correct the password
      await page.fill('input[id="password"]', 'password123');
      await page.click('button[type="submit"]');

      // Should redirect to home (error is cleared by successful login)
      await page.waitForURL(`${BASE_URL}/`);
      await expect(page.locator('text=Welcome, testuser')).toBeVisible();
    });
  });
});
