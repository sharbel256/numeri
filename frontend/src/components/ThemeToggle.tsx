import { useEffect, useState } from "react";

type Theme = "noon" | "midnight";

function readTheme(): Theme {
  return document.documentElement.getAttribute("data-theme") === "midnight"
    ? "midnight"
    : "noon";
}

function applyTheme(t: Theme) {
  if (t === "midnight") {
    document.documentElement.setAttribute("data-theme", "midnight");
  } else {
    document.documentElement.removeAttribute("data-theme");
  }
  try {
    localStorage.setItem("numeri.theme", t);
  } catch {
    /* storage unavailable */
  }
  const meta = document.querySelector('meta[name="theme-color"]');
  if (meta) {
    meta.setAttribute("content", t === "midnight" ? "#011627" : "#F4EDE0");
  }
}

const THEMES: Theme[] = ["noon", "midnight"];

export function ThemeToggle() {
  const [theme, setTheme] = useState<Theme>(readTheme);

  useEffect(() => {
    applyTheme(theme);
  }, [theme]);

  return (
    <div className="flex items-baseline gap-1.5 font-mono text-[10px] uppercase tracking-[0.12em]">
      {THEMES.map((t, i) => (
        <span key={t} className="flex items-baseline gap-1.5">
          {i > 0 && <span className="text-ink-faint">·</span>}
          <button
            type="button"
            onClick={() => setTheme(t)}
            aria-pressed={theme === t}
            className={`cursor-pointer transition-colors hover:text-ink
              ${theme === t ? "text-ink underline underline-offset-4 decoration-accent" : "text-ink-soft"}`}
          >
            {t}
          </button>
        </span>
      ))}
    </div>
  );
}
