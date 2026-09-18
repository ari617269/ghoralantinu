import { Router, Request, Response } from 'express';
import db from '../db/knex';
import { activeLogin } from '../middleware/activeLogin';
import { activeProject } from '../middleware/activeProject';

const router = Router();

router.get('/list', activeLogin, async (req: Request, res: Response): Promise<void> => {
  try {
    const projects = await db('projects')
      .innerJoin('project_users', 'projects.id', 'project_users.project_id')
      .where('project_users.user_id', req.user!.id)
      .select('projects.key', 'projects.name', 'projects.created_at', 'projects.updated_at');

    res.json({ projects });
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
});

router.get('/validate', activeLogin, async (req: Request, res: Response): Promise<void> => {
  try {
    const projectKey = req.query.projectKey as string;

    if (!projectKey) {
      res.status(400).json({ error: 'Project key is required' });
      return;
    }

    const project = await db('projects').where({ key: projectKey }).first();
    if (!project) {
      res.json({ valid: false });
      return;
    }

    const projectUser = await db('project_users')
      .where({ project_id: project.id, user_id: req.user!.id })
      .first();

    if (!projectUser) {
      res.json({ valid: false });
      return;
    }

    res.json({
      valid: true,
      project: {
        key: project.key,
        name: project.name,
      },
    });
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
});

router.get('/:projectKey/info', activeLogin, activeProject, async (req: Request, res: Response): Promise<void> => {
  try {
    res.json({ project: req.project });
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
});

export default router;
