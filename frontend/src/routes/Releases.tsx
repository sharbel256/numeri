import releasesMd from "../releases.md?raw";

interface Release {
  date: string;
  notes: string[];
}

function parseReleases(md: string): Release[] {
  const releases: Release[] = [];
  let current: Release | null = null;

  for (const rawLine of md.split("\n")) {
    const line = rawLine.trimEnd();
    const heading = line.match(/^##\s+(\S+)/);
    if (heading) {
      current = { date: heading[1], notes: [] };
      releases.push(current);
      continue;
    }
    const bullet = line.match(/^[-*]\s+(.*)/);
    if (bullet && current) {
      current.notes.push(bullet[1]);
    }
  }

  return releases;
}

const RELEASES = parseReleases(releasesMd);

export default function Releases() {
  return (
    <div className="px-5 sm:px-8 lg:px-12 py-8 sm:py-10 lg:py-12 max-w-[960px] flex flex-col lg:flex-row gap-10">
      <aside className="lg:w-44 lg:shrink-0 lg:sticky lg:top-8 lg:self-start">
        <div className="font-mono text-[10px] uppercase tracking-[0.16em] text-ink-soft mb-3">
          Releases
        </div>
        <nav className="flex flex-col gap-1.5">
          {RELEASES.map((r) => (
            <a
              key={r.date}
              href={`#${r.date}`}
              className="font-mono text-[11px] uppercase tracking-[0.12em] text-ink-soft hover:text-ink"
            >
              {r.date}
            </a>
          ))}
        </nav>
      </aside>

      <div className="flex-1 min-w-0">
        <div className="font-mono text-[10px] uppercase tracking-[0.16em] text-ink-soft mb-3">
          Release notes
        </div>

        <div className="flex flex-col gap-10">
          {RELEASES.map((r) => (
            <article
              key={r.date}
              id={r.date}
              className="flex flex-col gap-3 scroll-mt-8"
            >
              <h2 className="font-mono text-[10px] uppercase tracking-[0.12em] text-ink-soft">
                {r.date}
              </h2>
              <ul className="font-display text-base sm:text-[17px] leading-relaxed text-ink list-none pl-0 flex flex-col gap-2">
                {r.notes.map((n, i) => (
                  <li key={i} className="flex gap-3">
                    <span className="font-mono text-[10px] text-accent pt-2 shrink-0">
                      ·
                    </span>
                    <span>{n}</span>
                  </li>
                ))}
              </ul>
            </article>
          ))}
        </div>
      </div>
    </div>
  );
}
