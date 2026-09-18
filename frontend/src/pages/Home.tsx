import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAppDispatch, useAppSelector } from '../store/index';
import { clearCredentials } from '../store/authSlice';
import { setProjects, setError, setLoading } from '../store/projectSlice';
import { validateTokenApi, logoutApi } from '../api/auth';
import { getProjectsList } from '../api/projects';

export default function Home() {
  const dispatch = useAppDispatch();
  const navigate = useNavigate();
  const { token, user } = useAppSelector((state) => state.auth);
  const { projects, loading, error } = useAppSelector((state) => state.project);
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
    const fetchProjects = async () => {
      if (!token) return;

      dispatch(setLoading(true));
      dispatch(setError(null));

      try {
        const projectsList = await getProjectsList(token);
        dispatch(setProjects(projectsList));
      } catch (err) {
        dispatch(setError(err instanceof Error ? err.message : 'Failed to fetch projects'));
      } finally {
        dispatch(setLoading(false));
      }
    };

    fetchProjects();
  }, [token, dispatch]);

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

  const handleProjectClick = (projectKey: string) => {
    navigate(`/project/${projectKey}`);
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
          padding: '40px 24px',
        }}
      >
        <div style={{ maxWidth: '800px', margin: '0 auto' }}>
          <h2
            style={{
              fontSize: '28px',
              fontWeight: '600',
              marginBottom: '8px',
              color: 'var(--color-text)',
            }}
          >
            Welcome, {user?.username}!
          </h2>
          <p
            style={{
              fontSize: '16px',
              color: 'var(--color-muted)',
              margin: '0 0 32px 0',
            }}
          >
            Self-hosted data storage tool
          </p>

          {loading && (
            <div style={{ color: 'var(--color-muted)' }}>
              <p>Loading projects...</p>
            </div>
          )}

          {error && (
            <div
              style={{
                backgroundColor: 'rgba(239, 68, 68, 0.1)',
                color: 'var(--color-accent)',
                padding: '12px 16px',
                borderRadius: '4px',
                marginBottom: '16px',
              }}
            >
              Error: {error}
            </div>
          )}

          {!loading && projects && projects.length > 0 && (
            <div>
              <h3
                style={{
                  fontSize: '18px',
                  fontWeight: '600',
                  marginBottom: '16px',
                  color: 'var(--color-text)',
                }}
              >
                Your Projects
              </h3>
              <div
                style={{
                  display: 'grid',
                  gridTemplateColumns: 'repeat(auto-fill, minmax(250px, 1fr))',
                  gap: '16px',
                }}
              >
                {projects.map((project) => (
                  <div
                    key={project.key}
                    onClick={() => handleProjectClick(project.key)}
                    style={{
                      backgroundColor: 'var(--color-surface)',
                      border: '1px solid var(--color-border)',
                      borderRadius: '4px',
                      padding: '16px',
                      cursor: 'pointer',
                      transition: 'all 0.2s ease',
                    }}
                    onMouseEnter={(e) => {
                      e.currentTarget.style.borderColor = 'var(--color-accent)';
                      e.currentTarget.style.boxShadow = '0 2px 8px rgba(0, 0, 0, 0.1)';
                    }}
                    onMouseLeave={(e) => {
                      e.currentTarget.style.borderColor = 'var(--color-border)';
                      e.currentTarget.style.boxShadow = 'none';
                    }}
                  >
                    <h4
                      style={{
                        margin: '0 0 8px 0',
                        fontSize: '16px',
                        fontWeight: '600',
                        color: 'var(--color-text)',
                      }}
                    >
                      {project.name}
                    </h4>
                    <p
                      style={{
                        margin: 0,
                        fontSize: '12px',
                        color: 'var(--color-muted)',
                      }}
                    >
                      {project.key}
                    </p>
                  </div>
                ))}
              </div>
            </div>
          )}

          {!loading && projects && projects.length === 0 && (
            <div
              style={{
                textAlign: 'center',
                padding: '40px 20px',
                color: 'var(--color-muted)',
              }}
            >
              <p>No projects yet. Contact your administrator to get access to a project.</p>
            </div>
          )}
        </div>
      </main>
    </div>
  );
}
