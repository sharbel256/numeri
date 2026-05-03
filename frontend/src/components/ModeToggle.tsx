import type { Mode } from "../lib/types";

interface Props {
  current: Mode;
  onSwitch: (m: Mode) => void;
}

// Subtle inline link — surfaced only when the puzzle has both modes available.
export function ModeToggle({ current, onSwitch }: Props) {
  const target: Mode = current === "free" ? "choice" : "free";
  const label = target === "choice" ? "multiple choice options" : "type my own answer";
  return (
    <button
      onClick={() => onSwitch(target)}
      className="font-mono text-[10px] uppercase tracking-[0.12em] text-ink-soft
        bg-transparent border-none cursor-pointer underline underline-offset-4
        decoration-rule hover:decoration-accent hover:text-accent transition-colors
        self-start mt-1"
    >
      ↔ {label}
    </button>
  );
}
