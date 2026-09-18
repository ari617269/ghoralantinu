import 'dotenv/config';
import type { Knex } from 'knex';

const config: Record<string, Knex.Config> = {
  development: {
    client: 'pg',
    connection: {
      host: process.env.POSTGRES_HOST || 'localhost',
      port: parseInt(process.env.POSTGRES_PORT || '5432'),
      database: process.env.POSTGRES_DB || 'ghoralantinu',
      user: process.env.POSTGRES_USER || 'postgres',
      password: process.env.POSTGRES_PASSWORD || 'postgres',
    },
    migrations: {
      directory: './src/db/migrations',
      extension: 'ts',
      loadExtensions: ['.ts'],
    },
    seeds: {
      directory: './src/db/seeds',
      extension: 'ts',
      loadExtensions: ['.ts'],
    },
    pool: { min: 2, max: 10 },
  },
};

const defaultEnv = process.env.NODE_ENV || 'development';
module.exports = config[defaultEnv] || config.development;

