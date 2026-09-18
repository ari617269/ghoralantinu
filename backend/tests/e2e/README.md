# Backend Testing

## E2E Testing

End-to-end tests verify the full API workflow against a running database.

### Running Tests

```bash
npm run test:e2e
```

### Structure

- `tests/e2e/` — End-to-end test suites
- `tests/e2e/runner.ts` — Test runner and orchestration

### Requirements

- PostgreSQL instance running
- Environment variables configured (`.env.test`)
- Backend server running or started by test suite

## Development

### Type Checking

```bash
npm run type-check
```

### Linting

```bash
npm run lint
```

### Code Formatting

```bash
npm run format
```
