import { Knex } from 'knex';

export async function up(knex: Knex): Promise<void> {
  await knex.schema.createTable('expired_tokens', (table) => {
    table.increments('id').primary();
    table.text('token').notNullable();
    table.timestamp('expired_at').notNullable();
    table.timestamp('created_at').defaultTo(knex.fn.now());
    table.index('token');
  });
}

export async function down(knex: Knex): Promise<void> {
  await knex.schema.dropTableIfExists('expired_tokens');
}
