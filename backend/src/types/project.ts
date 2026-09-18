export interface Project {
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
