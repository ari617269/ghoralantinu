import { Request, Response, NextFunction } from 'express';
import jwt from 'jsonwebtoken';
import db from '../db/knex';

export async function activeLogin(
  req: Request,
  res: Response,
  next: NextFunction
): Promise<void> {
  try {
    const authHeader = req.headers.authorization;

    if (!authHeader || !authHeader.startsWith('Bearer ')) {
      res.status(401).json({ error: 'Missing or invalid Authorization header' });
      return;
    }

    const token = authHeader.slice(7);

    let decoded: jwt.JwtPayload;
    try {
      decoded = jwt.verify(token, process.env.JWT_SECRET!) as jwt.JwtPayload;
    } catch {
      res.status(401).json({ error: 'Invalid or expired token' });
      return;
    }

    const blocklisted = await db('expired_tokens').where({ token }).first();
    if (blocklisted) {
      res.status(401).json({ error: 'Token has been revoked' });
      return;
    }

    req.user = {
      id: decoded.id as number,
      username: decoded.username as string,
    };
    next();
  } catch (error) {
    res.status(500).json({ error: 'Internal server error' });
  }
}
