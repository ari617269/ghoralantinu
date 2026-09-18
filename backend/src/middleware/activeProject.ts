import { Request, Response, NextFunction } from 'express';
import db from '../db/knex';

export async function activeProject(
  req: Request,
  res: Response,
  next: NextFunction
): Promise<void> {
  try {
    if (!req.user) {
      res.status(401).json({ error: 'User not authenticated' });
      return;
    }

    const projectKey = req.params.projectKey;
    if (!projectKey) {
      res.status(400).json({ error: 'Project key is required' });
      return;
    }

    const project = await db('projects').where({ key: projectKey }).first();
    if (!project) {
      res.status(404).json({ error: 'Project not found' });
      return;
    }

    const projectUser = await db('project_users')
      .where({ project_id: project.id, user_id: req.user.id })
      .first();

    if (!projectUser) {
      res.status(403).json({ error: 'Access denied to this project' });
      return;
    }

    req.project = {
      key: project.key,
      name: project.name,
      created_at: project.created_at,
      updated_at: project.updated_at,
    };

    next();
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
}
