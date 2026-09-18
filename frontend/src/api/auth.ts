import axios from 'axios';

const api = axios.create({
  baseURL: '/api',
});

interface LoginResponse {
  token: string;
  user: {
    id: number;
    username: string;
  };
}

interface ValidResponse {
  valid: boolean;
  user: {
    id: number;
    username: string;
  };
}

export const loginApi = async (username: string, password: string): Promise<LoginResponse> => {
  const response = await api.post('/user/login', { username, password });
  return response.data;
};

export const logoutApi = async (token: string): Promise<void> => {
  await api.post('/user/logout', {}, {
    headers: {
      Authorization: `Bearer ${token}`,
    },
  });
};

export const validateTokenApi = async (token: string): Promise<ValidResponse> => {
  const response = await api.get('/user/valid', {
    headers: {
      Authorization: `Bearer ${token}`,
    },
  });
  return response.data;
};
