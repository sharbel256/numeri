import { useEffect, useRef, type CSSProperties } from "react";

import { prettyExpression } from "../lib/expression";
import type { PaletteKey } from "../lib/palettes";
import type { FreeInput as FreeInputT } from "../lib/types";

interface Props {
  config: FreeInputT;
  value: string;
  onChange: (v: string) => void;
  onSubmit: () => void;
  disabled: boolean;
  state: "solving" | "correct" | "failed";
  wrong: number;
  score: number;
  palette?: PaletteKey[];
}

export function FreeInput({
  config,
  value,
  onChange,
  onSubmit,
  disabled,
  state,
  wrong,
  score,
  palette,
}: Props) {
  const ref = useRef<HTMLInputElement>(null);
  useEffect(() => {
    if (!disabled) ref.current?.focus();
  }, [disabled]);

  const isExpression = config.kind === "expression";
  const preview = isExpression ? prettyExpression(value) : "";

  function insertToken(tok: string) {
    const el = ref.current;
    const start = el?.selectionStart ?? value.length;
    const end = el?.selectionEnd ?? value.length;
    const next = value.slice(0, start) + tok + value.slice(end);
    onChange(next);
    const pos = start + tok.length;
    requestAnimationFrame(() => {
      el?.focus();
      el?.setSelectionRange(pos, pos);
    });
  }

  const showPalette = isExpression && palette && palette.length > 0 && state === "solving";

  return (
    <div className="flex flex-col gap-3 max-w-[620px]">
      <div className="flex items-baseline justify-between font-mono">
        <span className="text-[10px] uppercase tracking-[0.16em] text-ink-soft">
          {state === "correct"
            ? "Solved"
            : state === "failed"
              ? "Answer"
              : isExpression
                ? "Your expression"
                : "Your answer"}
        </span>
        <span className="text-[32px] leading-none text-ink-soft">
          {[0, 1, 2].map((i) => (
            <span key={i} className={`ml-2.5 ${i < wrong ? "text-accent" : "text-rule"}`}>
              {i < wrong ? "×" : "·"}
            </span>
          ))}
        </span>
      </div>

      {showPalette && (
        <div className="flex flex-wrap gap-1.5">
          {palette!.map((k) => (
            <button
              key={k.label}
              type="button"
              onMouseDown={(e) => e.preventDefault()}
              onClick={() => insertToken(k.insert)}
              className="font-mono text-[13px] text-ink bg-paper-alt border border-rule rounded
                px-2.5 py-1 hover:border-accent hover:text-accent transition-colors"
            >
              {k.label}
            </button>
          ))}
        </div>
      )}

      <div
        className={`relative flex flex-wrap items-center gap-3 rounded border px-4 py-3 transition-all
          ${state === "correct" ? "border-accent bg-paper animate-glow-pulse" : "border-rule bg-paper-alt"}
          ${state === "failed" ? "opacity-70" : ""}
          focus-within:border-accent focus-within:bg-paper focus-within:shadow-[0_0_0_3px_rgb(232_196_172/0.4)]`}
      >
        <input
          ref={ref}
          value={value}
          onChange={(e) => onChange(e.target.value)}
          onKeyDown={(e) => e.key === "Enter" && onSubmit()}
          disabled={disabled}
          inputMode={isExpression ? "text" : "decimal"}
          placeholder={
            config.placeholder ?? (isExpression ? "Type your expression…" : "Type your answer…")
          }
          spellCheck={false}
          autoCapitalize="none"
          autoCorrect="off"
          className={`flex-1 min-w-0 bg-transparent border-none outline-none font-mono font-medium
            text-[28px] sm:text-[32px] lg:text-[36px]
            ${state === "correct" ? "text-accent animate-pop" : "text-ink"}
            placeholder:text-ink-faint placeholder:opacity-70`}
          style={{ caretColor: "var(--color-accent)" }}
        />
        {state === "solving" && (
          <button
            onClick={onSubmit}
            className="font-mono text-[11px] uppercase tracking-[0.16em] text-paper bg-ink
              border-none cursor-pointer px-4 py-3 min-h-12 w-full sm:w-auto"
          >
            Submit ↵
          </button>
        )}

        {state === "correct" && (
          <>
            <div
              aria-hidden
              className="pointer-events-none absolute -top-6 -right-3 sm:-top-9 sm:-right-6
                font-display font-black italic text-accent
                leading-none tracking-tight select-none animate-stamp-in
                [text-shadow:0_2px_0_rgb(184_84_41/0.15)]
                flex items-baseline gap-1"
            >
              <span className="text-[56px] sm:text-[80px] lg:text-[96px]">{score}</span>
              <span className="text-[20px] sm:text-[28px] lg:text-[32px] opacity-70">/100</span>
            </div>
            <SparkBurst />
          </>
        )}
      </div>

      {isExpression && value.trim() && state === "solving" && (
        <div className="font-display italic text-ink-soft text-base sm:text-lg">
          <span className="font-mono not-italic text-[10px] tracking-[0.12em] uppercase text-ink-faint mr-2.5">
            reads as
          </span>
          {preview}
        </div>
      )}
    </div>
  );
}

const SPARKS = [
  { glyph: "✦", x: -120, y: -70, rot: -20, delay: 0 },
  { glyph: "✧", x: 130, y: -80, rot: 18, delay: 60 },
  { glyph: "·", x: -60, y: -110, rot: 0, delay: 120 },
  { glyph: "✦", x: 80, y: -120, rot: 12, delay: 90 },
  { glyph: "✧", x: -160, y: -30, rot: -30, delay: 150 },
  { glyph: "·", x: 170, y: -40, rot: 25, delay: 40 },
];

function SparkBurst() {
  return (
    <div aria-hidden className="pointer-events-none absolute inset-0 overflow-visible">
      {SPARKS.map((s, i) => (
        <span
          key={i}
          className="absolute left-1/2 top-1/2 font-display text-accent text-xl sm:text-2xl
            animate-spark"
          style={
            {
              "--spark-x": `${s.x}px`,
              "--spark-y": `${s.y}px`,
              "--spark-rot": `${s.rot}deg`,
              animationDelay: `${s.delay}ms`,
            } as CSSProperties
          }
        >
          {s.glyph}
        </span>
      ))}
    </div>
  );
}
