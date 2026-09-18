# Project Entity System - End-to-End Test Suite

## Overview

This document describes the comprehensive E2E test suite for the project entity system, including project management, access control, and multi-project navigation.

## Test Execution

### Backend Tests

```bash
# Run all backend tests
npm run test -w backend

# Run only project E2E tests
npm run test -w backend -- projects.spec.ts

# Run with coverage
npm run test -w backend -- --coverage

# Run in watch mode
npm run test:watch -w backend
```

### Frontend Tests

```bash
# Run all frontend E2E tests
npm run test -w frontend

# Run only project E2E tests
npx playwright test projects.spec.ts

# Run specific test
npx playwright test projects.spec.ts -g "TC1.1"

# Run in UI mode (interactive)
npx playwright test --ui

# Run with headed browser (see browser)
npx playwright test --headed
```

## Backend Test Suite: `backend/tests/e2e/projects.spec.ts`

### Test Organization

The test suite is organized into 11 test categories covering all aspects of the project entity system:

#### 1. Database Schema Tests (TC1.x)
Verifies database structure and constraints are properly enforced.

- **TC1.1**: Projects table exists with correct schema
  - Validates columns: id, key, name, created_at, updated_at
  
- **TC1.2**: Project key has unique constraint
  - Attempts to create duplicate project key, expects error
  
- **TC1.3**: Project_users junction table exists with correct schema
  - Validates columns: id, project_id, user_id, role, created_at
  
- **TC1.4**: Project_users has unique constraint on (project_id, user_id)
  - Prevents duplicate user-project mappings
  
- **TC1.5**: Foreign key constraints exist
  - Ensures referential integrity

#### 2. Authentication Flow Tests (TC2.x)
Sets up authentication for subsequent tests.

- **TC2.1**: Valid login returns token for later use
  - Obtains and stores token for use in project tests

#### 3. GET /api/projects/list Tests (TC3.x)
Tests the project listing endpoint.

- **TC3.1**: List projects returns array of user projects
  - Validates response structure
  
- **TC3.2**: Test user has access to test-project
  - Verifies seeded test project is returned
  
- **TC3.3**: Returned project has key, name, and timestamps
  - Validates required fields
  
- **TC3.4**: Response does not contain internal user IDs or project IDs
  - Ensures API only exposes key and name
  
- **TC3.5**: Missing token returns 401
  - Validates authentication is required
  
- **TC3.6**: Invalid token returns 401
  - Tests invalid token handling

#### 4. GET /api/projects/validate Tests (TC4.x)
Tests the project validation endpoint.

- **TC4.1**: Validate returns true for accessible project
  
- **TC4.2**: Validate response includes project details
  
- **TC4.3**: Validate returns false for nonexistent project
  
- **TC4.4**: Validate returns false for project user lacks access to
  - Access control validation
  
- **TC4.5**: Missing projectKey parameter returns 400
  
- **TC4.6**: Missing token returns 401

#### 5. GET /api/projects/:projectKey/info Tests (TC5.x)
Tests the project info endpoint.

- **TC5.1**: Get project info returns project details
  
- **TC5.2**: Project info does not expose internal ID
  
- **TC5.3**: Nonexistent project returns 404
  
- **TC5.4**: User without access returns 403
  - Critical access control test
  
- **TC5.5**: Missing token returns 401
  
- **TC5.6**: Invalid token returns 401

#### 6. Access Control - activeProject Middleware Tests (TC6.x)
Tests the activeProject middleware logic.

- **TC6.1**: Middleware validates project exists before checking access
  - Error priority: 404 > 403
  
- **TC6.2**: Middleware validates user-project mapping
  - Checks project_users table
  
- **TC6.3**: User with admin role can access project
  - Role-based validation
  
- **TC6.4**: User with member role can access project
  - Validates member role works

#### 7. Multiple Projects Tests (TC7.x)
Tests multi-project scenarios.

- **TC7.1**: User can be part of multiple projects
  - Creates and maps multiple projects
  
- **TC7.2**: List includes only projects user has access to
  - Access isolation
  
- **TC7.3**: User can navigate between projects
  - Tests transitions between projects

#### 8. Project Seeding Tests (TC8.x)
Validates seeded test data.

- **TC8.1**: Test project was created by seeder
  
- **TC8.2**: Test user was mapped to test project
  
- **TC8.3**: Test project is accessible via API

#### 9. Session Isolation Tests (TC9.x)
Tests multi-session behavior.

- **TC9.1**: Different users cannot access each other projects
  - Cross-user access prevention
  
- **TC9.2**: Token from one session works independently
  - Session independence

#### 10. Error Handling Tests (TC10.x)
Tests error scenarios and recovery.

- **TC10.1**: Invalid project key format handled gracefully
  
- **TC10.2**: URL-encoded project keys work correctly
  
- **TC10.3**: Server recovers after errors

#### 11. Data Consistency Tests (TC11.x)
Validates data integrity.

- **TC11.1**: Project key is immutable
  
- **TC11.2**: Project timestamps are set correctly

### Running Individual Test Groups

```bash
# Run only schema tests
npm run test -w backend -- projects.spec.ts -t "Database Schema"

# Run only access control tests
npm run test -w backend -- projects.spec.ts -t "Access Control"

# Run only multiple projects tests
npm run test -w backend -- projects.spec.ts -t "Multiple Projects"
```

## Frontend Test Suite: `frontend/tests/e2e/projects.spec.ts`

### Test Organization

The test suite is organized into 10 test categories covering UI, navigation, and integration:

#### 1. Home Page - Project List Tests (TC1.x)
Tests the home page and project list display.

- **TC1.1**: Home page displays projects list
  
- **TC1.2**: Test project appears in project list
  
- **TC1.3**: Project card shows key and name
  
- **TC1.4**: Project list is grid layout
  
- **TC1.5**: Loading state appears while fetching projects
  - Validates UX feedback
  
- **TC1.6**: Error handling when projects fail to load
  - Validates error states
  
- **TC1.7**: Header shows app title and logout button
  
- **TC1.8**: Welcome message shows username

#### 2. Project Navigation Tests (TC2.x)
Tests navigation between pages.

- **TC2.1**: Clicking project card navigates to project dashboard
  
- **TC2.2**: Project dashboard loads project info
  
- **TC2.3**: Breadcrumb shows navigation path
  
- **TC2.4**: Breadcrumb Home link navigates back to home

#### 3. Project Dashboard Tests (TC3.x)
Tests the project detail page.

- **TC3.1**: Project dashboard displays project details
  
- **TC3.2**: Dashboard shows project key in monospace
  
- **TC3.3**: Dashboard shows creation date
  
- **TC3.4**: Dashboard shows info card styling
  
- **TC3.5**: Dashboard shows placeholder for future content
  
- **TC3.6**: Loading state appears while fetching project

#### 4. Access Control Tests (TC4.x)
Tests access control error handling.

- **TC4.1**: Accessing non-existent project shows 404 message
  
- **TC4.2**: Accessing unauthorized project shows 403 message
  
- **TC4.3**: Error message has go back button
  
- **TC4.4**: Go back button navigates to home

#### 5. Header and Navigation Tests (TC5.x)
Tests header and global navigation.

- **TC5.1**: Header is visible on all pages
  
- **TC5.2**: Logout button works from home page
  
- **TC5.3**: Logout button works from project page
  
- **TC5.4**: App title is visible

#### 6. Responsive Design Tests (TC6.x)
Tests mobile and small-screen layouts.

- **TC6.1**: Home page is responsive on mobile
  - Tests 375x667 viewport
  
- **TC6.2**: Project page is responsive on mobile
  
- **TC6.3**: Breadcrumb is visible on small screens

#### 7. Multiple Projects Navigation Tests (TC7.x)
Tests navigation between multiple projects.

- **TC7.1**: User can navigate between multiple projects
  
- **TC7.2**: Project key is correctly passed in URL

#### 8. Authentication Integration Tests (TC8.x)
Tests authentication-related frontend logic.

- **TC8.1**: Unauthorized access redirects to login
  
- **TC8.2**: Token validation happens on page load

#### 9. Error States Tests (TC9.x)
Tests error handling and recovery.

- **TC9.1**: Graceful error handling on API failure
  
- **TC9.2**: App recovers after navigation from error

#### 10. Browser History Tests (TC10.x)
Tests browser navigation and history.

- **TC10.1**: Browser back button works correctly
  
- **TC10.2**: Direct URL navigation works

### Running Individual Test Groups

```bash
# Run only home page tests
npx playwright test projects.spec.ts -g "Home Page"

# Run only navigation tests
npx playwright test projects.spec.ts -g "Project Navigation"

# Run only access control tests
npx playwright test projects.spec.ts -g "Access Control"
```

## Test Coverage Summary

### Backend Coverage
- **Database Layer**: Schema, constraints, foreign keys
- **API Endpoints**: All 3 project endpoints (list, validate, info)
- **Middleware**: activeLogin, activeProject
- **Access Control**: User-project mapping, role validation
- **Error Handling**: Invalid input, authorization, data integrity
- **Data Consistency**: Seeded data, timestamps, immutable fields

### Frontend Coverage
- **Components**: Home page, ProjectDashboard, Breadcrumb
- **Routes**: /, /project/:projectKey, error states
- **Navigation**: Project list navigation, breadcrumb links, browser history
- **UI States**: Loading, error, success
- **User Interactions**: Clicking, navigation
- **API Integration**: Token passing, error handling
- **Responsive Design**: Mobile viewports

## Test Data Setup

### Seeded Data
- **Test User**: username=`testuser`, password=`password123`, id=1
- **Test Project**: key=`test-project`, name=`Test Project`
- **Mapping**: testuser → test-project (role: admin)

### Dynamic Test Data
Tests create additional projects and mappings as needed:
- Projects with specific roles (admin, member, viewer)
- Multiple projects for the same user
- Projects without user access (for access control tests)

## Expected Results Summary

### Backend Tests
- **Total Tests**: 51
- **Categories**: 11
- **All tests should PASS** when:
  - Database is running (PostgreSQL)
  - Migrations are applied
  - Test seeder has run
  - JWT_SECRET is set in .env

### Frontend Tests
- **Total Tests**: 40+
- **Categories**: 10
- **All tests should PASS** when:
  - Backend is running on `http://localhost:3000`
  - Frontend is running on `http://localhost:5173`
  - Test user is seeded and can login
  - Playwright browsers are installed

## Debugging Tests

### Backend Test Debugging

```bash
# Run with debug output
npm run test -w backend -- projects.spec.ts --verbose

# Run single test
npm run test -w backend -- projects.spec.ts -t "TC3.1"

# Run with coverage report
npm run test -w backend -- projects.spec.ts --coverage --collectCoverageFrom='src/**/*.ts'
```

### Frontend Test Debugging

```bash
# Run in debug mode
npx playwright test projects.spec.ts --debug

# Run with logging
npx playwright test projects.spec.ts --trace on

# View trace after run
npx playwright show-trace trace/trace.zip

# Run single test
npx playwright test projects.spec.ts -g "TC1.1"

# Run with headed browser to see what's happening
npx playwright test projects.spec.ts --headed
```

## Known Limitations

1. **Multi-user Testing**: Backend tests assume single test user. Multi-user scenarios are mocked.
2. **Performance Tests**: Current suite doesn't include load/performance testing.
3. **Concurrent Access**: Tests run sequentially; concurrent modification scenarios not tested.
4. **Edge Cases**: Some boundary conditions may not be covered.

## Future Enhancements

1. Add tests for project creation/update/delete endpoints (when implemented)
2. Add performance/load testing
3. Add visual regression testing for UI
4. Add accessibility (a11y) testing
5. Add E2E tests for multi-user scenarios
6. Add tests for project roles (admin/member/viewer permissions)

## CI/CD Integration

### GitHub Actions Example

```yaml
- name: Run backend tests
  run: npm run test -w backend

- name: Run frontend tests
  run: npx playwright install && npx playwright test -w frontend
```

## Maintenance

### Adding New Tests

1. Follow the naming convention: `TCX.Y` where X is category, Y is test number
2. Group related tests in describe blocks
3. Add to appropriate section in this document
4. Run full suite to ensure no regressions

### Updating Tests

When modifying project API:
1. Update affected test cases
2. Add new test cases for new functionality
3. Update this documentation
4. Run full suite to validate changes

## Contact & Support

For issues or questions about the test suite, refer to the project documentation or contact the development team.
