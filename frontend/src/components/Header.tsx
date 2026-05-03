import { Link } from "react-router-dom";
import { storage } from "../lib/storage";

const TAGLINE = "math once a day.";

function issueNumber(date: Date): number {
  const start = new Date("2024-01-01T00:00:00Z").getTime();
  return Math.floor((date.getTime() - start) / 86_400_000) + 1;
}

export function Header() {
  const now = new Date();
  const issue = issueNumber(now);
  const dateStr = now.toLocaleDateString("en-US", {
    weekday: "long",
    month: "long",
    day: "numeric",
    year: "numeric",
  });
  const { streak } = storage.get();

  return (
    <header className="px-4 sm:px-8 lg:px-12 py-2 sm:py-2.5 lg:py-3 border-b border-rule">
      <div className="flex flex-wrap items-baseline gap-x-4 sm:gap-x-6 gap-y-3">
        {/* Brand + tagline — full width below lg so meta is forced to row 2 */}
        <div className="flex items-baseline gap-x-4 sm:gap-x-6 min-w-0 w-full lg:w-auto">
          <Link
            to="/"
            className="font-display font-normal text-lg sm:text-xl lg:text-2xl tracking-tight
              no-underline text-ink hover:text-accent transition-colors"
            aria-label="numeri — home"
          >
            numeri,
          </Link>
          <div className="font-display italic font-normal text-ink-soft text-sm sm:text-base lg:text-lg leading-snug min-w-0">
            {TAGLINE}
          </div>
        </div>

        {/* Meta — its own row on narrow screens, hugs right on wide */}
        <div className="flex flex-wrap items-baseline gap-x-5 sm:gap-x-6 text-[10px] uppercase font-mono tracking-[0.08em] text-ink-soft w-full lg:w-auto lg:ml-auto">
          <div>{dateStr}</div>
          <div>№ {issue}</div>
          {streak > 0 && (
            <div className="text-accent tracking-[0.12em]">streak: {streak}</div>
          )}
        </div>
      </div>
    </header>
  );
}