import { Link } from "react-router-dom";

// TODO: update if your repo URL differs.
const GITHUB_URL = "https://github.com/sharbel256/numeri";

export function Footer() {
  const year = new Date().getUTCFullYear();
  return (
    <footer
      className="border-t border-rule px-5 sm:px-8 lg:px-12 py-4 sm:py-5
        flex flex-row items-baseline justify-between gap-4
        font-mono text-[10px] uppercase tracking-[0.12em] text-ink-soft"
    >
      <div>© {year} numeri</div>
      <div className="flex gap-4 sm:gap-5">
        <a
          href={GITHUB_URL}
          target="_blank"
          rel="noreferrer"
          className="no-underline hover:underline underline-offset-4 decoration-accent
            hover:text-accent transition-colors"
        >
          source
        </a>
        <Link
          to="/releases"
          className="no-underline hover:underline underline-offset-4 decoration-accent
            hover:text-accent transition-colors"
        >
          release notes
        </Link>
      </div>
    </footer>
  );
}
