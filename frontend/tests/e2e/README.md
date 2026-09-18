# Frontend Testing

## E2E Testing

End-to-end tests verify user workflows in the application.

### Running Tests

```bash
npm run test:e2e
```

### Structure

- `tests/e2e/` — End-to-end test suites
- `tests/e2e/runner.js` — Test runner and orchestration

### Requirements

- Backend API running on configured port
- Frontend server running or started by test suite

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
