import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { Project } from '../api/projects';

interface ProjectState {
  projects: Project[] | null;
  activeProject: Project | null;
  selectedProjectKey: string | null;
  loading: boolean;
  error: string | null;
}

const initialState: ProjectState = {
  projects: null,
  activeProject: null,
  selectedProjectKey: null,
  loading: false,
  error: null,
};

const projectSlice = createSlice({
  name: 'project',
  initialState,
  reducers: {
    setProjects: (state, action: PayloadAction<Project[]>) => {
      state.projects = action.payload;
    },
    setActiveProject: (state, action: PayloadAction<Project>) => {
      state.activeProject = action.payload;
    },
    setSelectedProjectKey: (state, action: PayloadAction<string>) => {
      state.selectedProjectKey = action.payload;
    },
    setLoading: (state, action: PayloadAction<boolean>) => {
      state.loading = action.payload;
    },
    setError: (state, action: PayloadAction<string | null>) => {
      state.error = action.payload;
    },
    clearProject: (state) => {
      state.activeProject = null;
      state.selectedProjectKey = null;
      state.error = null;
    },
  },
});

export const {
  setProjects,
  setActiveProject,
  setSelectedProjectKey,
  setLoading,
  setError,
  clearProject,
} = projectSlice.actions;

export default projectSlice.reducer;
