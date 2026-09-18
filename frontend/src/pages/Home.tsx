import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAppDispatch, useAppSelector } from '../store/index';
import { clearCredentials } from '../store/authSlice';
import { validateTokenApi, logoutApi } from '../api/auth';

export default function Home() {
  const dispatch = useAppDispatch();
  const navigate = useNavigate();
  const { token, user } = useAppSelector((state) => state.auth);
  const [isLoggingOut, setIsLoggingOut] = useState(false);

  useEffect(() => {
    const validateToken = async () => {
      if (!token) {
        navigate('/login', { replace: true });
        return;
      }

      try {
        await validateTokenApi(token);
      } catch {
        dispatch(clearCredentials());
        navigate('/login', { replace: true });
      }
    };

    validateToken();
  }, [token, navigate, dispatch]);

  const handleLogout = async () => {
    if (!token) return;

    setIsLoggingOut(true);
    try {
      await logoutApi(token);
    } catch (error) {
      console.error('Logout error:', error);
    }

    dispatch(clearCredentials());
    navigate('/login', { replace: true });
  };

  return (
    <div
      style={{
        minHeight: '100vh',
        backgroundColor: 'var(--color-bg)',
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      <header
        style={{
          backgroundColor: 'var(--color-surface)',
          borderBottom: '1px solid var(--color-border)',
          padding: '16px 24px',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
        }}
      >
        <h1
          style={{
            margin: 0,
            fontSize: '20px',
            fontWeight: '600',
            color: 'var(--color-text)',
          }}
        >
          ghoralantinu
        </h1>
        <button
          onClick={handleLogout}
          disabled={isLoggingOut}
          style={{
            padding: '8px 16px',
            backgroundColor: 'var(--color-accent)',
            color: 'white',
            border: 'none',
            borderRadius: '2px',
            fontSize: '14px',
            fontWeight: '500',
            cursor: isLoggingOut ? 'not-allowed' : 'pointer',
            opacity: isLoggingOut ? 0.6 : 1,
            fontFamily: 'inherit',
          }}
        >
          {isLoggingOut ? 'Logging out...' : 'Logout'}
        </button>
      </header>

      <main
        style={{
          flex: 1,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          padding: '40px 24px',
        }}
      >
        <div style={{ textAlign: 'center' }}>
          <h2
            style={{
              fontSize: '32px',
              fontWeight: '600',
              marginBottom: '12px',
              color: 'var(--color-text)',
            }}
          >
            Welcome, {user?.username}!
          </h2>
          <p
            style={{
              fontSize: '16px',
              color: 'var(--color-muted)',
              margin: 0,
            }}
          >
            Self-hosted data storage tool
          </p>
        </div>
      </main>
    </div>
  );
}
