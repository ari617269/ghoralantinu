# Backend E2E Test Cases

## Setup
- PostgreSQL running with database `ghoralantinu`
- Migrations applied (`npm run migrate`)
- Seed data loaded (`npm run seed`)
- Backend running on `http://localhost:3000`

---

## Test Suite 1: Authentication Routes

### TC1.1: POST /api/user/login - Valid Credentials
**Precondition**: Test user exists (testuser / password123)

**Steps**:
1. POST to `http://localhost:3000/api/user/login`
2. Body: `{ "username": "testuser", "password": "password123" }`

**Expected**:
- Status: `200`
- Response: `{ "token": "<JWT>", "user": { "id": 1, "username": "testuser" } }`
- JWT is valid for 1 hour
- JWT payload contains: `{ id: 1, username: "testuser" }`

**Verify**:
```bash
curl -X POST http://localhost:3000/api/user/login \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"password123"}'
```

---

### TC1.2: POST /api/user/login - Invalid Username
**Steps**:
1. POST to `http://localhost:3000/api/user/login`
2. Body: `{ "username": "nonexistent", "password": "password123" }`

**Expected**:
- Status: `401`
- Response: `{ "error": "Invalid credentials" }`
- No token issued

---

### TC1.3: POST /api/user/login - Invalid Password
**Steps**:
1. POST to `http://localhost:3000/api/user/login`
2. Body: `{ "username": "testuser", "password": "wrongpassword" }`

**Expected**:
- Status: `401`
- Response: `{ "error": "Invalid credentials" }`
- No token issued

---

### TC1.4: POST /api/user/login - Missing Username
**Steps**:
1. POST to `http://localhost:3000/api/user/login`
2. Body: `{ "password": "password123" }`

**Expected**:
- Status: `400`
- Response: `{ "error": "Username and password are required" }`

---

### TC1.5: POST /api/user/login - Missing Password
**Steps**:
1. POST to `http://localhost:3000/api/user/login`
2. Body: `{ "username": "testuser" }`

**Expected**:
- Status: `400`
- Response: `{ "error": "Username and password are required" }`

---

### TC1.6: POST /api/user/login - Empty Body
**Steps**:
1. POST to `http://localhost:3000/api/user/login`
2. Body: `{}`

**Expected**:
- Status: `400`
- Response: `{ "error": "Username and password are required" }`

---

## Test Suite 2: Protected Routes - GET /api/user/valid

### TC2.1: GET /api/user/valid - Valid Token
**Precondition**: Valid JWT token from TC1.1

**Steps**:
1. GET `http://localhost:3000/api/user/valid`
2. Header: `Authorization: Bearer <valid_token>`

**Expected**:
- Status: `200`
- Response: `{ "valid": true, "user": { "id": 1, "username": "testuser" } }`

---

### TC2.2: GET /api/user/valid - Missing Authorization Header
**Steps**:
1. GET `http://localhost:3000/api/user/valid`
2. No Authorization header

**Expected**:
- Status: `401`
- Response: `{ "error": "Missing or invalid Authorization header" }`

---

### TC2.3: GET /api/user/valid - Malformed Authorization Header
**Steps**:
1. GET `http://localhost:3000/api/user/valid`
2. Header: `Authorization: Invalid <token>`

**Expected**:
- Status: `401`
- Response: `{ "error": "Missing or invalid Authorization header" }`

---

### TC2.4: GET /api/user/valid - Invalid Token
**Steps**:
1. GET `http://localhost:3000/api/user/valid`
2. Header: `Authorization: Bearer invalid.token.here`

**Expected**:
- Status: `401`
- Response: `{ "error": "Invalid or expired token" }`

---

### TC2.5: GET /api/user/valid - Expired Token
**Precondition**: Token with past expiry time

**Steps**:
1. Manually create a JWT with expiry time in the past
2. GET `http://localhost:3000/api/user/valid`
3. Header: `Authorization: Bearer <expired_token>`

**Expected**:
- Status: `401`
- Response: `{ "error": "Invalid or expired token" }`

---

### TC2.6: GET /api/user/valid - Revoked Token
**Precondition**: Token has been logged out (exists in expired_tokens table)

**Steps**:
1. Login to get token (TC1.1)
2. Logout with that token (TC3.1)
3. GET `http://localhost:3000/api/user/valid`
4. Header: `Authorization: Bearer <revoked_token>`

**Expected**:
- Status: `401`
- Response: `{ "error": "Token has been revoked" }`

---

## Test Suite 3: Protected Routes - POST /api/user/logout

### TC3.1: POST /api/user/logout - Valid Token
**Precondition**: Valid JWT token from TC1.1

**Steps**:
1. POST `http://localhost:3000/api/user/logout`
2. Header: `Authorization: Bearer <valid_token>`

**Expected**:
- Status: `200`
- Response: `{ "message": "Logged out successfully" }`
- Token is added to `expired_tokens` table
- Token has `expired_at` set to JWT expiry time

**Database Verify**:
```sql
SELECT token, expired_at FROM expired_tokens WHERE token = '<token>';
```
Should return 1 row with matching token and correct expired_at timestamp.

---

### TC3.2: POST /api/user/logout - Missing Authorization Header
**Steps**:
1. POST `http://localhost:3000/api/user/logout`
2. No Authorization header

**Expected**:
- Status: `401`
- Response: `{ "error": "Missing or invalid Authorization header" }`

---

### TC3.3: POST /api/user/logout - Invalid Token
**Steps**:
1. POST `http://localhost:3000/api/user/logout`
2. Header: `Authorization: Bearer invalid.token.here`

**Expected**:
- Status: `401`
- Response: `{ "error": "Invalid or expired token" }`

---

### TC3.4: POST /api/user/logout - Already Revoked Token
**Precondition**: Token already logged out

**Steps**:
1. Login to get token (TC1.1)
2. Logout with that token (TC3.1)
3. POST `/api/user/logout` again with same token

**Expected**:
- Status: `401`
- Response: `{ "error": "Token has been revoked" }`

---

## Test Suite 4: Session Management

### TC4.1: Multiple Users - Isolated Sessions
**Steps**:
1. Create second user in database:
   ```sql
   INSERT INTO users (username, password_hash) VALUES 
   ('user2', '<bcrypt_hash_of_password456>');
   ```
2. Login as testuser → get token1
3. Login as user2 → get token2
4. GET `/api/user/valid` with token1 → should return testuser
5. GET `/api/user/valid` with token2 → should return user2

**Expected**:
- Status: `200` for both
- Each token returns correct user data
- Tokens are independent

---

### TC4.2: Token Expiry After 1 Hour
**Steps**:
1. Login to get token
2. Wait 1 hour
3. GET `/api/user/valid` with expired token

**Expected**:
- Status: `401`
- Response: `{ "error": "Invalid or expired token" }`

---

### TC4.3: Logout Prevents Token Reuse
**Steps**:
1. Login → get token
2. Logout → token added to blocklist
3. GET `/api/user/valid` with same token

**Expected**:
- Status: `401`
- Response: `{ "error": "Token has been revoked" }`

---

## Test Suite 5: Health Check (Existing)

### TC5.1: GET /health - Server Health
**Steps**:
1. GET `http://localhost:3000/health`

**Expected**:
- Status: `200`
- Response: `{ "status": "ok" }`

---

## Test Suite 6: Database Integrity

### TC6.1: Users Table Structure
**Steps**:
```sql
\d users
```

**Expected**:
- Columns: id (PRIMARY KEY), username (UNIQUE NOT NULL), password_hash (NOT NULL), created_at (DEFAULT NOW())
- Index on username

---

### TC6.2: Expired Tokens Table Structure
**Steps**:
```sql
\d expired_tokens
```

**Expected**:
- Columns: id (PRIMARY KEY), token (TEXT NOT NULL), expired_at (TIMESTAMP NOT NULL), created_at (DEFAULT NOW())
- Index on token column

---

### TC6.3: Test User Exists
**Steps**:
```sql
SELECT * FROM users WHERE username = 'testuser';
```

**Expected**:
- Returns 1 row
- username = 'testuser'
- password_hash is bcrypt hash (starts with $2a$, $2b$, or $2y$)
- created_at is recent timestamp

---

## Test Suite 7: Error Handling

### TC7.1: 500 Error on Unhandled Exception
**Steps**:
1. Attempt to trigger unhandled error (e.g., database connection issue)

**Expected**:
- Status: `500`
- Response: `{ "error": "Internal server error" }`

---

## Test Automation Script (Bash)

```bash
#!/bin/bash

BASE_URL="http://localhost:3000"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

pass() { echo -e "${GREEN}✓ $1${NC}"; }
fail() { echo -e "${RED}✗ $1${NC}"; }

# TC1.1: Valid login
TOKEN=$(curl -s -X POST $BASE_URL/api/user/login \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"password123"}' \
  | jq -r '.token')

if [ ! -z "$TOKEN" ] && [ "$TOKEN" != "null" ]; then
  pass "TC1.1: Login with valid credentials"
else
  fail "TC1.1: Login failed"
fi

# TC2.1: Valid token check
RESPONSE=$(curl -s -X GET $BASE_URL/api/user/valid \
  -H "Authorization: Bearer $TOKEN")

if echo "$RESPONSE" | jq -e '.valid == true' > /dev/null; then
  pass "TC2.1: Valid token check"
else
  fail "TC2.1: Token validation failed"
fi

# TC3.1: Logout
LOGOUT=$(curl -s -X POST $BASE_URL/api/user/logout \
  -H "Authorization: Bearer $TOKEN" | jq -r '.message')

if [ "$LOGOUT" = "Logged out successfully" ]; then
  pass "TC3.1: Logout successful"
else
  fail "TC3.1: Logout failed"
fi

# TC3.4: Revoked token check
REVOKED=$(curl -s -X GET $BASE_URL/api/user/valid \
  -H "Authorization: Bearer $TOKEN" | jq -r '.error')

if [ "$REVOKED" = "Token has been revoked" ]; then
  pass "TC3.4: Revoked token rejected"
else
  fail "TC3.4: Revoked token not rejected"
fi

# TC1.2: Invalid username
INVALID=$(curl -s -X POST $BASE_URL/api/user/login \
  -H "Content-Type: application/json" \
  -d '{"username":"invalid","password":"password123"}' \
  | jq -r '.error')

if [ "$INVALID" = "Invalid credentials" ]; then
  pass "TC1.2: Invalid username rejected"
else
  fail "TC1.2: Invalid username not rejected"
fi

# TC2.2: Missing auth header
MISSING=$(curl -s -X GET $BASE_URL/api/user/valid | jq -r '.error')

if [ "$MISSING" = "Missing or invalid Authorization header" ]; then
  pass "TC2.2: Missing auth header rejected"
else
  fail "TC2.2: Missing auth header not rejected"
fi

echo ""
echo "Backend E2E tests completed!"
```

---

## Test Coverage Summary

| Suite | Test Cases | Focus |
|-------|-----------|-------|
| 1. Authentication | 6 | Login validation, input validation |
| 2. Token Validation | 6 | Valid/invalid/expired/revoked tokens |
| 3. Logout | 4 | Logout flows and token blocklist |
| 4. Sessions | 3 | Multi-user sessions, expiry, reuse prevention |
| 5. Health | 1 | Service health |
| 6. Database | 3 | Schema integrity |
| 7. Error Handling | 1 | Error responses |
| **Total** | **24** | **Comprehensive auth flow** |
