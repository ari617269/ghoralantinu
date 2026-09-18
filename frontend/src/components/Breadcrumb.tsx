import { Link } from 'react-router-dom';

interface BreadcrumbItem {
  label: string;
  href?: string;
}

interface BreadcrumbProps {
  items: BreadcrumbItem[];
}

export default function Breadcrumb({ items }: BreadcrumbProps) {
  return (
    <nav style={{ padding: '12px 24px' }}>
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          gap: '8px',
          fontSize: '14px',
          color: 'var(--color-muted)',
        }}
      >
        {items.map((item, index) => (
          <div key={index} style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
            {index > 0 && <span>/</span>}
            {item.href ? (
              <Link
                to={item.href}
                style={{
                  color: 'var(--color-accent)',
                  textDecoration: 'none',
                }}
              >
                {item.label}
              </Link>
            ) : (
              <span style={{ color: 'var(--color-text)' }}>{item.label}</span>
            )}
          </div>
        ))}
      </div>
    </nav>
  );
}
