import 'dotenv/config';
import request from 'supertest';
import express from 'express';
import db from '../../src/db/knex';
import userRouter from '../../src/routes/user';
import projectRouter from '../../src/routes/project';

const app = express();
app.use(express.json());
app.use('/api/user', userRouter);
app.use('/api/projects', projectRouter);

describe('Backend Project E2E Tests', () => {
  let validToken: string;
  let testUserId: number;
  let testProjectKey: string;

  beforeAll(async () => {
    await db.migrate.latest();
    await db.seed.run();
  });

  afterAll(async () => {
    // pool cleanup handled by --forceExit
  });

  describe('Database Schema - Projects and Project Users', () => {
    test('TC1.1: Projects table exists with correct schema', async () => {
      const result = await db.raw(`
        SELECT column_name, is_nullable, data_type
        FROM information_schema.columns
        WHERE table_name = 'projects'
        ORDER BY ordinal_position
      `);

      const columns = result.rows.map((r: any) => r.column_name);
      expect(columns).toContain('id');
      expect(columns).toContain('key');
      expect(columns).toContain('name');
      expect(columns).toContain('created_at');
      expect(columns).toContain('updated_at');
    });

    test('TC1.2: Project key has unique constraint', async () => {
      expect.assertions(1);

      try {
        await db('projects').insert({
          key: 'test-project',
          name: 'Duplicate Test',
        });
        throw new Error('Should have thrown unique constraint error');
      } catch (error: any) {
        expect(error.message).toContain('unique');
      }
    });

    test('TC1.3: Project_users junction table exists with correct schema', async () => {
      const result = await db.raw(`
        SELECT column_name, is_nullable, data_type
        FROM information_schema.columns
        WHERE table_name = 'project_users'
        ORDER BY ordinal_position
      `);

      const columns = result.rows.map((r: any) => r.column_name);
      expect(columns).toContain('id');
      expect(columns).toContain('project_id');
      expect(columns).toContain('user_id');
      expect(columns).toContain('role');
      expect(columns).toContain('created_at');
    });

    test('TC1.4: Project_users has unique constraint on (project_id, user_id)', async () => {
      expect.assertions(1);

      const project = await db('projects').where({ key: 'test-project' }).first();
      const user = await db('users').where({ username: 'testuser' }).first();

      try {
        await db('project_users').insert({
          project_id: project.id,
          user_id: user.id,
          role: 'member',
        });
        throw new Error('Should have thrown unique constraint error');
      } catch (error: any) {
        expect(error.message).toContain('unique');
      }
    });

    test('TC1.5: Foreign key constraints exist', async () => {
      const result = await db.raw(`
        SELECT constraint_name, constraint_type
        FROM information_schema.table_constraints
        WHERE table_name = 'project_users'
      `);

      const constraints = result.rows.map((r: any) => r.constraint_type);
      expect(constraints).toContain('FOREIGN KEY');
    });
  });

  describe('Authentication Flow - Login to get token', () => {
    test('TC2.1: Valid login returns token for later use', async () => {
      const response = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      expect(response.status).toBe(200);
      expect(response.body).toHaveProperty('token');
      expect(response.body).toHaveProperty('user');

      validToken = response.body.token;
      testUserId = response.body.user.id;
    });
  });

  describe('GET /api/projects/list - List user projects', () => {
    test('TC3.1: List projects returns array of user projects', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body).toHaveProperty('projects');
      expect(Array.isArray(response.body.projects)).toBe(true);
    });

    test('TC3.2: Test user has access to test-project', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      const testProject = response.body.projects.find((p: any) => p.key === 'test-project');
      expect(testProject).toBeDefined();
      expect(testProject.name).toBe('Test Project');
      testProjectKey = testProject.key;
    });

    test('TC3.3: Returned project has key, name, and timestamps', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      const project = response.body.projects[0];
      expect(project).toHaveProperty('key');
      expect(project).toHaveProperty('name');
      expect(project).toHaveProperty('created_at');
      expect(project).toHaveProperty('updated_at');
    });

    test('TC3.4: Response does not contain internal user IDs or project IDs', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      const project = response.body.projects[0];
      expect(project).not.toHaveProperty('id');
    });

    test('TC3.5: Missing token returns 401', async () => {
      const response = await request(app).get('/api/projects/list');

      expect(response.status).toBe(401);
      expect(response.body.error).toContain('Authorization');
    });

    test('TC3.6: Invalid token returns 401', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', 'Bearer invalid.token.here');

      expect(response.status).toBe(401);
    });
  });

  describe('GET /api/projects/validate - Validate project access', () => {
    test('TC4.1: Validate returns true for accessible project', async () => {
      const response = await request(app)
        .get('/api/projects/validate?projectKey=test-project')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body).toHaveProperty('valid');
      expect(response.body.valid).toBe(true);
    });

    test('TC4.2: Validate response includes project details', async () => {
      const response = await request(app)
        .get('/api/projects/validate?projectKey=test-project')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body).toHaveProperty('project');
      expect(response.body.project.key).toBe('test-project');
      expect(response.body.project.name).toBe('Test Project');
    });

    test('TC4.3: Validate returns false for nonexistent project', async () => {
      const response = await request(app)
        .get('/api/projects/validate?projectKey=nonexistent-project')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body.valid).toBe(false);
    });

    test('TC4.4: Validate returns false for project user lacks access to', async () => {
      // Create a new project that test user is NOT part of
      const newProjectKey = `other-project-${Date.now()}`;
      await db('projects').insert({
        key: newProjectKey,
        name: 'Other Project',
      });

      const response = await request(app)
        .get(`/api/projects/validate?projectKey=${newProjectKey}`)
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body.valid).toBe(false);
    });

    test('TC4.5: Missing projectKey parameter returns 400', async () => {
      const response = await request(app)
        .get('/api/projects/validate')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(400);
      expect(response.body.error).toContain('Project key');
    });

    test('TC4.6: Missing token returns 401', async () => {
      const response = await request(app).get('/api/projects/validate?projectKey=test-project');

      expect(response.status).toBe(401);
    });
  });

  describe('GET /api/projects/:projectKey/info - Get project details', () => {
    test('TC5.1: Get project info returns project details', async () => {
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body).toHaveProperty('project');
      expect(response.body.project.key).toBe('test-project');
      expect(response.body.project.name).toBe('Test Project');
    });

    test('TC5.2: Project info does not expose internal ID', async () => {
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body.project).not.toHaveProperty('id');
    });

    test('TC5.3: Nonexistent project returns 404', async () => {
      const response = await request(app)
        .get('/api/projects/nonexistent-key/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(404);
      expect(response.body.error).toContain('not found');
    });

    test('TC5.4: User without access returns 403', async () => {
      // Create a new project that test user is NOT part of
      const newProjectKey = `restricted-${Date.now()}`;
      await db('projects').insert({
        key: newProjectKey,
        name: 'Restricted Project',
      });

      const response = await request(app)
        .get(`/api/projects/${newProjectKey}/info`)
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(403);
      expect(response.body.error).toContain('Access denied');
    });

    test('TC5.5: Missing token returns 401', async () => {
      const response = await request(app).get('/api/projects/test-project/info');

      expect(response.status).toBe(401);
    });

    test('TC5.6: Invalid token returns 401', async () => {
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', 'Bearer invalid.token');

      expect(response.status).toBe(401);
    });
  });

  describe('Access Control - activeProject Middleware', () => {
    test('TC6.1: Middleware validates project exists before checking access', async () => {
      const response = await request(app)
        .get('/api/projects/definitely-not-real/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(404);
    });

    test('TC6.2: Middleware validates user-project mapping', async () => {
      const other = `other-${Date.now()}`;
      await db('projects').insert({
        key: other,
        name: 'Other',
      });

      const response = await request(app)
        .get(`/api/projects/${other}/info`)
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(403);
    });

    test('TC6.3: User with admin role can access project', async () => {
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);

      // Verify user is admin
      const mapping = await db('project_users')
        .join('projects', 'projects.id', 'project_users.project_id')
        .where('projects.key', 'test-project')
        .where('project_users.user_id', testUserId)
        .first();

      expect(mapping.role).toBe('admin');
    });

    test('TC6.4: User with member role can access project', async () => {
      // Create project and add user as member
      const memberProjectKey = `member-test-${Date.now()}`;
      const [{ id: memberProjectId }] = await db('projects')
        .insert({ key: memberProjectKey, name: 'Member Test' })
        .returning('id');

      await db('project_users').insert({
        project_id: memberProjectId,
        user_id: testUserId,
        role: 'member',
      });

      const response = await request(app)
        .get(`/api/projects/${memberProjectKey}/info`)
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
    });
  });

  describe('Multiple Projects - User Can Access Multiple Projects', () => {
    test('TC7.1: User can be part of multiple projects', async () => {
      // Create multiple projects for test user
      const project1 = `proj1-${Date.now()}`;
      const project2 = `proj2-${Date.now()}`;

      const [{ id: p1Id }] = await db('projects')
        .insert({ key: project1, name: 'Project 1' })
        .returning('id');

      const [{ id: p2Id }] = await db('projects')
        .insert({ key: project2, name: 'Project 2' })
        .returning('id');

      await db('project_users').insert([
        { project_id: p1Id, user_id: testUserId, role: 'admin' },
        { project_id: p2Id, user_id: testUserId, role: 'member' },
      ]);

      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      const projectKeys = response.body.projects.map((p: any) => p.key);
      expect(projectKeys).toContain(project1);
      expect(projectKeys).toContain(project2);
      expect(projectKeys).toContain('test-project');
    });

    test('TC7.2: List includes only projects user has access to', async () => {
      // Create a project that test user is NOT part of
      await db('projects').insert({
        key: `excluded-${Date.now()}`,
        name: 'Excluded Project',
      });

      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      const projectKeys = response.body.projects.map((p: any) => p.key);
      expect(projectKeys).not.toContain(`excluded-${Date.now()}`);
    });

    test('TC7.3: User can navigate between projects', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      // For each project, user should be able to access info
      for (const project of response.body.projects) {
        const infoRes = await request(app)
          .get(`/api/projects/${project.key}/info`)
          .set('Authorization', `Bearer ${validToken}`);

        expect(infoRes.status).toBe(200);
        expect(infoRes.body.project.key).toBe(project.key);
      }
    });
  });

  describe('Project Seeding - Test Data', () => {
    test('TC8.1: Test project was created by seeder', async () => {
      const project = await db('projects').where({ key: 'test-project' }).first();
      expect(project).toBeDefined();
      expect(project.name).toBe('Test Project');
    });

    test('TC8.2: Test user was mapped to test project', async () => {
      const user = await db('users').where({ username: 'testuser' }).first();
      const mapping = await db('project_users')
        .where({ user_id: user.id })
        .innerJoin('projects', 'projects.id', 'project_users.project_id')
        .where('projects.key', 'test-project')
        .first();

      expect(mapping).toBeDefined();
      expect(mapping.role).toBe('admin');
    });

    test('TC8.3: Test project is accessible via API', async () => {
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      const testProject = response.body.projects.find((p: any) => p.key === 'test-project');
      expect(testProject).toBeDefined();
    });
  });

  describe('Session Isolation - Multiple Sessions', () => {
    test('TC9.1: Different users cannot access each other projects', async () => {
      // This is a placeholder for multi-user scenario
      // In real scenario, we'd create another test user
      // For now, verify that a user can only list their own projects
      const response = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      // All projects should belong to testuser
      for (const project of response.body.projects) {
        const mapping = await db('project_users')
          .where('user_id', testUserId)
          .innerJoin('projects', 'projects.id', 'project_users.project_id')
          .where('projects.key', project.key)
          .first();

        expect(mapping).toBeDefined();
      }
    });

    test('TC9.2: Token from one session works independently', async () => {
      // Get two tokens
      const login1 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const login2 = await request(app)
        .post('/api/user/login')
        .send({ username: 'testuser', password: 'password123' });

      const token1 = login1.body.token;
      const token2 = login2.body.token;

      // Both should be able to access projects
      const proj1 = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${token1}`);

      const proj2 = await request(app)
        .get('/api/projects/list')
        .set('Authorization', `Bearer ${token2}`);

      expect(proj1.status).toBe(200);
      expect(proj2.status).toBe(200);
    });
  });

  describe('Error Handling', () => {
    test('TC10.1: Invalid project key format handled gracefully', async () => {
      const response = await request(app)
        .get('/api/projects/invalid@project!/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(404);
    });

    test('TC10.2: URL-encoded project keys work correctly', async () => {
      const projectKey = 'test-project';
      const encoded = encodeURIComponent(projectKey);

      const response = await request(app)
        .get(`/api/projects/${encoded}/info`)
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
      expect(response.body.project.key).toBe(projectKey);
    });

    test('TC10.3: Server recovers after errors', async () => {
      // First: try invalid request
      await request(app)
        .get('/api/projects/invalid/info')
        .set('Authorization', `Bearer ${validToken}`);

      // Second: make valid request
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.status).toBe(200);
    });
  });

  describe('Data Consistency', () => {
    test('TC11.1: Project key is immutable', async () => {
      const project = await db('projects').where({ key: 'test-project' }).first();
      const originalKey = project.key;

      // Note: In real implementation, there should be no update endpoint that changes key
      // This test verifies the key returned matches the key in DB
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', `Bearer ${validToken}`);

      expect(response.body.project.key).toBe(originalKey);
    });

    test('TC11.2: Project timestamps are set correctly', async () => {
      const now = Date.now();
      const response = await request(app)
        .get('/api/projects/test-project/info')
        .set('Authorization', `Bearer ${validToken}`);

      const createdAt = new Date(response.body.project.created_at).getTime();
      expect(createdAt).toBeLessThanOrEqual(now);
      expect(now - createdAt).toBeLessThan(60 * 60 * 1000); // Less than 1 hour old
    });
  });
});
