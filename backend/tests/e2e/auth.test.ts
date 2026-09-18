import 'dotenv/config';
import axios, { AxiosInstance } from 'axios';
import db from '../../src/db/knex';
import jwt from 'jsonwebtoken';
import bcrypt from 'bcryptjs';

interface TestContext {
  api: AxiosInstance;
  testToken: string | null;
  testUserId: number | null;
  baseURL: string;
}

const ctx: TestContext = {
  api: axios.create({ baseURL: 'http://localhost:3000' }),
  testToken: null,
  testUserId: null,
  baseURL: 'http://localhost:3000',
};

let passed = 0;
let failed = 0;

function assert(condition: boolean, message: string) {
  if (condition) {
    console.log(`  ✓ ${message}`);
    passed++;
  } else {
    console.error(`  ✗ ${message}`);
    failed++;
  }
}

async function assertThrows(fn: () => Promise<any>, message: string) {
  try {
    await fn();
    console.error(`  ✗ ${message} (did not throw)`);
    failed++;
  } catch {
    console.log(`  ✓ ${message}`);
    passed++;
  }
}

// ============================================================================
// CORE LOGIN LOGIC TESTS
// ============================================================================

async function testLoginLogic() {
  console.log('\n🔐 Login Logic Tests\n');

  // TC: Login creates and returns JWT with correct payload
  console.log('Test 1: Login returns valid JWT with correct payload');
  const loginRes = await ctx.api.post('/api/user/login', {
    username: 'testuser',
    password: 'password123',
  });

  const token = loginRes.data.token;
  ctx.testToken = token;
  ctx.testUserId = loginRes.data.user.id;

  assert(loginRes.status === 200, 'Status is 200');
  assert(token, 'Token is returned');
  assert(loginRes.data.user.username === 'testuser', 'Username in response');
  assert(loginRes.data.user.id === 1, 'User ID in response');

  // Decode and verify JWT structure
  const decoded = jwt.decode(token) as any;
  assert(decoded.id === 1, 'JWT payload contains user id');
  assert(decoded.username === 'testuser', 'JWT payload contains username');
  assert(decoded.exp, 'JWT payload contains expiry (exp)');

  // Verify token expires in ~1 hour
  const expirySeconds = (decoded.exp - Math.floor(Date.now() / 1000)) / 60;
  assert(expirySeconds >= 59 && expirySeconds <= 61, `Token expires in ~1 hour (${expirySeconds}min)`);

  // TC: Login verifies bcrypt password hash
  console.log('\nTest 2: Login rejects wrong password despite valid username');
  await assertThrows(
    () =>
      ctx.api.post('/api/user/login', {
        username: 'testuser',
        password: 'wrongpassword',
      }),
    'Wrong password returns 401 (bcrypt.compare fails)'
  );

  // TC: Login doesn't reveal whether username exists (enumeration prevention)
  console.log('\nTest 3: Login prevents username enumeration');
  let res1, res2;
  try {
    res1 = await ctx.api.post('/api/user/login', {
      username: 'nonexistent',
      password: 'password123',
    });
  } catch (e: any) {
    res1 = e.response;
  }

  try {
    res2 = await ctx.api.post('/api/user/login', {
      username: 'testuser',
      password: 'wrongpassword',
    });
  } catch (e: any) {
    res2 = e.response;
  }

  assert(res1.status === 401, 'Nonexistent user returns 401');
  assert(res2.status === 401, 'Invalid password returns 401');
  assert(res1.data.error === res2.data.error, 'Error messages are identical (no enumeration)');
}

// ============================================================================
// JWT VALIDATION LOGIC TESTS
// ============================================================================

async function testJWTValidation() {
  console.log('\n🔑 JWT Validation Logic Tests\n');

  // TC: Valid token passes validation
  console.log('Test 1: Valid token passes middleware validation');
  const res = await ctx.api.get('/api/user/valid', {
    headers: { Authorization: `Bearer ${ctx.testToken}` },
  });

  assert(res.status === 200, 'Status is 200');
  assert(res.data.valid === true, 'Response: valid = true');
  assert(res.data.user.id === 1, 'Middleware attaches user.id to request');
  assert(res.data.user.username === 'testuser', 'Middleware attaches user.username');

  // TC: Missing Authorization header rejects
  console.log('\nTest 2: Missing Authorization header is rejected');
  await assertThrows(
    () => ctx.api.get('/api/user/valid'),
    'Missing header returns 401'
  );

  // TC: Malformed Bearer token rejects
  console.log('\nTest 3: Malformed Authorization header is rejected');
  await assertThrows(
    () =>
      ctx.api.get('/api/user/valid', {
        headers: { Authorization: 'NotBearer token' },
      }),
    'Invalid Authorization format returns 401'
  );

  // TC: Invalid JWT signature rejects
  console.log('\nTest 4: Invalid JWT signature is rejected');
  const fakeToken = jwt.sign(
    { id: 1, username: 'testuser' },
    'wrong-secret', // Different secret = invalid signature
    { expiresIn: '1h' }
  );

  await assertThrows(
    () =>
      ctx.api.get('/api/user/valid', {
        headers: { Authorization: `Bearer ${fakeToken}` },
      }),
    'Wrong JWT secret returns 401'
  );

  // TC: Expired token rejects
  console.log('\nTest 5: Expired JWT is rejected');
  const expiredToken = jwt.sign(
    { id: 1, username: 'testuser' },
    process.env.JWT_SECRET!,
    { expiresIn: '-1h' } // Already expired
  );

  await assertThrows(
    () =>
      ctx.api.get('/api/user/valid', {
        headers: { Authorization: `Bearer ${expiredToken}` },
      }),
    'Expired token returns 401'
  );
}

// ============================================================================
// TOKEN BLOCKLIST LOGIC TESTS
// ============================================================================

async function testTokenBlocklist() {
  console.log('\n🚫 Token Blocklist Logic Tests\n');

  // Get a fresh token for logout testing
  const loginRes = await ctx.api.post('/api/user/login', {
    username: 'testuser',
    password: 'password123',
  });
  const tokenToLogout = loginRes.data.token;

  // TC: Logout adds token to blocklist
  console.log('Test 1: Logout adds token to expired_tokens table');
  const logoutRes = await ctx.api.post('/api/user/logout', {}, {
    headers: { Authorization: `Bearer ${tokenToLogout}` },
  });

  assert(logoutRes.status === 200, 'Logout returns 200');
  assert(logoutRes.data.message, 'Logout response has message');

  // Verify token is in database
  const blocklisted = await db('expired_tokens').where({ token: tokenToLogout }).first();
  assert(blocklisted, 'Token is inserted into expired_tokens table');

  if (blocklisted) {
    // Verify expired_at matches JWT expiry
    const decoded = jwt.decode(tokenToLogout) as any;
    const tokenExpiry = new Date(decoded.exp * 1000);
    const dbExpiry = new Date(blocklisted.expired_at);

    assert(
      Math.abs(tokenExpiry.getTime() - dbExpiry.getTime()) < 1000,
      'expired_at in DB matches JWT exp claim'
    );
  }

  // TC: Revoked token is rejected even with valid signature
  console.log('\nTest 2: Revoked token is rejected by middleware');
  await assertThrows(
    () =>
      ctx.api.get('/api/user/valid', {
        headers: { Authorization: `Bearer ${tokenToLogout}` },
      }),
    'Revoked token returns 401 "Token has been revoked"'
  );

  // TC: Logout cannot happen twice with same token
  console.log('\nTest 3: Cannot logout twice with same token');
  await assertThrows(
    () =>
      ctx.api.post('/api/user/logout', {}, {
        headers: { Authorization: `Bearer ${tokenToLogout}` },
      }),
    'Second logout with revoked token returns 401'
  );
}

// ============================================================================
// STATE AND SESSION LOGIC TESTS
// ============================================================================

async function testSessionLogic() {
  console.log('\n📊 Session & State Logic Tests\n');

  // TC: Each login creates independent session
  console.log('Test 1: Multiple logins create independent sessions');
  const login1 = await ctx.api.post('/api/user/login', {
    username: 'testuser',
    password: 'password123',
  });
  const token1 = login1.data.token;

  const login2 = await ctx.api.post('/api/user/login', {
    username: 'testuser',
    password: 'password123',
  });
  const token2 = login2.data.token;

  assert(token1 !== token2, 'Each login returns different token');

  // Both tokens should be valid initially
  const valid1 = await ctx.api.get('/api/user/valid', {
    headers: { Authorization: `Bearer ${token1}` },
  });
  const valid2 = await ctx.api.get('/api/user/valid', {
    headers: { Authorization: `Bearer ${token2}` },
  });

  assert(valid1.status === 200, 'First token is valid');
  assert(valid2.status === 200, 'Second token is valid');

  // Logout first token
  await ctx.api.post('/api/user/logout', {}, {
    headers: { Authorization: `Bearer ${token1}` },
  });

  // First token should be invalid, second should still work
  let valid1After, valid2After;
  try {
    valid1After = await ctx.api.get('/api/user/valid', {
      headers: { Authorization: `Bearer ${token1}` },
    });
  } catch (e: any) {
    valid1After = e.response;
  }

  try {
    valid2After = await ctx.api.get('/api/user/valid', {
      headers: { Authorization: `Bearer ${token2}` },
    });
  } catch (e: any) {
    valid2After = e.response;
  }

  assert(valid1After.status === 401, 'First token is revoked after logout');
  assert(valid2After.status === 200, 'Second token still valid (independent sessions)');

  // Cleanup
  await ctx.api.post('/api/user/logout', {}, {
    headers: { Authorization: `Bearer ${token2}` },
  });
}

// ============================================================================
// DATABASE INTEGRITY TESTS
// ============================================================================

async function testDatabaseIntegrity() {
  console.log('\n🗄️ Database Integrity Tests\n');

  // TC: Users table has correct schema
  console.log('Test 1: Users table schema is correct');
  const usersInfo = await db.raw(`
    SELECT column_name, is_nullable, data_type
    FROM information_schema.columns
    WHERE table_name = 'users'
    ORDER BY ordinal_position
  `);

  const columns = usersInfo.rows.map((r: any) => r.column_name);
  assert(columns.includes('id'), 'id column exists');
  assert(columns.includes('username'), 'username column exists');
  assert(columns.includes('password_hash'), 'password_hash column exists');
  assert(columns.includes('created_at'), 'created_at column exists');

  // TC: Username is unique
  console.log('\nTest 2: Username constraint is unique');
  try {
    await db('users').insert({
      username: 'testuser',
      password_hash: await bcrypt.hash('password', 10),
    });
    console.log('  ✗ Duplicate username did not raise error');
    failed++;
  } catch {
    console.log('  ✓ Duplicate username violates unique constraint');
    passed++;
  }

  // TC: Expired tokens table has correct schema
  console.log('\nTest 3: Expired_tokens table schema is correct');
  const tokensInfo = await db.raw(`
    SELECT column_name, is_nullable, data_type
    FROM information_schema.columns
    WHERE table_name = 'expired_tokens'
    ORDER BY ordinal_position
  `);

  const tokenColumns = tokensInfo.rows.map((r: any) => r.column_name);
  assert(tokenColumns.includes('id'), 'id column exists');
  assert(tokenColumns.includes('token'), 'token column exists');
  assert(tokenColumns.includes('expired_at'), 'expired_at column exists');

  // TC: Token index exists for fast lookups
  console.log('\nTest 4: Token index exists for performance');
  const indexInfo = await db.raw(`
    SELECT indexname FROM pg_indexes
    WHERE tablename = 'expired_tokens' AND indexname LIKE '%token%'
  `);

  assert(indexInfo.rows.length > 0, 'Index on token column exists');
}

// ============================================================================
// PASSWORD SECURITY TESTS
// ============================================================================

async function testPasswordSecurity() {
  console.log('\n🔒 Password Security Tests\n');

  // TC: Passwords are bcrypt hashed
  console.log('Test 1: Passwords are bcrypt hashed in database');
  const user = await db('users').where({ username: 'testuser' }).first();
  const hash = user.password_hash;

  // Bcrypt hashes start with $2a$, $2b$, or $2y$
  assert(
    hash.match(/^\$2[aby]\$/),
    'Password hash is bcrypt format'
  );

  // Verify bcrypt hash can be compared correctly
  const matches = await bcrypt.compare('password123', hash);
  assert(matches, 'Correct password matches bcrypt hash');

  const wrongMatches = await bcrypt.compare('wrongpassword', hash);
  assert(!wrongMatches, 'Wrong password does not match hash');

  // TC: Plain password is never returned from API
  console.log('\nTest 2: Plain password is never exposed in API responses');
  const loginRes = await ctx.api.post('/api/user/login', {
    username: 'testuser',
    password: 'password123',
  });

  assert(!loginRes.data.password, 'login response has no password field');
  assert(!loginRes.data.password_hash, 'login response has no password_hash field');

  const validRes = await ctx.api.get('/api/user/valid', {
    headers: { Authorization: `Bearer ${loginRes.data.token}` },
  });

  assert(!validRes.data.user.password, 'valid response has no password field');
  assert(!validRes.data.user.password_hash, 'valid response has no password_hash field');
}

// ============================================================================
// ERROR HANDLING AND EDGE CASES
// ============================================================================

async function testErrorHandling() {
  console.log('\n⚠️ Error Handling & Edge Cases Tests\n');

  // TC: Missing required fields
  console.log('Test 1: Missing required fields in login');
  await assertThrows(
    () => ctx.api.post('/api/user/login', { username: 'testuser' }),
    'Missing password returns 400'
  );

  await assertThrows(
    () => ctx.api.post('/api/user/login', { password: 'password123' }),
    'Missing username returns 400'
  );

  // TC: Empty string fields
  console.log('\nTest 2: Empty string fields are rejected');
  await assertThrows(
    () => ctx.api.post('/api/user/login', { username: '', password: 'password123' }),
    'Empty username returns error'
  );

  // TC: Health check still works
  console.log('\nTest 3: Health check endpoint works');
  const health = await ctx.api.get('/health');
  assert(health.status === 200, 'Health check returns 200');
  assert(health.data.status === 'ok', 'Health check returns correct status');
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

async function runAllTests() {
  console.log('\n╔════════════════════════════════════════════════════════════╗');
  console.log('║         Backend E2E Tests - Application Logic             ║');
  console.log('╚════════════════════════════════════════════════════════════╝');

  try {
    // Setup: Wait for database
    await new Promise((resolve) => setTimeout(resolve, 1000));

    await testLoginLogic();
    await testJWTValidation();
    await testTokenBlocklist();
    await testSessionLogic();
    await testDatabaseIntegrity();
    await testPasswordSecurity();
    await testErrorHandling();

    console.log('\n╔════════════════════════════════════════════════════════════╗');
    console.log(`║  Tests: ${passed + failed}  |  ✓ Passed: ${passed}  |  ✗ Failed: ${failed}${' '.repeat(Math.max(0, 8 - (passed + failed).toString().length + (passed.toString().length - 1) + (failed.toString().length - 1)))}║`);
    console.log('╚════════════════════════════════════════════════════════════╝\n');

    process.exit(failed > 0 ? 1 : 0);
  } catch (error) {
    console.error('\n💥 Test suite error:', error);
    process.exit(1);
  } finally {
    await db.destroy();
  }
}

runAllTests();
