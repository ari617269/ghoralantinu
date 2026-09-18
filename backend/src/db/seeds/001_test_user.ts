import { Knex } from 'knex';
import bcrypt from 'bcryptjs';

export async function seed(knex: Knex): Promise<void> {
  // Delete existing test user first (idempotent)
  await knex('users').where({ username: 'testuser' }).delete();

  // Hash the password
  const hashedPassword = await bcrypt.hash('password123', 10);

  // Insert test user
  await knex('users').insert({
    username: 'testuser',
    password_hash: hashedPassword,
  });
}
