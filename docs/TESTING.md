# Testing & Development

## Testing Strategy

Only end-to-end (E2E) testing. No unit or integration tests.

### Backend E2E
- Full API workflows against PostgreSQL
- Run: `npm run test:e2e`

### Frontend E2E
- User workflows through the UI
- Run: `npm run test:e2e`

## Code Quality

### Type Checking
- Backend: `npm run type-check`
- Frontend: `npm run type-check`

### Linting
- ESLint for both backend and frontend
- Run: `npm run lint`
- Max warnings: 0 (all warnings treated as errors)

### Formatting
- Prettier for automatic code formatting
- Run: `npm run format`

## Development Commands

### Backend
```bash
npm run dev         # Start dev server
npm run build       # Compile TypeScript
npm run type-check  # Check types
npm run lint        # Run ESLint
npm run format      # Format code
npm run test:e2e    # Run E2E tests
```

### Frontend
```bash
npm run dev         # Start dev server
npm run build       # Build for production
npm run type-check  # Check types
npm run lint        # Run ESLint
npm run format      # Format code
npm run test:e2e    # Run E2E tests
```
