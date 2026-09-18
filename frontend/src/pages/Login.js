import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAppDispatch, useAppSelector } from '../store/index';
import { setCredentials } from '../store/authSlice';
import { loginApi } from '../api/auth';
export default function Login() {
    const [username, setUsername] = useState('');
    const [password, setPassword] = useState('');
    const [error, setError] = useState('');
    const [isLoading, setIsLoading] = useState(false);
    const navigate = useNavigate();
    const dispatch = useAppDispatch();
    const isAuthenticated = useAppSelector((state) => state.auth.isAuthenticated);
    useEffect(() => {
        if (isAuthenticated) {
            navigate('/', { replace: true });
        }
    }, [isAuthenticated, navigate]);
    const handleSubmit = async (e) => {
        e.preventDefault();
        setError('');
        setIsLoading(true);
        try {
            const response = await loginApi(username, password);
            dispatch(setCredentials({ token: response.token, user: response.user }));
            navigate('/', { replace: true });
        }
        catch {
            setError('Invalid username or password');
        }
        finally {
            setIsLoading(false);
        }
    };
    return (_jsx("div", { style: {
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            minHeight: '100vh',
            backgroundColor: 'var(--color-bg)',
        }, children: _jsxs("div", { style: {
                backgroundColor: 'white',
                border: '1px solid var(--color-border)',
                borderRadius: '2px',
                padding: '40px',
                width: '100%',
                maxWidth: '360px',
                boxSizing: 'border-box',
            }, children: [_jsx("h1", { style: {
                        fontSize: '28px',
                        fontWeight: '600',
                        marginBottom: '32px',
                        color: 'var(--color-text)',
                        textAlign: 'center',
                    }, children: "Login" }), _jsxs("form", { onSubmit: handleSubmit, children: [_jsx("label", { htmlFor: "username", style: {
                                display: 'block',
                                marginBottom: '8px',
                                fontSize: '14px',
                                fontWeight: '500',
                                color: 'var(--color-text)',
                            }, children: "Username" }), _jsx("input", { id: "username", type: "text", value: username, onChange: (e) => setUsername(e.target.value), placeholder: "testuser", style: {
                                width: '100%',
                                padding: '10px 12px',
                                marginBottom: '16px',
                                border: '1px solid var(--color-border)',
                                borderRadius: '2px',
                                fontSize: '14px',
                                backgroundColor: 'white',
                                boxSizing: 'border-box',
                                fontFamily: 'inherit',
                            }, disabled: isLoading }), _jsx("label", { htmlFor: "password", style: {
                                display: 'block',
                                marginBottom: '8px',
                                fontSize: '14px',
                                fontWeight: '500',
                                color: 'var(--color-text)',
                            }, children: "Password" }), _jsx("input", { id: "password", type: "password", value: password, onChange: (e) => setPassword(e.target.value), placeholder: "password123", style: {
                                width: '100%',
                                padding: '10px 12px',
                                marginBottom: '24px',
                                border: '1px solid var(--color-border)',
                                borderRadius: '2px',
                                fontSize: '14px',
                                backgroundColor: 'white',
                                boxSizing: 'border-box',
                                fontFamily: 'inherit',
                            }, disabled: isLoading }), error && (_jsx("div", { style: {
                                marginBottom: '16px',
                                padding: '12px',
                                backgroundColor: '#fee',
                                border: '1px solid #fcc',
                                borderRadius: '2px',
                                color: 'var(--color-error)',
                                fontSize: '14px',
                            }, children: error })), _jsx("button", { type: "submit", disabled: isLoading, style: {
                                width: '100%',
                                padding: '10px 16px',
                                backgroundColor: 'var(--color-accent)',
                                color: 'white',
                                border: 'none',
                                borderRadius: '2px',
                                fontSize: '14px',
                                fontWeight: '500',
                                cursor: isLoading ? 'not-allowed' : 'pointer',
                                opacity: isLoading ? 0.6 : 1,
                                fontFamily: 'inherit',
                            }, children: isLoading ? 'Logging in...' : 'Login' })] })] }) }));
}
