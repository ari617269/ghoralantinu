import 'dotenv/config';
import express from 'express';
import db from './db/knex';
import userRouter from './routes/user';

const app = express();
const PORT = process.env.API_PORT || 3000;

app.use(express.json());

app.get('/health', (req, res) => {
  res.json({ status: 'ok' });
});

app.use('/api/user', userRouter);

app.use((err: Error, req: express.Request, res: express.Response) => {
  res.status(500).json({ error: 'Internal server error' });
});

(async () => {
  try {
    await db.migrate.latest();
    app.listen(PORT, () => {
      console.log(`Server running on port ${PORT}`);
    });
  } catch (error) {
    console.error('Migration failed:', error);
    process.exit(1);
  }
})();
