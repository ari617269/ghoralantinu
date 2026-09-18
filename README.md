# ghoralantinu

A self-hosted data storage and access tool for small teams. Teams store and access their data on their own infrastructure via a JSON HTTP API and a web UI, with per-user authentication and project-based access control.

## Tech Stack

| Layer | Technology |
|---|---|
| Backend API | Node.js, Express 4, TypeScript |
| Frontend | React 18, Vite 5, Redux Toolkit |
| Database | PostgreSQL (via Knex query builder) |
| Auth | JWT (1 h expiry) + server-side token revocation |
| Testing | Jest + Supertest (backend), Playwright (frontend) |

## Prerequisites

- Node.js 20+
- PostgreSQL 14+
- npm 9+

## Setup

### 1. Clone and install

```bash
git clone <repo-url>
cd ghoralantinu
npm install
```

### 2. Create the databases

```sql
CREATE DATABASE ghoralantinu;
CREATE DATABASE ghoralantinu_test;  -- for running tests
```

### 3. Configure environment

Copy the root `.env` file and set values for your environment:

```bash
cp .env .env.local   # or edit .env directly
```

| Variable | Default | Description |
|---|---|---|
| `POSTGRES_HOST` | `localhost` | PostgreSQL host |
| `POSTGRES_PORT` | `5432` | PostgreSQL port |
| `POSTGRES_DB` | `ghoralantinu` | Database name |
| `POSTGRES_USER` | `postgres` | Database user |
| `POSTGRES_PASSWORD` | `postgres` | Database password |
| `API_PORT` | `3000` | Backend port |
| `JWT_SECRET` | — | **Required.** 256-bit random hex string |
| `NODE_ENV` | `development` | `development` \| `test` \| `production` |

Generate a suitable `JWT_SECRET`:

```bash
node -e "console.log(require('crypto').randomBytes(32).toString('hex'))"
```

### 4. Run migrations and seed

```bash
cd backend
npm run migrate   # create tables
npm run seed      # insert test user and test project
```

The seed creates:

| Resource | Value |
|---|---|
| Username | `testuser` |
| Password | `password123` |
| Project key | `test-project` |
| Project name | `Test Project` |

### 5. Start the development servers

From the repository root:

```bash
npm run dev
```

This starts both servers concurrently:

- Backend API: `http://localhost:3000`
- Frontend: `http://localhost:5173`

## API Reference

All API routes are under `/api`. Protected routes require `Authorization: Bearer <token>`.

### Authentication

| Method | Path | Auth | Description |
|---|---|---|---|
| `POST` | `/api/user/login` | — | Returns a JWT and user object |
| `POST` | `/api/user/logout` | Required | Revokes the current token |
| `GET` | `/api/user/valid` | Required | Validates the current token |

**Login request body:**

```json
{ "username": "testuser", "password": "password123" }
```

**Login response:**

```json
{
  "token": "<jwt>",
  "user": { "id": 1, "username": "testuser" }
}
```

### Projects

| Method | Path | Auth | Description |
|---|---|---|---|
| `GET` | `/api/projects/list` | Required | Returns all projects the user has access to |
| `GET` | `/api/projects/validate?projectKey=<key>` | Required | Checks existence and user access for a project |
| `GET` | `/api/projects/:projectKey/info` | Required | Returns details for a single project |

**GET /api/projects/list response:**

```json
[
  { "key": "test-project", "name": "Test Project", "created_at": "...", "updated_at": "..." }
]
```

### Health

```
GET /health  →  200 OK
```

## Running Tests

### Backend (Jest + Supertest)

Requires the `ghoralantinu_test` database to exist and migrations to have run against it.

```bash
cd backend
NODE_ENV=test npm run migrate   # run once
npm test
```

### Frontend (Playwright)

The Playwright config starts the frontend dev server automatically. The backend must be running.

```bash
# terminal 1
cd backend && npm run dev

# terminal 2
cd frontend && npm test
```

Run with the interactive UI:

```bash
cd frontend && npm run test:ui
```

## Project Structure

```
ghoralantinu/
├── backend/
│   ├── src/
│   │   ├── index.ts           # Express app entry point
│   │   ├── routes/            # user.ts, project.ts
│   │   ├── middleware/        # activeLogin.ts, activeProject.ts
│   │   ├── db/
│   │   │   ├── knex.ts        # DB connection
│   │   │   ├── migrations/    # Knex migration files
│   │   │   └── seeds/         # Test data seeds
│   │   └── types/             # Express + domain type extensions
│   └── tests/e2e/             # Jest + Supertest E2E specs
├── frontend/
│   ├── src/
│   │   ├── App.tsx            # Router and route definitions
│   │   ├── pages/             # Login, Home, ProjectDashboard
│   │   ├── components/        # ProtectedRoute, Breadcrumb
│   │   ├── api/               # auth.ts, projects.ts
│   │   ├── store/             # Redux slices (auth, project)
│   │   └── styles/            # theme.css (CSS custom properties)
│   └── tests/e2e/             # Playwright specs
├── docs/                      # Architecture, scope, testing docs
├── config/                    # Database setup notes
└── package.json               # npm workspace root
```

## Code Quality

```bash
# from backend/ or frontend/
npm run lint         # ESLint (0 warnings allowed)
npm run type-check   # TypeScript type check (no emit)
npm run format       # Prettier
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

See [LICENSE](LICENSE).
