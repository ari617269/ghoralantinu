# Security Policy

## Supported Versions

Security fixes are applied to the latest release on `main`. Older versions are not maintained.

## Reporting a Vulnerability

Do not use the public issue tracker for security vulnerabilities.

Email the maintainers directly. Include:

- A description of the vulnerability and its potential impact.
- Steps to reproduce or a proof-of-concept (if available).
- Any suggested mitigations.

You will receive an acknowledgement within 48 hours. We aim to release a fix or mitigation within 14 days for critical issues.

## Security Model

### Authentication

- Passwords are hashed with **bcrypt** at cost factor 10. Plain-text passwords are never stored or logged.
- Authentication issues a signed **JWT** with a 1-hour expiry. The payload contains `{ id, username }`.
- The `JWT_SECRET` must be set to a random 256-bit (32-byte) value in production. The default value in the repository is a placeholder and must not be used in any environment accessible over a network.

### Token revocation

- Logout inserts the token into an `expired_tokens` database table. The `activeLogin` middleware rejects any token present in this table, regardless of the JWT signature being valid.
- `expired_at` is set to the token's original `exp` claim so revoked tokens can be pruned after they would have expired naturally.

### Client-side storage

- The frontend stores the JWT **in Redux memory only** — never in `localStorage`, `sessionStorage`, or cookies. The token is lost on page refresh, which forces re-authentication.
- This prevents token theft via XSS that targets persistent browser storage.

### Transport

- The project does not include TLS configuration. In production, terminate TLS at a reverse proxy (nginx, Caddy) in front of both the API and the frontend.

### CORS

- The backend does not configure CORS headers. If the API and frontend are served from different origins in production, add `cors` middleware to the Express app and restrict allowed origins explicitly. Do not use a wildcard (`*`) with credentials.

## Production Hardening Checklist

- [ ] Set `JWT_SECRET` to a securely generated random value (e.g. `node -e "console.log(require('crypto').randomBytes(32).toString('hex'))"`)
- [ ] Set `NODE_ENV=production`
- [ ] Terminate TLS at a reverse proxy; redirect HTTP to HTTPS
- [ ] Run the backend under a non-root OS user with minimal filesystem permissions
- [ ] Restrict database user permissions: the application user needs only `SELECT`, `INSERT`, `UPDATE`, `DELETE` on application tables — not `CREATE`, `DROP`, or superuser rights
- [ ] Rotate `JWT_SECRET` periodically; existing sessions will be invalidated on rotation
- [ ] Configure firewall rules so the PostgreSQL port (5432) is not exposed externally
