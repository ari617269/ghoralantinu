# Plan: Project Entity and Access Control System

## Overview
Implement a project entity system with multi-project support, user-project mapping, and project-based access control middleware. This enables users to manage multiple projects with appropriate access boundaries.

## Key Entities

### Database Schema

#### 1. `projects` table
```
- id (primary key)
- key (string, unique, immutable) - URL-safe identifier for projects
- name (string) - Display name
- created_at (timestamp)
- updated_at (timestamp)
```

#### 2. `project_users` table (Junction Table)
```
- id (primary key)
- project_id (foreign key → projects.id)
- user_id (foreign key → users.id)
- role (string) - Future: 'admin', 'member', 'viewer'
- created_at (timestamp)
- unique(project_id, user_id) - Prevent duplicate mappings
```

## Backend Implementation

### Step 1: Create Database Migrations
**File**: `backend/src/db/migrations/003_projects.ts`

Create `projects` table with:
- id, key (unique), name, created_at, updated_at
- Ensure key is URL-safe (alphanumeric + dash/underscore)

Create `project_users` table with:
- id, project_id, user_id, role, created_at
- Foreign key constraints to users and projects
- Unique constraint on (project_id, user_id)

### Step 2: Create Seeder
**File**: `backend/src/db/seeds/002_test_project.ts`

Seed data:
- Create test project with key: "test-project", name: "Test Project"
- Map to existing test user via project_users junction table
- Role: "admin" (for full access)

### Step 3: Create Type Definitions
**File**: `backend/src/types/project.ts`

```typescript
export interface Project {
  id: number;
  key: string;
  name: string;
  created_at: Date;
  updated_at: Date;
}

export interface ProjectUser {
  project_id: number;
  user_id: number;
  role: 'admin' | 'member' | 'viewer';
}
```

Extend `backend/src/types/express.d.ts` to include active project:
```typescript
req.project = { id: number; key: string; name: string; }
```

### Step 4: Create activeProject Middleware
**File**: `backend/src/middleware/activeProject.ts`

- Extract project key from route parameter (`:projectKey`)
- Query projects table to get project by key
- Check if req.user is in project_users for that project
- If not, return 403 Forbidden
- If yes, attach project to req.project and call next()
- Handle missing project (404), unauthorized user (403), other errors (500)

### Step 5: Create Project Routes
**File**: `backend/src/routes/project.ts`

#### `GET /api/projects/list`
- Protected by `activeLogin` middleware
- Query project_users where user_id = req.user.id
- Join with projects table to get project details
- Return array of projects: `[{ id, key, name, created_at }]`

#### `GET /api/projects/validate` (or `/api/projects/:projectKey/validate`)
- Protected by `activeLogin` middleware
- Check if user has access to project via project_users
- Verify project exists
- Return `{ valid: true, project: { id, key, name } }`

#### `GET /api/projects/:projectKey/info`
- Protected by `activeLogin` + `activeProject` middleware
- Return current project from req.project
- Return `{ project: { id, key, name, created_at } }`

### Step 6: Register Routes and Middleware
**File**: `backend/src/index.ts`

- Import and use projectRouter before error handler
- Middleware stack:
  ```
  /api/projects/list → activeLogin
  /api/projects/validate → activeLogin
  /api/projects/:projectKey/* → activeLogin → activeProject
  ```

## Frontend Implementation

### Step 1: Create API Service
**File**: `frontend/src/api/projects.ts`

- `getProjectsList(token: string)` - Fetch projects from `/api/projects/list`
- `validateProject(token: string, projectKey: string)` - Check project access
- `getProjectInfo(token: string, projectKey: string)` - Fetch project details

### Step 2: Create Redux Slice for Projects
**File**: `frontend/src/store/projectSlice.ts`

State shape:
```typescript
{
  projects: Project[] | null;
  activeProject: Project | null;
  selectedProjectKey: string | null;
  loading: boolean;
  error: string | null;
}
```

Actions:
- `setProjects(projects)`
- `setActiveProject(project)`
- `setSelectedProjectKey(key)`
- `setLoading(boolean)`
- `setError(string | null)`

### Step 3: Update Frontend App Router
**File**: `frontend/src/App.tsx`

Route structure:
```
/login → Login page
/ → Home page (projects list)
/project/:projectKey → Project dashboard
```

Add route parameter for `:projectKey` and pass to project page.

### Step 4: Create Home Page
**File**: `frontend/src/pages/Home.tsx`

- Load projects list on mount using Redux action
- Display loading state
- Show error if list fetch fails
- Render projects as clickable cards/list
- Each card shows: project name, key
- Navigate to `/project/:projectKey` on click
- Keep header with logout button

### Step 5: Create Project Dashboard Page
**File**: `frontend/src/pages/ProjectDashboard.tsx`

- Extract `projectKey` from URL params
- Load project info on mount via API
- Update Redux activeProject
- Display breadcrumbs: `Home > Project Name`
- Display project info: key, name, created_at
- Placeholder for dashboard content
- Error handling if project not found (403) or doesn't exist (404)
- Redirect to home if user lacks access

### Step 6: Create Breadcrumb Component
**File**: `frontend/src/components/Breadcrumb.tsx`

- Display navigation hierarchy
- Links to parent routes
- Current page highlighted (not clickable)
- Example: `Home > Test Project`

### Step 7: Update App Router
**File**: `frontend/src/App.tsx`

```typescript
<Route path="/" element={<Home />} />
<Route path="/project/:projectKey" element={<ProjectDashboard />} />
```

## Testing Checklist

### Backend
- [ ] Migrations run without errors
- [ ] Seeder creates test project and project_users mapping
- [ ] `activeProject` middleware blocks unauthorized users (403)
- [ ] `activeProject` middleware blocks invalid projects (404)
- [ ] `/api/projects/list` returns user's projects only
- [ ] `/api/projects/validate` correctly validates access
- [ ] `/api/projects/:projectKey/info` returns correct project info

### Frontend
- [ ] Home page loads and displays projects list
- [ ] Clicking project navigates to dashboard
- [ ] Project dashboard loads with correct project info
- [ ] Breadcrumbs render correctly
- [ ] Logout works from both pages
- [ ] Unauthorized access redirected to home
- [ ] Project key in URL matches selected project

## Data Flow

### List Projects
```
User clicks Home → useEffect → API.getProjectsList(token)
→ Backend: /api/projects/list (activeLogin)
→ Query project_users + projects join
→ Return projects array
→ Frontend: dispatch setProjects(), render cards
```

### Navigate to Project
```
User clicks project card → useNavigate(`/project/${key}`)
→ ProjectDashboard mounts with projectKey param
→ useEffect → API.getProjectInfo(token, key)
→ Backend: /api/projects/:projectKey/info (activeLogin → activeProject)
→ activeProject validates access, returns 403 if unauthorized
→ Frontend: display breadcrumb + project info
```

## File Structure Summary

### Backend
```
backend/src/
├── db/
│   ├── migrations/
│   │   └── 003_projects.ts (NEW)
│   └── seeds/
│       └── 002_test_project.ts (NEW)
├── middleware/
│   ├── activeLogin.ts (existing)
│   └── activeProject.ts (NEW)
├── routes/
│   ├── user.ts (existing)
│   └── project.ts (NEW)
├── types/
│   ├── express.d.ts (UPDATE)
│   └── project.ts (NEW)
└── index.ts (UPDATE)
```

### Frontend
```
frontend/src/
├── api/
│   ├── auth.ts (existing)
│   └── projects.ts (NEW)
├── pages/
│   ├── Home.tsx (UPDATE)
│   ├── Login.tsx (existing)
│   └── ProjectDashboard.tsx (NEW)
├── components/
│   ├── ProtectedRoute.tsx (existing)
│   └── Breadcrumb.tsx (NEW)
├── store/
│   ├── index.ts (existing)
│   ├── authSlice.ts (existing)
│   └── projectSlice.ts (NEW)
└── App.tsx (UPDATE)
```

## Implementation Order

1. **Backend Database** (3_projects migration + 2_test_project seed)
2. **Backend Types** (project.ts + express.d.ts update)
3. **Backend Middleware** (activeProject)
4. **Backend Routes** (project.ts router)
5. **Backend Integration** (index.ts)
6. **Frontend Types & API** (projects.ts)
7. **Frontend Redux** (projectSlice.ts)
8. **Frontend Components** (Breadcrumb.tsx)
9. **Frontend Pages** (Home.tsx update + ProjectDashboard.tsx)
10. **Frontend Router** (App.tsx update)
11. **Testing** (manual e2e + existing test suite)

## Success Criteria

✓ Database migrations apply successfully  
✓ Test project exists with test user mapped  
✓ Home page shows "Test Project" in projects list  
✓ Clicking project navigates to dashboard with breadcrumb + info  
✓ Unauthorized users get 403 on project endpoints  
✓ Project key is URL-safe and immutable  
✓ All endpoints properly protected by activeLogin middleware  
