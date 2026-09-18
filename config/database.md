# Database

## Setup

PostgreSQL 14+ is required. Create the application database and, if running tests, a separate test database:

```sql
CREATE DATABASE ghoralantinu;
CREATE DATABASE ghoralantinu_test;
```

## Migrations

Migration scripts live in `backend/src/db/migrations/` and are managed by Knex.

```bash
# from backend/
npm run migrate           # apply all pending migrations
npm run migrate:rollback  # roll back the last batch
```

For the test database, set `NODE_ENV=test` before running:

```bash
NODE_ENV=test npm run migrate
```

## Seeds

Seeds insert a test user and a test project for local development and testing:

```bash
# from backend/
npm run seed
```

| Resource | Value |
|---|---|
| Username | `testuser` |
| Password | `password123` |
| Project key | `test-project` |
| Project name | `Test Project` |

Seeds are idempotent — safe to run multiple times.

## Schema

### `users`

Stores registered users. Passwords are never stored in plain text.

| Column | Type | Constraints |
|---|---|---|
| `id` | integer | PK, auto-increment |
| `username` | varchar(255) | not null, unique |
| `password_hash` | varchar(255) | not null |
| `created_at` | timestamp | not null, default now() |

### `expired_tokens`

Server-side JWT revocation blocklist. When a user logs out, their token is inserted here. `activeLogin` middleware rejects any token found in this table.

| Column | Type | Constraints |
|---|---|---|
| `id` | integer | PK, auto-increment |
| `token` | text | not null, indexed |
| `expired_at` | timestamp | not null (set to the token's `exp` claim) |
| `created_at` | timestamp | not null, default now() |

Index on `token` for fast lookup on every authenticated request.

### `projects`

A project is the top-level organisational unit. Each project has a human-readable `key` used in URLs and API calls.

| Column | Type | Constraints |
|---|---|---|
| `id` | integer | PK, auto-increment |
| `key` | varchar(255) | not null, unique |
| `name` | varchar(255) | not null |
| `created_at` | timestamp | not null, default now() |
| `updated_at` | timestamp | not null, default now() |

### `project_users`

Many-to-many join table that controls which users have access to which projects, and at what role level.

| Column | Type | Constraints |
|---|---|---|
| `id` | integer | PK, auto-increment |
| `project_id` | integer | FK → `projects.id`, CASCADE DELETE |
| `user_id` | integer | FK → `users.id`, CASCADE DELETE |
| `role` | varchar(50) | not null, default `'member'` |
| `created_at` | timestamp | not null, default now() |

Unique constraint on `(project_id, user_id)` — a user can only appear once per project.

**Role values**: `admin`, `member`, `viewer`.

## Entity Relationships

```
users ─────────────────────────── project_users ─── projects
  id ←─────────────────── user_id    project_id ───→ id
  username                 role       key
  password_hash                       name
  created_at                          created_at
                                      updated_at

expired_tokens
  token  (blocklist for revoked JWTs)
  expired_at
```
