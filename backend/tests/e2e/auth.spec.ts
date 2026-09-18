import 'dotenv/config';
import request from 'supertest';
import express from 'express';
import jwt from 'jsonwebtoken';
import bcrypt from 'bcryptjs';
import db from '../../src/db/knex';
import userRouter from '../../src/routes/user';

// Setup Express app for testing
const app = express();
app.use(express.json());
app.use('/api/user', userRouter);

describe('Backend Auth E2E Tests - Application Logic', () => {
  let validToken: string;
  let testUserId: number;

  beforeAll(async () => {
    // Ensure tables exist
    await db.migrate.latest();
  });

  afterAll(async () => {
    await db.destroy();
  });

  describe('Login Logic - Password Verification & JWT Generation', () => {
    test('TC1.1: Valid credentials return JWT with correct user data', async () => {
      const response = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      expect(response.status).toBe(200);
      expect(response.body).toHaveProperty('token');
      expect(response.body).toHaveProperty('user');
      expect(response.body.user.username).toBe('testuser');
      expect(response.body.user.id).toBe(1);

      // Verify JWT structure and payload
      const decoded = jwt.decode(response.body.token) as any;
      expect(decoded).toHaveProperty('id');
      expect(decoded).toHaveProperty('username');
      expect(decoded).toHaveProperty('exp');
      expect(decoded.id).toBe(1);
      expect(decoded.username).toBe('testuser');

      // Store for later tests
      validToken = response.body.token;
      testUserId = response.body.user.id;
    });

    test('TC1.2: Invalid password is rejected via bcrypt', async () => {
      const response = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'wrongpassword' });

      expect(response.status).toBe(401);
      expect(response.body.error).toBe('Invalid credentials');
    });

    test('TC1.3: Nonexistent username returns same error as wrong password (no enumeration)', async () => {
      const wrongUserRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'nonexistent', password: 'password123' });

      const wrongPassRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'wrongpassword' });

      expect(wrongUserRes.status).toBe(401);
      expect(wrongPassRes.status).toBe(401);
      expect(wrongUserRes.body.error).toBe(wrongPassRes.body.error);
    });

    test('TC1.4: JWT has 1-hour expiry', async () => {
      const response = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const decoded = jwt.decode(response.body.token) as any;
      const nowSeconds = Math.floor(Date.now() / 1000);
      const expiryMinutes = (decoded.exp - nowSeconds) / 60;

      // Should be approximately 60 minutes
      expect(expiryMinutes).toBeGreaterThan(59);
      expect(expiryMinutes).toBeLessThan(61);
    });

    test('TC1.5: Missing required fields returns 400', async () => {
      const noPassword = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser' });

      const noUsername = await request(app)
        .post('/api/user/login')
        .send({ password: 'password123' });

      expect(noPassword.status).toBe(400);
      expect(noUsername.status).toBe(400);
    });
  });

  describe('JWT Validation Middleware - activeLogin Logic', () => {
    test('TC2.1: Valid token passes middleware and returns user data', async () => {
      const response = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body.valid).toBe(true);
      expect(response.body.user.id).toBe(testUserId);
      expect(response.body.user.username).toBe('testuser');
    });

    test('TC2.2: Missing Authorization header returns 401', async () => {
      const response = await request(app).get('/api/user/valid');

      expect(response.status).toBe(401);
      expect(response.body.error).toContain('Authorization');
    });

    test('TC2.3: Malformed Bearer format returns 401', async () => {
      const response = await request(app)
        .get('/api/user/valid')
        .set('Authorization', 'InvalidFormat token');

      expect(response.status).toBe(401);
    });

    test('TC2.4: Invalid JWT signature returns 401', async () => {
      const fakeToken = jwt.sign(
        { id: 1, username: 'testuser' },
        'wrong-secret',
        { expiresIn: '1h' }
      );

      const response = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${fakeToken}`);

      expect(response.status).toBe(401);
      expect(response.body.error).toContain('Invalid or expired');
    });

    test('TC2.5: Expired JWT returns 401', async () => {
      const expiredToken = jwt.sign(
        { id: 1, username: 'testuser' },
        process.env.JWT_SECRET!,
        { expiresIn: '-1h' }
      );

      const response = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${expiredToken}`);

      expect(response.status).toBe(401);
      expect(response.body.error).toContain('Invalid or expired');
    });
  });

  describe('Token Blocklist Logic - Logout & Revocation', () => {
    test('TC3.1: Logout adds token to expired_tokens table', async () => {
      // Get a fresh token for logout
      const loginRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const tokenToLogout = loginRes.body.token;

      // Logout
      const logoutRes = await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${tokenToLogout}`);

      expect(logoutRes.status).toBe(200);
      expect(logoutRes.body.message).toContain('Logged out');

      // Verify token is in database
      const blocklisted = await db('expired_tokens')
        .where({ token: tokenToLogout })
        .first();

      expect(blocklisted).toBeDefined();
      expect(blocklisted.token).toBe(tokenToLogout);
    });

    test('TC3.2: Revoked token is rejected even with valid signature', async () => {
      // Get and logout a token
      const loginRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const tokenToRevoke = loginRes.body.token;

      await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${tokenToRevoke}`);

      // Try to use revoked token
      const response = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${tokenToRevoke}`);

      expect(response.status).toBe(401);
      expect(response.body.error).toContain('revoked');
    });

    test('TC3.3: Cannot logout twice with same token', async () => {
      // Get and logout a token
      const loginRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const token = loginRes.body.token;

      const logout1 = await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${token}`);

      expect(logout1.status).toBe(200);

      // Try to logout again with same token
      const logout2 = await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${token}`);

      expect(logout2.status).toBe(401);
    });

    test('TC3.4: expired_at in DB matches JWT exp claim', async () => {
      // Get and logout a token
      const loginRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const token = loginRes.body.token;
      const decoded = jwt.decode(token) as any;

      await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${token}`);

      // Get from database
      const blocklisted = await db('expired_tokens')
        .where({ token })
        .first();

      const tokenExpiry = new Date(decoded.exp * 1000);
      const dbExpiry = new Date(blocklisted.expired_at);

      // Should be within 1 second
      expect(Math.abs(tokenExpiry.getTime() - dbExpiry.getTime())).toBeLessThan(1000);
    });
  });

  describe('Session Independence Logic', () => {
    test('TC4.1: Each login creates independent session with different token', async () => {
      const login1 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const login2 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      expect(login1.body.token).not.toBe(login2.body.token);

      // Both tokens should be valid
      const valid1 = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${login1.body.token}`);

      const valid2 = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${login2.body.token}`);

      expect(valid1.status).toBe(200);
      expect(valid2.status).toBe(200);
    });

    test('TC4.2: Logout of one session does not affect other sessions', async () => {
      // Create two sessions
      const login1 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const login2 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const token1 = login1.body.token;
      const token2 = login2.body.token;

      // Logout session 1
      await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${token1}`);

      // Token1 should be invalid
      const check1 = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${token1}`);

      // Token2 should still be valid
      const check2 = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${token2}`);

      expect(check1.status).toBe(401);
      expect(check2.status).toBe(200);

      // Cleanup
      await request(app)
        .post('/api/user/logout')
        .set('Authorization', `Bearer ${token2}`);
    });
  });

  describe('Password Security Logic', () => {
    test('TC5.1: Passwords are bcrypt hashed in database', async () => {
      const user = await db('users').where({ username: 'testuser' }).first();

      // Bcrypt hash format check
      expect(user.password_hash).toMatch(/^\$2[aby]\$/);

      // Verify hash works
      const matches = await bcrypt.compare('password123', user.password_hash);
      expect(matches).toBe(true);

      const wrongMatches = await bcrypt.compare('wrongpassword', user.password_hash);
      expect(wrongMatches).toBe(false);
    });

    test('TC5.2: Plain passwords are never exposed in responses', async () => {
      const loginRes = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      expect(loginRes.body).not.toHaveProperty('password');
      expect(loginRes.body).not.toHaveProperty('password_hash');

      const validRes = await request(app)
        .get('/api/user/valid')
        .set('Authorization', `Bearer ${loginRes.body.token}`);

      expect(validRes.body.user).not.toHaveProperty('password');
      expect(validRes.body.user).not.toHaveProperty('password_hash');
    });
  });

  describe('Database Integrity', () => {
    test('TC6.1: Users table has correct schema', async () => {
      const result = await db.raw(`
        SELECT column_name, is_nullable, data_type
        FROM information_schema.columns
        WHERE table_name = 'users'
        ORDER BY ordinal_position
      `);

      const columns = result.rows.map((r: any) => r.column_name);
      expect(columns).toContain('id');
      expect(columns).toContain('username');
      expect(columns).toContain('password_hash');
      expect(columns).toContain('created_at');
    });

    test('TC6.2: Username unique constraint enforced', async () => {
      expect.assertions(1);

      try {
        await db('users').insert({
          username: 'testuser',
          password_hash: await bcrypt.hash('password', 10),
        });
        fail('Should have thrown unique constraint error');
      } catch (error: any) {
        expect(error.message).toContain('unique');
      }
    });

    test('TC6.3: Expired tokens table has correct schema', async () => {
      const result = await db.raw(`
        SELECT column_name, is_nullable, data_type
        FROM information_schema.columns
        WHERE table_name = 'expired_tokens'
        ORDER BY ordinal_position
      `);

      const columns = result.rows.map((r: any) => r.column_name);
      expect(columns).toContain('id');
      expect(columns).toContain('token');
      expect(columns).toContain('expired_at');
      expect(columns).toContain('created_at');
    });

    test('TC6.4: Token index exists for performance', async () => {
      const result = await db.raw(`
        SELECT indexname FROM pg_indexes
        WHERE tablename = 'expired_tokens' AND indexname LIKE '%token%'
      `);

      expect(result.rows.length).toBeGreaterThan(0);
    });
  });

  describe('Error Handling', () => {
    test('TC7.1: Server recovers from errors gracefully', async () => {
      // First request with valid data
      const res1 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      expect(res1.status).toBe(200);

      // After error, should still work
      const res2 = await request(app)
        .post('/api/user/login')
        .send({ username: 'invalid', password: 'invalid' });

      expect(res2.status).toBe(401);

      // Should still be able to make valid requests
      const res3 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      expect(res3.status).toBe(200);
    });
  });
});
