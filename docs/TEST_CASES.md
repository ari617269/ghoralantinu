# Project Entity System - E2E Test Cases Summary

## Executive Summary

- **Backend Tests**: 51 test cases across 11 categories
- **Frontend Tests**: 40+ test cases across 10 categories
- **Total Coverage**: Database schema, API endpoints, middleware, UI/UX, navigation, access control, error handling

---

## Backend Test Cases (backend/tests/e2e/projects.spec.ts)

### TC1: Database Schema Tests (5 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC1.1 | Projects table schema | Validates projects table structure | Schema |
| TC1.2 | Project key uniqueness | Enforces unique project key constraint | Constraint |
| TC1.3 | Project_users schema | Validates junction table structure | Schema |
| TC1.4 | Project_users uniqueness | Enforces (project_id, user_id) uniqueness | Constraint |
| TC1.5 | Foreign key constraints | Validates referential integrity | Constraint |

### TC2: Authentication Flow (1 test)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC2.1 | Valid login | Obtains JWT token for subsequent tests | Auth |

### TC3: GET /api/projects/list (6 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC3.1 | List projects | Returns array of projects | Functionality |
| TC3.2 | Test project exists | Test project appears in list | Data |
| TC3.3 | Project fields | Response includes key, name, timestamps | Structure |
| TC3.4 | No internal IDs | Response doesn't expose internal IDs | Security |
| TC3.5 | Missing token | Returns 401 without token | Auth |
| TC3.6 | Invalid token | Returns 401 with invalid token | Auth |

### TC4: GET /api/projects/validate (6 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC4.1 | Valid project | Returns valid=true for accessible project | Functionality |
| TC4.2 | Response details | Includes project details in response | Structure |
| TC4.3 | Nonexistent project | Returns valid=false for missing project | Functionality |
| TC4.4 | Unauthorized access | Returns valid=false for inaccessible project | Access Control |
| TC4.5 | Missing parameter | Returns 400 without projectKey | Validation |
| TC4.6 | Missing token | Returns 401 without token | Auth |

### TC5: GET /api/projects/:projectKey/info (6 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC5.1 | Get project info | Returns full project details | Functionality |
| TC5.2 | No internal ID | Response doesn't expose ID | Security |
| TC5.3 | Not found | Returns 404 for nonexistent project | Error Handling |
| TC5.4 | Access denied | Returns 403 for unauthorized user | Access Control |
| TC5.5 | Missing token | Returns 401 without token | Auth |
| TC5.6 | Invalid token | Returns 401 with invalid token | Auth |

### TC6: Access Control - activeProject Middleware (4 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC6.1 | Project existence check | Validates project exists before access check | Middleware |
| TC6.2 | User-project mapping | Validates user in project_users table | Middleware |
| TC6.3 | Admin role access | Admin role can access project | Role-based |
| TC6.4 | Member role access | Member role can access project | Role-based |

### TC7: Multiple Projects (3 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC7.1 | Multiple projects | User can be in multiple projects | Functionality |
| TC7.2 | Filtered list | List only shows user's projects | Access Control |
| TC7.3 | Project transitions | User can access all their projects | Functionality |

### TC8: Project Seeding (3 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC8.1 | Project seeded | Test project exists in database | Data |
| TC8.2 | User mapped | Test user mapped to test project | Data |
| TC8.3 | API accessible | Test project accessible via API | Integration |

### TC9: Session Isolation (2 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC9.1 | User isolation | Users can't access other users' projects | Access Control |
| TC9.2 | Session independence | Multiple sessions work independently | Session |

### TC10: Error Handling (3 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC10.1 | Invalid key format | Handles unusual project keys gracefully | Error Handling |
| TC10.2 | URL encoding | Properly decodes URL-encoded project keys | URL Handling |
| TC10.3 | Recovery | Server recovers from errors | Resilience |

### TC11: Data Consistency (2 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC11.1 | Key immutability | Project key never changes | Data Integrity |
| TC11.2 | Timestamps | Timestamps correctly set | Data Integrity |

---

## Frontend Test Cases (frontend/tests/e2e/projects.spec.ts)

### TC1: Home Page - Project List (8 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC1.1 | Projects list displayed | Home page shows project list | Render |
| TC1.2 | Test project visible | Test project card appears | Render |
| TC1.3 | Card content | Project name and key shown | Render |
| TC1.4 | Grid layout | Projects displayed in grid | Layout |
| TC1.5 | Loading state | Loading indicator shown during fetch | UX |
| TC1.6 | Error handling | Error message displayed on failure | UX |
| TC1.7 | Header visible | App header and logout button shown | Render |
| TC1.8 | Welcome message | Username shown in greeting | Render |

### TC2: Project Navigation (4 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC2.1 | Card navigation | Clicking card navigates to project | Navigation |
| TC2.2 | Dashboard loads | Project details load on dashboard | Navigation |
| TC2.3 | Breadcrumb visible | Breadcrumb navigation shown | Render |
| TC2.4 | Breadcrumb link | Home link in breadcrumb navigates correctly | Navigation |

### TC3: Project Dashboard (6 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC3.1 | Project details | Dashboard shows project info | Render |
| TC3.2 | Key formatting | Project key displayed in monospace | Styling |
| TC3.3 | Creation date | Created date shown | Render |
| TC3.4 | Card styling | Info displayed in styled card | Styling |
| TC3.5 | Placeholder | Future content placeholder shown | Render |
| TC3.6 | Loading state | Loading indicator shown during fetch | UX |

### TC4: Access Control (4 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC4.1 | 404 handling | Non-existent project shows error | Error Handling |
| TC4.2 | 403 handling | Unauthorized project shows error | Error Handling |
| TC4.3 | Error button | "Go back" button in error message | UX |
| TC4.4 | Error navigation | "Go back" button works correctly | Navigation |

### TC5: Header and Navigation (4 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC5.1 | Header persistent | Header visible on all pages | Render |
| TC5.2 | Logout from home | Logout button works on home page | Functionality |
| TC5.3 | Logout from project | Logout button works on project page | Functionality |
| TC5.4 | App title | Title visible on page | Render |

### TC6: Responsive Design (3 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC6.1 | Mobile home | Home page responsive on 375x667 | Responsive |
| TC6.2 | Mobile project | Project page responsive on 375x667 | Responsive |
| TC6.3 | Mobile breadcrumb | Breadcrumb visible on mobile | Responsive |

### TC7: Multiple Projects Navigation (2 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC7.1 | Between projects | Can navigate between multiple projects | Navigation |
| TC7.2 | URL projection key | Project key in URL matches project | Navigation |

### TC8: Authentication Integration (2 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC8.1 | Unauthorized redirect | Unauthorized access handled | Auth |
| TC8.2 | Token validation | Token validated on page load | Auth |

### TC9: Error States (2 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC9.1 | API failure handling | Graceful error on API failure | Error Handling |
| TC9.2 | Error recovery | App recovers after error navigation | Resilience |

### TC10: Browser History (2 tests)

| ID | Test | Description | Type |
|----|------|-------------|------|
| TC10.1 | Back button | Browser back button works | Navigation |
| TC10.2 | Direct navigation | Direct URL navigation works | Navigation |

---

## Test Coverage Matrix

### Coverage by Component

| Component | Backend | Frontend | Total |
|-----------|---------|----------|-------|
| Database/Schema | 5 | - | 5 |
| Authentication | 1 | 2 | 3 |
| /api/projects/list | 6 | 8 | 14 |
| /api/projects/validate | 6 | - | 6 |
| /api/projects/:key/info | 6 | 6 | 12 |
| Middleware | 4 | - | 4 |
| Access Control | 3 | 4 | 7 |
| Multi-project | 3 | 2 | 5 |
| Navigation | - | 10 | 10 |
| Error Handling | 3 | 4 | 7 |
| Data Integrity | 2 | - | 2 |
| **Total** | **39** | **36** | **75** |

### Coverage by Test Type

| Type | Count | Examples |
|------|-------|----------|
| Schema/Structure | 6 | TC1.1, TC1.3, TC3.3 |
| Functionality | 18 | TC3.1, TC4.1, TC5.1 |
| Access Control | 10 | TC6.1-4, TC7.2, TC8.1 |
| Error Handling | 10 | TC5.3-4, TC9.1-2, TC10.1-2 |
| Authentication | 8 | TC2.1, TC3.5-6, TC8.1-2 |
| Navigation | 12 | TC2.1-4, TC7.1-2, TC10.1-2 |
| Data Integrity | 5 | TC8.1-3, TC11.1-2 |
| UI/UX | 6 | TC1.5-6, TC3.6, TC6.1-3 |

---

## Test Execution Commands

### Backend

```bash
# All tests
npm run test -w backend

# Only projects
npm run test -w backend -- projects.spec.ts

# Specific category
npm run test -w backend -- projects.spec.ts -t "Database Schema"

# Single test
npm run test -w backend -- projects.spec.ts -t "TC1.1"

# With coverage
npm run test -w backend -- --coverage
```

### Frontend

```bash
# All tests
npm run test -w frontend

# Only projects
npx playwright test projects.spec.ts

# Specific category
npx playwright test projects.spec.ts -g "Home Page"

# Single test
npx playwright test projects.spec.ts -g "TC1.1"

# Interactive mode
npx playwright test --ui

# With browser visible
npx playwright test --headed
```

---

## Expected Results

### Passing Criteria
✅ All 51 backend tests pass
✅ All 36+ frontend tests pass
✅ No console errors or warnings
✅ Database migrations apply correctly
✅ Test data seeds successfully
✅ API endpoints return correct status codes
✅ UI renders correctly on desktop and mobile
✅ Navigation works as expected

### Performance Targets
- Backend tests: < 30 seconds total
- Frontend tests: < 60 seconds total
- Page load time: < 2 seconds
- API response time: < 500ms

---

## Related Documentation

- [E2E Test Documentation](./E2E_TESTS.md) - Detailed test descriptions
- [Architecture Documentation](./ARCHITECTURE.md) - System design
- [Project Plan](../.claude/plans/plan0x2.md) - Implementation plan

---

## Test Maintenance Schedule

- **Weekly**: Run full test suite in CI/CD
- **Per PR**: Run affected tests on code changes
- **Monthly**: Review test coverage and add new tests as needed
- **Quarterly**: Performance profiling and optimization

---

*Last Updated: 2026-09-18*
*Total Test Cases: 75*
