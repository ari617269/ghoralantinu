# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

## [0.0.1] — Initial implementation

### Added

#### Authentication system

- `POST /api/user/login` — username/password login returning a signed JWT (1 h expiry) and user object.
- `POST /api/user/logout` — server-side token revocation: inserts the token into an `expired_tokens` blocklist table.
- `GET /api/user/valid` — token validation endpoint; returns `{ valid: true, user }` for active tokens.
- `activeLogin` middleware — validates JWT signature, rejects expired and revoked tokens, attaches `req.user`.
- `bcryptjs` password hashing (cost factor 10).
- `expired_tokens` database table with index on `token` for fast blocklist lookup.

#### Projects

- `GET /api/projects/list` — returns all projects the authenticated user has access to.
- `GET /api/projects/validate?projectKey=<key>` — checks project existence and user membership.
- `GET /api/projects/:projectKey/info` — returns project details; 404 if not found, 403 if unauthorised.
- `activeProject` middleware — resolves project by key, enforces membership via `project_users`, attaches `req.project`.
- `projects` and `project_users` database tables with role support (`admin`, `member`, `viewer`).

#### Frontend

- React 18 SPA with Vite 5 and TypeScript.
- Login page with form validation and redirect on success.
- Home page: fetches and displays a grid of accessible projects; logout button.
- Project dashboard page: project name, key, creation date, breadcrumb navigation.
- `ProtectedRoute` component redirecting unauthenticated users to `/login`.
- Redux Toolkit state management with `auth` and `project` slices.
- In-memory JWT storage only — no `localStorage` or `sessionStorage`.
- Vite dev server proxy: `/api/*` → `http://localhost:3000`.
- Greyscale CSS theme via CSS custom properties.

#### Database

- Knex migration runner with three migrations: `users`, `expired_tokens`, `projects`+`project_users`.
- Idempotent seed data: `testuser` / `password123` mapped to `test-project`.

#### Testing

- Backend E2E test suite (Jest + Supertest): auth flows, token revocation, projects API, middleware, DB schema integrity — ~55 test cases.
- Frontend E2E test suite (Playwright, Chromium): auth logic, session management, project navigation, access control — ~50 test cases.

#### Infrastructure

- npm workspace monorepo (`backend/` + `frontend/`).
- `npm run dev` at the root starts both servers concurrently via `concurrently`.
- ESLint and Prettier configured for both packages (0 warnings allowed).
- TypeScript strict mode enabled in both packages.
