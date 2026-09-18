import { Knex } from 'knex';
import bcrypt from 'bcryptjs';

export async function seed(knex: Knex): Promise<void> {
  const existing = await knex('users').where({ username: 'testuser' }).first();
  if (existing) return;

  const hashedPassword = await bcrypt.hash('password123', 10);
  await knex('users').insert({
    username: 'testuser',
    password_hash: hashedPassword,
  });
}
