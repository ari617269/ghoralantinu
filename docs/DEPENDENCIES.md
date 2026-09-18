# Dependencies

## Backend (`backend/package.json`)

### Runtime

| Package | Version | Purpose |
|---|---|---|
| `express` | 4.18.2 | HTTP server and routing |
| `knex` | 3.1.0 | SQL query builder and migration runner |
| `pg` | ^8.23.0 | PostgreSQL driver (used by Knex) |
| `postgres` | 3.4.4 | PostgreSQL client (direct queries) |
| `jsonwebtoken` | ^9.0.3 | JWT signing and verification |
| `bcryptjs` | ^3.0.3 | Password hashing |
| `dotenv` | ^18.0.0 | Environment variable loading |
| `typescript` | 5.3.3 | TypeScript compiler |

### Development

| Package | Version | Purpose |
|---|---|---|
| `jest` | ^30.5.1 | Test runner |
| `ts-jest` | ^29.4.12 | TypeScript preprocessor for Jest |
| `supertest` | ^7.2.2 | HTTP assertion library for Express |
| `ts-node` | 10.9.2 | TypeScript execution for dev server |
| `eslint` | 8.56.0 | Linter |
| `@typescript-eslint/parser` | 6.17.0 | TypeScript ESLint parser |
| `@typescript-eslint/eslint-plugin` | 6.17.0 | TypeScript ESLint rules |
| `prettier` | 3.1.1 | Code formatter |
| `@types/express` | 4.17.21 | Express type definitions |
| `@types/node` | 20.10.6 | Node.js type definitions |
| `@types/bcryptjs` | ^2.4.6 | bcryptjs type definitions |
| `@types/jsonwebtoken` | ^9.0.10 | jsonwebtoken type definitions |
| `@types/pg` | ^8.23.1 | pg type definitions |
| `@types/supertest` | ^7.2.1 | supertest type definitions |
| `@types/jest` | ^30.0.0 | Jest type definitions |

## Frontend (`frontend/package.json`)

### Runtime

| Package | Version | Purpose |
|---|---|---|
| `react` | 18.2.0 | UI library |
| `react-dom` | 18.2.0 | React DOM renderer |
| `react-router-dom` | 6.20.1 | Client-side routing |
| `@reduxjs/toolkit` | 1.9.7 | Redux state management |
| `react-redux` | 8.1.3 | React bindings for Redux |
| `axios` | 1.6.5 | HTTP client (auth API calls) |

### Development

| Package | Version | Purpose |
|---|---|---|
| `vite` | 5.0.8 | Build tool and dev server |
| `@vitejs/plugin-react` | 4.2.1 | Vite React plugin |
| `@playwright/test` | ^1.63.0 | End-to-end browser testing |
| `typescript` | 5.3.3 | TypeScript compiler |
| `eslint` | 8.56.0 | Linter |
| `@typescript-eslint/parser` | 6.17.0 | TypeScript ESLint parser |
| `@typescript-eslint/eslint-plugin` | 6.17.0 | TypeScript ESLint rules |
| `prettier` | 3.1.1 | Code formatter |
| `@types/react` | 18.2.45 | React type definitions |
| `@types/react-dom` | 18.2.18 | React DOM type definitions |

## Root workspace (`package.json`)

| Package | Version | Purpose |
|---|---|---|
| `concurrently` | ^8.0.0 | Run backend and frontend dev servers simultaneously |
