import { Router, Request, Response } from 'express';
import jwt from 'jsonwebtoken';
import bcrypt from 'bcryptjs';
import db from '../db/knex';
import { activeLogin } from '../middleware/activeLogin';

const router = Router();

interface LoginBody {
  username: string;
  password: string;
}

router.post('/login', async (req: Request<{}, {}, LoginBody>, res: Response): Promise<void> => {
  try {
    const { username, password } = req.body;

    if (!username || !password) {
      res.status(400).json({ error: 'Username and password are required' });
      return;
    }

    const user = await db('users').where({ username }).first();

    if (!user) {
      res.status(401).json({ error: 'Invalid credentials' });
      return;
    }

    const passwordValid = await bcrypt.compare(password, user.password_hash);
    if (!passwordValid) {
      res.status(401).json({ error: 'Invalid credentials' });
      return;
    }

    const token = jwt.sign({ id: user.id, username: user.username }, process.env.JWT_SECRET!, {
      expiresIn: '1h',
    });

    res.json({ token, user: { id: user.id, username: user.username } });
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
});

router.post('/logout', activeLogin, async (req: Request, res: Response): Promise<void> => {
  try {
    const token = req.headers.authorization!.slice(7);
    const decoded = jwt.decode(token) as jwt.JwtPayload;

    await db('expired_tokens').insert({
      token,
      expired_at: new Date((decoded.exp!) * 1000),
    });

    res.json({ message: 'Logged out successfully' });
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
});

router.get('/valid', activeLogin, async (req: Request, res: Response): Promise<void> => {
  res.json({ valid: true, user: req.user });
});

export default router;
