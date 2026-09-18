declare global {
  namespace Express {
    interface Request {
      user?: {
        id: number;
        username: string;
      };
      project?: {
        key: string;
        name: string;
        created_at?: Date;
        updated_at?: Date;
      };
    }
  }
}

export {};
