# Contributing

Contributions are welcome. Please follow the guidelines below.

## Development Environment

### Requirements

- Node.js 20+
- PostgreSQL 14+
- npm 9+

### First-time setup

```bash
git clone <repo-url>
cd ghoralantinu
npm install
cp .env .env.local   # configure for your local Postgres
cd backend
npm run migrate
npm run seed
```

Start both servers:

```bash
# from repo root
npm run dev
```

See [README.md](README.md) for full setup instructions.

## Branching

- Branch from `main`.
- Use descriptive branch names: `feat/project-search`, `fix/token-expiry`, `docs/api-reference`.
- Open a pull request against `main`.

## Making Changes

- Keep each commit focused on one logical change.
- Write a clear commit message in the imperative mood: `Add project search endpoint`, not `added project search endpoint`.
- If your change modifies a public API endpoint or database schema, update the relevant docs in `docs/` and `config/database.md`.

## Tests

All changes must pass the existing test suite. New features and bug fixes should include tests.

### Backend (Jest + Supertest)

Tests live in `backend/tests/e2e/` and run against a real PostgreSQL test database.

```bash
# ensure ghoralantinu_test database exists and is migrated
NODE_ENV=test cd backend && npm run migrate
npm test
```

### Frontend (Playwright)

Tests live in `frontend/tests/e2e/` and drive a real browser. The backend must be running.

```bash
# terminal 1
cd backend && npm run dev

# terminal 2
cd frontend && npm test
```

## Code Quality

All PRs must pass linting and type checking with no warnings or errors.

```bash
# from backend/ or frontend/
npm run lint         # ESLint — 0 warnings allowed
npm run type-check   # tsc --noEmit
```

Format code with Prettier before committing:

```bash
npm run format
```

## Pull Request Checklist

- [ ] Tests pass (`npm test` in both `backend/` and `frontend/`)
- [ ] No lint errors (`npm run lint`)
- [ ] No type errors (`npm run type-check`)
- [ ] Code formatted (`npm run format`)
- [ ] Documentation updated if public API or schema changed
