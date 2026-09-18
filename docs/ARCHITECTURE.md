# Architecture

## System Overview

ghoralantinu is a monorepo containing a Node.js/Express REST API (`backend/`) and a React single-page application (`frontend/`). They are deployed independently; during development Vite proxies `/api` requests from the frontend to the backend.

```
Browser (port 5173)
  │
  ├── React SPA (Vite)
  │     ├── Redux store (in-memory auth state, project list, active project)
  │     ├── React Router (/, /login, /project/:key)
  │     └── Axios / fetch → /api (proxied to :3000 in dev)
  │
  └── /api → Express API (port 3000)
              ├── POST  /api/user/login
              ├── POST  /api/user/logout
              ├── GET   /api/user/valid
              ├── GET   /api/projects/list
              ├── GET   /api/projects/validate
              ├── GET   /api/projects/:key/info
              └── GET   /health
                    │
                    └── PostgreSQL
                          ├── users
                          ├── expired_tokens
                          ├── projects
                          └── project_users
```

## Backend

**Runtime**: Node.js with Express 4 and TypeScript (compiled to CommonJS via `tsc`).

**Database access**: Knex query builder connects to PostgreSQL. The connection is configured from environment variables; the test environment (`NODE_ENV=test`) connects to a separate `ghoralantinu_test` database to keep test runs isolated.

**Middleware chain (protected routes)**:

```
Request
  → activeLogin   — verifies JWT signature, checks expired_tokens blocklist,
                    attaches req.user = { id, username }
  → activeProject — resolves :projectKey, checks project_users membership,
                    attaches req.project (only on project routes)
  → Route handler
```

**Authentication flow**:

1. `POST /api/user/login` — verifies bcrypt hash, issues a signed JWT with `{ id, username }` payload and 1 h expiry.
2. `POST /api/user/logout` — inserts the token into `expired_tokens` with the JWT's original `exp` as `expired_at`.
3. Every subsequent request — `activeLogin` rejects tokens present in `expired_tokens`.

There is no token refresh mechanism. After expiry the client must log in again.

## Frontend

**Build tool**: Vite 5 with the React plugin. The dev server proxies `/api/*` to `http://localhost:3000`.

**State management**: Redux Toolkit with two slices:

| Slice | State |
|---|---|
| `auth` | `token`, `user`, `isAuthenticated` |
| `project` | `projects`, `activeProject`, `selectedProjectKey`, `loading`, `error` |

The JWT is stored **only in Redux memory** — never in `localStorage` or `sessionStorage`. A page refresh clears the token and redirects to `/login`. This is by design.

**Routing**:

| Path | Component | Guard |
|---|---|---|
| `/login` | `Login` | Redirects to `/` if already authenticated |
| `/` | `Home` | `ProtectedRoute` — redirects to `/login` if not authenticated |
| `/project/:projectKey` | `ProjectDashboard` | `ProtectedRoute` |
| `*` | — | Redirects to `/` |

**API layer**: `src/api/auth.ts` uses axios; `src/api/projects.ts` uses the native `fetch` API. Both send `Authorization: Bearer <token>` headers on protected calls.

## Database Schema

See [config/database.md](../config/database.md) for full schema details.

## Deployment

ghoralantinu is designed for self-hosted deployment on team-controlled infrastructure.

**Production checklist:**

- Set `NODE_ENV=production`.
- Set a strong random `JWT_SECRET` (32+ bytes of entropy).
- Run `npm run migrate` against the production database before starting.
- Serve the frontend build (`npm run build`) behind a reverse proxy (nginx, Caddy).
- Configure the reverse proxy to forward `/api/*` to the backend process.
- Add CORS middleware to the backend if the frontend and backend are served from different origins.
- Use a process manager (systemd, PM2) to run `node dist/index.js`.

## Repository Layout

```
ghoralantinu/
├── backend/
│   ├── src/
│   │   ├── index.ts            entry point, Express app setup
│   │   ├── routes/
│   │   │   ├── user.ts         /api/user routes
│   │   │   └── project.ts      /api/projects routes
│   │   ├── middleware/
│   │   │   ├── activeLogin.ts  JWT validation + blocklist check
│   │   │   └── activeProject.ts project existence + access check
│   │   ├── db/
│   │   │   ├── knex.ts         Knex connection (dev + test)
│   │   │   ├── migrations/     001_users, 002_expired_tokens, 003_projects
│   │   │   └── seeds/          001_test_user, 002_test_project
│   │   └── types/
│   │       ├── express.d.ts    req.user / req.project type augmentation
│   │       └── project.ts      Project, ProjectUser interfaces
│   └── tests/e2e/
│       ├── auth.spec.ts
│       └── projects.spec.ts
├── frontend/
│   ├── src/
│   │   ├── main.tsx            Redux Provider, global styles
│   │   ├── App.tsx             BrowserRouter + route definitions
│   │   ├── pages/
│   │   │   ├── Login.tsx
│   │   │   ├── Home.tsx
│   │   │   └── ProjectDashboard.tsx
│   │   ├── components/
│   │   │   ├── ProtectedRoute.tsx
│   │   │   └── Breadcrumb.tsx
│   │   ├── api/
│   │   │   ├── auth.ts         loginApi, logoutApi, validateTokenApi
│   │   │   └── projects.ts     getProjectsList, validateProject, getProjectInfo
│   │   ├── store/
│   │   │   ├── index.ts        Redux store + typed hooks
│   │   │   ├── authSlice.ts
│   │   │   └── projectSlice.ts
│   │   └── styles/
│   │       └── theme.css       CSS custom properties (greyscale palette)
│   └── tests/e2e/
│       ├── auth.spec.ts
│       ├── auth-logic.spec.ts
│       └── projects.spec.ts
├── config/
│   └── database.md             DB setup and schema reference
├── docs/                       Architecture, scope, testing, dependencies
└── package.json                npm workspace root (backend + frontend)
```
