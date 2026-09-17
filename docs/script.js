* {
  box-sizing: border-box;
}

:root {
  --bg: #07111f;
  --bg-alt: #0d1a2b;
  --panel: rgba(18, 27, 40, 0.88);
  --panel-soft: rgba(255, 255, 255, 0.04);
  --primary: #43d9ad;
  --primary-strong: #1ec98f;
  --secondary: #5da7ff;
  --text: #edf3ff;
  --muted: #a9b7cc;
  --line: rgba(255, 255, 255, 0.1);
  --warning: #ffb454;
  --danger: #ff6b6b;
  --shadow: 0 20px 55px rgba(0, 0, 0, 0.35);
}

html {
  scroll-behavior: smooth;
}

body {
  margin: 0;
  font-family: "Inter", sans-serif;
  background:
    radial-gradient(circle at top left, rgba(67, 217, 173, 0.15), transparent 30%),
    radial-gradient(circle at bottom right, rgba(93, 167, 255, 0.16), transparent 25%),
    var(--bg);
  color: var(--text);
}

img {
  max-width: 100%;
}

a {
  color: inherit;
  text-decoration: none;
}

code {
  font-family: "SFMono-Regular", Consolas, monospace;
  background: rgba(255, 255, 255, 0.06);
  border: 1px solid var(--line);
  border-radius: 6px;
  padding: 0.12rem 0.5rem;
}

.container {
  width: min(1120px, calc(100% - 2rem));
  margin: 0 auto;
}

.topbar {
  position: sticky;
  top: 0;
  z-index: 10;
  backdrop-filter: blur(14px);
  background: rgba(7, 17, 31, 0.76);
  border-bottom: 1px solid var(--line);
}

.nav {
  height: 72px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 1rem;
}

.brand {
  display: inline-flex;
  align-items: center;
  gap: 0.75rem;
  font-weight: 700;
  letter-spacing: 0.03em;
}

.brand-mark {
  width: 32px;
  height: 32px;
  border-radius: 10px;
  display: grid;
  place-items: center;
  background: linear-gradient(135deg, var(--primary), var(--secondary));
  color: #04101a;
  font-weight: 800;
}

nav {
  display: flex;
  align-items: center;
  gap: 1.5rem;
  color: var(--muted);
  font-size: 0.96rem;
}

nav a:hover {
  color: var(--text);
}

.hero {
  padding: 4rem 0 2.5rem;
}

.hero-grid {
  display: grid;
  grid-template-columns: 1.1fr 0.9fr;
  gap: 2rem;
  align-items: center;
}

.eyebrow {
  display: inline-block;
  font-size: 0.8rem;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--primary);
  font-weight: 700;
  margin-bottom: 1rem;
}

.hero-copy h1 {
  margin: 0;
  font-size: clamp(2.4rem, 5vw, 4.3rem);
  line-height: 1.05;
  letter-spacing: -0.06em;
}

.hero-copy p {
  max-width: 620px;
  color: var(--muted);
  font-size: 1.08rem;
  line-height: 1.7;
  margin: 1.1rem 0 0;
}

.cta-row {
  display: flex;
  flex-wrap: wrap;
  gap: 1rem;
  margin-top: 1.7rem;
}

.button {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 0.9rem 1.4rem;
  border-radius: 12px;
  font-weight: 600;
  transition: 0.2s ease;
}

.button.primary {
  background: linear-gradient(135deg, var(--primary), var(--secondary));
  color: #07111f;
  box-shadow: var(--shadow);
}

.button.secondary {
  border: 1px solid var(--line);
  background: rgba(255, 255, 255, 0.02);
}

.button:hover {
  transform: translateY(-1px);
}

.small-stats {
  list-style: none;
  padding: 0;
  margin: 2rem 0 0;
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 1rem;
}

.small-stats li {
  background: rgba(255, 255, 255, 0.02);
  border: 1px solid var(--line);
  border-radius: 14px;
  padding: 1rem;
}

.small-stats strong {
  display: block;
  font-size: 1.2rem;
  margin-bottom: 0.25rem;
}

.small-stats span {
  color: var(--muted);
  font-size: 0.85rem;
}

.dashboard-card,
.feature-card,
.architecture-box {
  background: var(--panel);
  border: 1px solid var(--line);
  border-radius: 22px;
  box-shadow: var(--shadow);
}

.dashboard-card {
  padding: 1.2rem;
}

.panel-header,
.neighbor-head {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding-bottom: 1rem;
  border-bottom: 1px solid var(--line);
  color: var(--muted);
  font-size: 0.9rem;
}

.status-dot {
  color: var(--primary);
  font-size: 1rem;
}

.metrics {
  display: grid;
  gap: 0.85rem;
  margin-top: 1rem;
}

.row-3 {
  grid-template-columns: repeat(3, minmax(0, 1fr));
}

.metric {
  background: var(--panel-soft);
  border: 1px solid var(--line);
  border-radius: 16px;
  padding: 0.8rem;
}

.metric label {
  display: block;
  font-size: 0.76rem;
  color: var(--muted);
  margin-bottom: 0.5rem;
  letter-spacing: 0.04em;
  text-transform: uppercase;
}

.metric strong {
  font-size: 1.1rem;
}

.chart-box {
  display: flex;
  align-items: end;
  gap: 0.75rem;
  height: 150px;
  margin-top: 1.2rem;
  padding: 1rem;
  border-radius: 16px;
  background: linear-gradient(180deg, rgba(67, 217, 173, 0.08), rgba(93, 167, 255, 0.04));
  border: 1px solid var(--line);
}

.chart-bar {
  flex: 1;
  height: 100%;
  position: relative;
  display: flex;
  align-items: end;
  justify-content: center;
}

.chart-bar span {
  display: block;
  width: 100%;
  background: linear-gradient(180deg, var(--secondary), var(--primary));
  border-radius: 8px 8px 0 0;
  min-height: 30%;
}

.neighbor-list {
  margin-top: 1.3rem;
}

.neighbor-item {
  display: flex;
  justify-content: space-between;
  background: rgba(255, 255, 255, 0.02);
  border: 1px solid var(--line);
  border-radius: 12px;
  padding: 0.8rem 0.9rem;
  margin-top: 0.7rem;
  color: var(--muted);
}

.section {
  padding: 4.25rem 0;
}

.section.alt {
  background: rgba(255, 255, 255, 0.01);
  border-top: 1px solid var(--line);
  border-bottom: 1px solid var(--line);
}

.section-heading {
  margin-bottom: 1.75rem;
}

.section-heading h2 {
  margin: 0;
  font-size: clamp(2rem, 3vw, 2.8rem);
  letter-spacing: -0.05em;
}

.feature-grid {
  display: grid;
  gap: 1.2rem;
}

.three-col {
  grid-template-columns: repeat(3, minmax(0, 1fr));
}

.four-col {
  grid-template-columns: repeat(4, minmax(0, 1fr));
}

.feature-card {
  padding: 1.4rem;
}

.feature-card h3 {
  margin-top: 0;
  margin-bottom: 0.75rem;
  font-size: 1.15rem;
}

.feature-card p {
  margin: 0;
  color: var(--muted);
  line-height: 1.7;
}

.architecture-box {
  padding: 1.6rem;
}

.flow {
  display: flex;
  align-items: center;
  justify-content: center;
  flex-wrap: wrap;
  gap: 0.7rem;
  margin-bottom: 1rem;
}

.node {
  background: rgba(67, 217, 173, 0.1);
  border: 1px solid rgba(67, 217, 173, 0.35);
  color: var(--text);
  border-radius: 999px;
  padding: 0.8rem 1rem;
  font-weight: 700;
  white-space: nowrap;
}

.arrow {
  color: var(--muted);
  font-weight: 700;
}

.architecture-box p {
  margin: 0;
  color: var(--muted);
  line-height: 1.7;
  font-size: 1rem;
}

.setup-list {
  margin: 0;
  padding-left: 1.5rem;
  color: var(--muted);
  line-height: 1.9;
  font-size: 1.03rem;
}

.footer {
  border-top: 1px solid var(--line);
  padding: 1.2rem 0 2rem;
}

.footer-inner {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 1rem;
  color: var(--muted);
}

@media (max-width: 900px) {
  .hero-grid,
  .three-col,
  .four-col {
    grid-template-columns: 1fr;
  }

  nav {
    display: none;
  }
}

@media (max-width: 520px) {
  .small-stats,
  .row-3 {
    grid-template-columns: 1fr;
  }

  .hero {
    padding-top: 2.75rem;
  }

  .cta-row {
    flex-direction: column;
  }

  .button {
    width: 100%;
  }
}
