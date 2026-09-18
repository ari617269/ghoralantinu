import { Knex } from 'knex';

export async function seed(knex: Knex): Promise<void> {
  const testUser = await knex('users').where({ username: 'testuser' }).first();

  if (!testUser) {
    console.log('Test user not found, skipping project seed');
    return;
  }

  let project = await knex('projects').where({ key: 'test-project' }).first();

  if (!project) {
    const [inserted] = await knex('projects')
      .insert({ key: 'test-project', name: 'Test Project' })
      .returning('*');
    project = inserted;
  }

  const existingMapping = await knex('project_users')
    .where({ project_id: project.id, user_id: testUser.id })
    .first();

  if (!existingMapping) {
    await knex('project_users').insert({
      project_id: project.id,
      user_id: testUser.id,
      role: 'admin',
    });
  }
}
