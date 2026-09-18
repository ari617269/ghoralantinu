const API_BASE = ((import.meta as unknown) as Record<string, unknown>).env?.VITE_API_URL || 'http://localhost:3000/api';

export interface Project {
  key: string;
  name: string;
  created_at: string;
  updated_at: string;
}

export async function getProjectsList(token: string): Promise<Project[]> {
  const response = await fetch(`${API_BASE}/projects/list`, {
    method: 'GET',
    headers: {
      Authorization: `Bearer ${token}`,
      'Content-Type': 'application/json',
    },
  });

  if (!response.ok) {
    throw new Error(`Failed to fetch projects: ${response.statusText}`);
  }

  const data = await response.json();
  return data.projects;
}

export async function validateProject(
  token: string,
  projectKey: string
): Promise<{ valid: boolean; project?: Project }> {
  const response = await fetch(`${API_BASE}/projects/validate?projectKey=${encodeURIComponent(projectKey)}`, {
    method: 'GET',
    headers: {
      Authorization: `Bearer ${token}`,
      'Content-Type': 'application/json',
    },
  });

  if (!response.ok) {
    throw new Error(`Failed to validate project: ${response.statusText}`);
  }

  return response.json();
}

export async function getProjectInfo(token: string, projectKey: string): Promise<Project> {
  const response = await fetch(`${API_BASE}/projects/${encodeURIComponent(projectKey)}/info`, {
    method: 'GET',
    headers: {
      Authorization: `Bearer ${token}`,
      'Content-Type': 'application/json',
    },
  });

  if (!response.ok) {
    throw new Error(`Failed to fetch project info: ${response.statusText}`);
  }

  const data = await response.json();
  return data.project;
}
