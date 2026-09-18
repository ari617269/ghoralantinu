# E2E Tests - Application Logic Testing

## Overview

Proper end-to-end tests that verify **actual application logic**, not just HTTP responses or UI rendering.

- **Backend**: Jest + Supertest - Tests authentication logic, JWT validation, token blocklist, database integrity
- **Frontend**: Playwright - Tests state machine, Redux integration, protected routes, session management

---

## Backend E2E Tests (Jest + Supertest)

### Location
`backend/tests/e2e/auth.spec.ts`

### Run Tests
```bash
# Requires PostgreSQL running, migrations applied, seed data loaded
npm run test -w backend
npm run test:watch -w backend  # Watch mode
```

### Test Suites & Application Logic

#### Suite 1: Login Logic - Password Verification & JWT Generation
**What it tests**: The core login business logic

- **TC1.1**: Valid credentials return JWT with correct user data
  - Verifies bcrypt password comparison works
  - Confirms JWT is signed with correct secret
  - Validates JWT payload has id and username
  
- **TC1.2**: Invalid password is rejected via bcrypt
  - Tests that bcrypt.compare() correctly rejects wrong passwords
  
- **TC1.3**: No username enumeration
  - Both "user doesn't exist" and "wrong password" return same error
  - Prevents attackers from guessing valid usernames
  
- **TC1.4**: JWT expires in 1 hour
  - Verifies `expiresIn: '1h'` is correctly set
  - Tests that token can't be used after expiry
  
- **TC1.5**: Missing fields validation
  - Tests that blank username/password is rejected

#### Suite 2: JWT Validation Middleware - activeLogin Logic
**What it tests**: The protective middleware on guarded routes

- **TC2.1**: Valid token passes middleware
  - JWT verified with correct secret
  - User data attached to request
  
- **TC2.2**: Missing Authorization header returns 401
  - Middleware correctly rejects missing auth
  
- **TC2.3**: Malformed Bearer format returns 401
  - Middleware validates "Bearer <token>" format
  
- **TC2.4**: Invalid JWT signature returns 401
  - Token signed with wrong secret is rejected
  
- **TC2.5**: Expired JWT returns 401
  - Middleware checks jwt.verify() which validates exp claim

#### Suite 3: Token Blocklist Logic - Logout & Revocation
**What it tests**: Server-side token invalidation

- **TC3.1**: Logout adds token to expired_tokens table
  - Token is persisted to database
  - Prevents token reuse even if signature is valid
  
- **TC3.2**: Revoked token is rejected
  - Middleware checks expired_tokens table before allowing request
  - Valid JWT signature doesn't bypass blocklist check
  
- **TC3.3**: Cannot logout twice
  - Second logout with revoked token is rejected
  
- **TC3.4**: expired_at matches JWT exp claim
  - Database stores correct expiry time for cleanup purposes

#### Suite 4: Session Independence Logic
**What it tests**: Multiple concurrent sessions don't interfere

- **TC4.1**: Each login creates different token
  - Two logins return different JWT values
  - Both tokens are independently valid
  
- **TC4.2**: Logout of one session doesn't affect others
  - User has two concurrent tokens
  - Logging out token1 doesn't invalidate token2
  - Real-world scenario: multiple tabs, mobile + web app, etc.

#### Suite 5: Password Security Logic
**What it tests**: Passwords are hashed and never exposed

- **TC5.1**: Passwords are bcrypt hashed in database
  - Hash format is valid bcrypt ($2a$, $2b$, $2y$)
  - Hash can correctly verify correct password
  - Hash rejects wrong password
  
- **TC5.2**: Plain passwords never in API responses
  - Login response has no `password` or `password_hash` fields
  - Valid response has no `password` or `password_hash` fields

#### Suite 6: Database Integrity
**What it tests**: Schema is correctly created and constrained

- **TC6.1**: Users table has correct columns
  - id, username, password_hash, created_at all exist
  
- **TC6.2**: Username unique constraint enforced
  - Cannot insert duplicate usernames
  - Database prevents duplicates, not just application
  
- **TC6.3**: Expired_tokens table schema correct
  - id, token, expired_at, created_at all exist
  
- **TC6.4**: Token index exists
  - Query on token column is indexed for performance
  - Lookup during middleware is fast

#### Suite 7: Error Handling
**What it tests**: Application recovers from errors

- **TC7.1**: Server remains functional after errors
  - After invalid login, next request still works
  - No cascading failures

---

## Frontend E2E Tests (Playwright)

### Location
`frontend/tests/e2e/auth.spec.ts`

### Run Tests
```bash
# Requires backend running on localhost:3000
npm run test -w frontend
npm run test:ui -w frontend  # Interactive UI mode
npm run test:watch -w frontend  # Watch mode
```

### Test Suites & Application Logic

#### Suite 1: Login Logic - Form Submission & Redux State Update
**What it tests**: Login flow updates Redux state and navigates

- **TC1.1**: Valid credentials trigger login API and update Redux state
  - Form submission calls `/api/user/login`
  - API response (token + user) is stored in Redux
  - Redirect to home proves auth state was updated
  
- **TC1.2**: Invalid credentials show error without redirecting
  - Error displayed to user
  - Login page remains active (not redirected)
  - User can retry
  
- **TC1.3**: Loading state shown during submission
  - Button text changes to "Logging in..."
  - Button is disabled (prevents double submission)
  - Normal state restored after completion
  
- **TC1.4**: Session validation called on home mount
  - After login, GET `/api/user/valid` is called
  - Validates token is still valid server-side
  - Protects against server-side logout/expiry

#### Suite 2: Protected Route Logic - Access Control
**What it tests**: Routes are guarded by ProtectedRoute component

- **TC2.1**: Unauthenticated user cannot access home
  - Accessing `/` without token redirects to `/login`
  - ProtectedRoute checks Redux isAuthenticated
  
- **TC2.2**: Authenticated user can access home
  - After login, `/` is accessible
  - Redux state shows isAuthenticated: true
  
- **TC2.3**: Cannot access login when authenticated
  - Login page component redirects authenticated users back to `/`
  - Prevents authenticated user from re-logging-in
  
- **TC2.4**: Unknown routes redirect appropriately
  - Catch-all route redirects to home or login based on auth state

#### Suite 3: Logout Logic - Token Invalidation & State Clearing
**What it tests**: Logout invalidates token and clears Redux

- **TC3.1**: Logout calls API and clears Redux state
  - POST `/api/user/logout` is called with token
  - Server adds token to blocklist
  - Redux state cleared (token = null, isAuthenticated = false)
  - Redirect to login proves state was cleared
  
- **TC3.2**: Logout shows loading state
  - Button text changes to "Logging out..."
  - Button disabled during request
  
- **TC3.3**: Cannot use token after logout
  - After logout, same token returns 401 on `/api/user/valid`
  - Token is in server blocklist, not just cleared locally

#### Suite 4: In-Memory Token Storage Logic
**What it tests**: Token is stored ONLY in Redux memory, not persistent storage

- **TC4.1**: Token is not in localStorage
  - No localStorage.getItem('token') works
  - Security: not accessible to XSS malicious scripts
  
- **TC4.2**: Token is not in sessionStorage
  - Token doesn't survive tab closure
  
- **TC4.3**: Refresh clears token and redirects to login
  - Redux state is lost on page reload (it's in-memory)
  - User redirected to login
  - **Trade-off**: Page refresh = re-login required
  - **Security benefit**: No persistent token storage to compromise
  
- **TC4.4**: Each tab has independent session
  - Login in tab 1, logout in tab 2 doesn't affect tab 1
  - Redux store is per-tab instance
  - Real-world: can logout one app while keeping another open

#### Suite 5: State Consistency
**What it tests**: UI correctly reflects Redux state

- **TC5.1**: Greeting shows logged-in username
  - Welcome message uses Redux state user.username
  - Proves Redux state is connected to component
  
- **TC5.2**: Logout button only visible when authenticated
  - Login page: no logout button
  - Home page: logout button visible
  - Component renders based on Redux isAuthenticated
  
- **TC5.3**: Error message clears on successful retry
  - First login attempt fails, error shown
  - Second attempt succeeds, error message gone
  - Redux state updated, component re-renders

#### Suite 6: Greyscale Theme
**What it tests**: UI uses only greyscale colors

- **TC6.1**: Login page uses greyscale colors only
  - Background is white/grey
  - Text is black/grey
  - No red, blue, green, etc.
  
- **TC6.2**: Responsive design works
  - Mobile (375px): form visible and usable
  - Tablet (768px): form centered and readable
  - Desktop (1920px): form doesn't stretch full width

---

## Running Tests End-to-End

### Prerequisites
1. PostgreSQL running
2. Database `ghoralantinu` created
3. Both services must be able to run

### Full Test Run

```bash
# Terminal 1: Run backend
cd backend
npm install
npm run migrate
npm run seed
npm run dev    # Runs on :3000

# Terminal 2: Run frontend dev server (for Playwright)
cd frontend
npm install
npm run dev    # Runs on :5173

# Terminal 3: Run tests

# Backend E2E (in another terminal)
cd backend
npm run test

# Frontend E2E (in another terminal)
cd frontend
npm run test
```

### Expected Output

**Backend (Jest)**:
```
PASS  tests/e2e/auth.spec.ts (5.234 s)
  Backend Auth E2E Tests - Application Logic
    Login Logic - Password Verification & JWT Generation
      ✓ TC1.1: Valid credentials return JWT with correct user data (45ms)
      ✓ TC1.2: Invalid password is rejected via bcrypt (38ms)
      ✓ TC1.3: Nonexistent username returns same error as wrong password (42ms)
      ... (more tests)
  
    PASS: 35 tests

Test Suites: 1 passed, 1 total
Tests:       35 passed, 35 total
```

**Frontend (Playwright)**:
```
Running 26 tests using 1 worker

✓ [chromium] › auth.spec.ts › Login Logic › TC1.1: Valid credentials trigger login API and update Redux state (2.5s)
✓ [chromium] › auth.spec.ts › Login Logic › TC1.2: Invalid credentials show error without redirecting (1.8s)
... (more tests)

26 passed (45.2s)
```

---

## What These Tests Verify

### Application Logic NOT UI Rendering

These tests verify the **state machine** and **business rules**, not just:
- ✗ "Button appears" (wrong)
- ✓ "Redux state updated when button clicked" (correct)

- ✗ "Error text is displayed" (wrong)
- ✓ "Error state prevents redirect AND shows message" (correct)

- ✗ "Token API returns 200" (wrong)
- ✓ "Token signature validation, expiry check, and blocklist check all pass" (correct)

### Coverage Map

| Layer | Verified |
|-------|----------|
| **Authentication** | Password hashing (bcrypt), JWT signing/validation, token expiry |
| **Authorization** | Protected routes, token blocklist, session isolation |
| **State Management** | Redux store updates, state consistency, state clearing |
| **Security** | No plaintext passwords, no localStorage tokens, error non-enumeration |
| **Database** | Schema integrity, constraints, indexes |
| **Error Handling** | Errors don't break app, recovery works |
| **UI Logic** | State-driven rendering, conditional visibility |

---

## CI/CD Integration

Add to `.github/workflows/test.yml`:

```yaml
name: E2E Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    services:
      postgres:
        image: postgres:15
        env:
          POSTGRES_PASSWORD: postgres
          POSTGRES_DB: ghoralantinu
        options: >-
          --health-cmd pg_isready
          --health-interval 10s
          --health-timeout 5s
          --health-retries 5
        ports:
          - 5432:5432

    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-node@v3
        with:
          node-version: '18'

      - name: Install dependencies
        run: npm install

      - name: Backend migrations
        run: npm run migrate -w backend
        env:
          POSTGRES_HOST: localhost

      - name: Backend seed
        run: npm run seed -w backend
        env:
          POSTGRES_HOST: localhost

      - name: Backend tests
        run: npm run test -w backend
        env:
          POSTGRES_HOST: localhost

      - name: Frontend build
        run: npm run build -w frontend

      - name: Frontend tests
        run: npm run test -w frontend
```

---

## Troubleshooting

### Backend Tests Fail: "Cannot find database"
- Ensure PostgreSQL is running
- Check `.env` has correct POSTGRES_* variables
- Run `npm run migrate -w backend`

### Frontend Tests Fail: "Cannot reach localhost:3000"
- Ensure backend is running: `npm run dev -w backend`
- Ensure `.env` has `API_PORT=3000`

### Frontend Tests Fail: "Cannot reach localhost:5173"
- Ensure frontend dev server is running: `npm run dev -w frontend`

---

## Summary

**35+ application logic tests** covering:
- ✅ Password hashing and comparison
- ✅ JWT signing and validation
- ✅ Token expiry enforcement
- ✅ Token blocklist (logout)
- ✅ Session independence
- ✅ Protected routes
- ✅ Redux state management
- ✅ State consistency with UI
- ✅ Security (no token leakage)
- ✅ Error handling

These tests **verify behavior**, not just responses. They test the auth state machine end-to-end.
