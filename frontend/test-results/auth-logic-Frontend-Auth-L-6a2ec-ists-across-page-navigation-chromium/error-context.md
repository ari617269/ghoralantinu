# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: auth-logic.spec.ts >> Frontend Auth Logic E2E Tests >> Session Validation Logic >> TC4: Session persists across page navigation
- Location: tests/e2e/auth-logic.spec.ts:195:5

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
  151 |       // Wait for home page
  152 |       await page.waitForURL(`${BASE_URL}/`);
  153 | 
  154 |       // Verify session validation API was called
  155 |       const validationRequest = page.waitForResponse(
  156 |         response => response.url().includes('/api/user/valid') && response.status() === 200
  157 |       );
  158 | 
  159 |       await validationRequest;
  160 | 
  161 |       // Verify home page is visible
  162 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  163 |       await expect(page.locator('button:has-text("Logout")')).toBeVisible();
  164 |     });
  165 | 
  166 |     test('TC2: Home page calls session validation on mount', async ({ page }) => {
  167 |       // Navigate directly to home without auth
  168 |       await page.goto(`${BASE_URL}/`);
  169 | 
  170 |       // Should redirect to login because no token
  171 |       await page.waitForURL(`${BASE_URL}/login`);
  172 |       await expect(page.locator('h1')).toContainText('Login');
  173 |     });
  174 | 
  175 |     test('TC3: Invalid token redirects to login', async ({ page, context }) => {
  176 |       // Login first
  177 |       await page.goto(`${BASE_URL}/login`);
  178 |       await page.fill('input[id="username"]', 'testuser');
  179 |       await page.fill('input[id="password"]', 'password123');
  180 |       await page.click('button[type="submit"]');
  181 |       await page.waitForURL(`${BASE_URL}/`);
  182 | 
  183 |       // Intercept and fail the validation check
  184 |       await page.route('**/api/user/valid', route => {
  185 |         route.abort('failed');
  186 |       });
  187 | 
  188 |       // Refresh or navigate to trigger validation
  189 |       await page.reload();
  190 | 
  191 |       // Should redirect to login because validation failed
  192 |       await page.waitForURL(`${BASE_URL}/login`);
  193 |     });
  194 | 
  195 |     test('TC4: Session persists across page navigation', async ({ page }) => {
  196 |       // Login
  197 |       await page.goto(`${BASE_URL}/login`);
  198 |       await page.fill('input[id="username"]', 'testuser');
  199 |       await page.fill('input[id="password"]', 'password123');
  200 |       await page.click('button[type="submit"]');
> 201 |       await page.waitForURL(`${BASE_URL}/`);
      |                  ^ Error: page.waitForURL: Test timeout of 30000ms exceeded.
  202 | 
  203 |       // Verify on home
  204 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  205 | 
  206 |       // Navigate away via URL manipulation (if there were other pages)
  207 |       // For now, just verify home is still accessible
  208 |       await page.goto(`${BASE_URL}/`);
  209 |       await expect(page.locator('text=Welcome, testuser')).toBeVisible();
  210 |     });
  211 |   });
  212 | 
  213 |   // ========================================================================
  214 |   // LOGOUT LOGIC TESTS
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
```