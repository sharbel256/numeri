import { ThemeToggle } from "./ThemeToggle";

export function Footer() {
  const year = new Date().getUTCFullYear();
  return (
    <footer
      className="border-t border-rule px-5 sm:px-8 lg:px-12 py-4 sm:py-5
        flex items-baseline justify-between gap-4
        font-mono text-[10px] uppercase tracking-[0.12em] text-ink-soft"
    >
      <span>© {year} numeri</span>
      <ThemeToggle />
    </footer>
  );
}
