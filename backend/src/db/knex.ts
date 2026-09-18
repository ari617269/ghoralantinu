import 'dotenv/config';
import knex from 'knex';
import path from 'path';

const isTest = process.env.NODE_ENV === 'test';

const db = knex({
  client: 'pg',
  connection: {
    host: process.env.POSTGRES_HOST || 'localhost',
    port: parseInt(process.env.POSTGRES_PORT || '5432'),
    database: isTest
      ? (process.env.POSTGRES_TEST_DB || 'ghoralantinu_test')
      : (process.env.POSTGRES_DB || 'ghoralantinu'),
    user: process.env.POSTGRES_USER || 'postgres',
    password: process.env.POSTGRES_PASSWORD || 'postgres',
  },
  migrations: {
    directory: path.join(__dirname, 'migrations'),
    extension: 'ts',
    loadExtensions: ['.ts'],
  },
  seeds: {
    directory: path.join(__dirname, 'seeds'),
    extension: 'ts',
    loadExtensions: ['.ts'],
  },
  pool: { min: 2, max: 10 },
});

export default db;
export type Db = typeof db;
