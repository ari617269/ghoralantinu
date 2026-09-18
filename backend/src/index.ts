/// <reference path="./types/express.d.ts" />
import 'dotenv/config';
import express from 'express';
import db from './db/knex';
import userRouter from './routes/user';
import projectRouter from './routes/project';

const app = express();
const PORT = process.env.API_PORT || 3000;

app.use(express.json());

app.get('/health', (req, res) => {
  res.json({ status: 'ok' });
});

app.use('/api/user', userRouter);
app.use('/api/projects', projectRouter);

app.use((err: Error, req: express.Request, res: express.Response) => {
  res.status(500).json({ error: 'Internal server error' });
});

(async () => {
  try {
    // Migrations already run separately
    // await db.migrate.latest();
    app.listen(PORT, () => {
      console.log(`Server running on port ${PORT}`);
    });
  } catch (error) {
    console.error('Migration failed:', error);
    process.exit(1);
  }
})();
