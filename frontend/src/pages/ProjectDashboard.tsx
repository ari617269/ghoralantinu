import { useEffect, useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { useAppDispatch, useAppSelector } from '../store/index';
import { clearCredentials } from '../store/authSlice';
import { setActiveProject, setError, setLoading, clearProject } from '../store/projectSlice';
import { validateTokenApi, logoutApi } from '../api/auth';
import { getProjectInfo } from '../api/projects';
import Breadcrumb from '../components/Breadcrumb';

export default function ProjectDashboard() {
  const { projectKey } = useParams<{ projectKey: string }>();
  const dispatch = useAppDispatch();
  const navigate = useNavigate();
  const { token } = useAppSelector((state) => state.auth);
  const { activeProject, loading, error } = useAppSelector((state) => state.project);
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

  useEffect(() => {
    const fetchProject = async () => {
      if (!token || !projectKey) return;

      dispatch(setLoading(true));
      dispatch(setError(null));

      try {
        const project = await getProjectInfo(token, projectKey);
        dispatch(setActiveProject(project));
      } catch (err) {
        const errMessage = err instanceof Error ? err.message : 'Failed to fetch project';
        dispatch(setError(errMessage));

        if (errMessage.includes('403')) {
          setTimeout(() => navigate('/'), 1000);
        }
      } finally {
        dispatch(setLoading(false));
      }
    };

    fetchProject();

    return () => {
      dispatch(clearProject());
    };
  }, [token, projectKey, dispatch, navigate]);

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

  if (error) {
    return (
      <div
        style={{
          minHeight: '100vh',
          backgroundColor: 'var(--color-bg)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          padding: '20px',
        }}
      >
        <div
          style={{
            backgroundColor: 'var(--color-surface)',
            border: '1px solid var(--color-border)',
            borderRadius: '4px',
            padding: '24px',
            maxWidth: '400px',
            textAlign: 'center',
          }}
        >
          <h2 style={{ color: 'var(--color-accent)', marginTop: 0 }}>Access Denied</h2>
          <p style={{ color: 'var(--color-muted)' }}>{error}</p>
          <button
            onClick={() => navigate('/')}
            style={{
              padding: '8px 16px',
              backgroundColor: 'var(--color-accent)',
              color: 'white',
              border: 'none',
              borderRadius: '2px',
              cursor: 'pointer',
              fontFamily: 'inherit',
              fontSize: '14px',
              fontWeight: '500',
            }}
          >
            Go back to projects
          </button>
        </div>
      </div>
    );
  }

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

      <Breadcrumb
        items={[
          { label: 'Home', href: '/' },
          { label: activeProject?.name || 'Project' },
        ]}
      />

      <main style={{ flex: 1, padding: '24px' }}>
        {loading ? (
          <div style={{ color: 'var(--color-muted)' }}>
            <p>Loading project...</p>
          </div>
        ) : activeProject ? (
          <div style={{ maxWidth: '800px' }}>
            <div
              style={{
                backgroundColor: 'var(--color-surface)',
                border: '1px solid var(--color-border)',
                borderRadius: '4px',
                padding: '24px',
              }}
            >
              <h2
                style={{
                  margin: '0 0 16px 0',
                  fontSize: '24px',
                  fontWeight: '600',
                  color: 'var(--color-text)',
                }}
              >
                {activeProject.name}
              </h2>

              <div style={{ marginBottom: '16px' }}>
                <label style={{ color: 'var(--color-muted)', fontSize: '12px', textTransform: 'uppercase' }}>
                  Project Key
                </label>
                <p
                  style={{
                    margin: '4px 0 0 0',
                    fontSize: '14px',
                    color: 'var(--color-text)',
                    fontFamily: 'monospace',
                  }}
                >
                  {activeProject.key}
                </p>
              </div>

              <div style={{ marginBottom: '16px' }}>
                <label style={{ color: 'var(--color-muted)', fontSize: '12px', textTransform: 'uppercase' }}>
                  Created
                </label>
                <p style={{ margin: '4px 0 0 0', fontSize: '14px', color: 'var(--color-text)' }}>
                  {new Date(activeProject.created_at).toLocaleDateString()}
                </p>
              </div>

              <div
                style={{
                  marginTop: '24px',
                  padding: '16px',
                  backgroundColor: 'var(--color-bg)',
                  borderRadius: '4px',
                  color: 'var(--color-muted)',
                  textAlign: 'center',
                }}
              >
                <p style={{ margin: 0 }}>Dashboard content coming soon...</p>
              </div>
            </div>
          </div>
        ) : null}
      </main>
    </div>
  );
}