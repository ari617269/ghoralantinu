import axios from 'axios';
const api = axios.create({
    baseURL: '/api',
});
export const loginApi = async (username, password) => {
    const response = await api.post('/user/login', { username, password });
    return response.data;
};
export const logoutApi = async (token) => {
    await api.post('/user/logout', {}, {
        headers: {
            Authorization: `Bearer ${token}`,
        },
    });
};
export const validateTokenApi = async (token) => {
    const response = await api.get('/user/valid', {
        headers: {
            Authorization: `Bearer ${token}`,
        },
    });
    return response.data;
};
