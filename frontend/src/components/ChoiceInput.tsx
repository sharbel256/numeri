import { useEffect, useState } from "react";

import type { StatsResponse } from "../lib/types";
import { FitText } from "./FitText";
import { Latex } from "./Latex";

interface Props {
  choices: string[];
  labels?: string[] | null;
  picked: string | null;
  correctAnswer: string | null;
  onPick: (choice: string) => void;
  onSubmit: () => void;
  disabled: boolean;
  state: "solving" | "correct" | "failed";
  wrong: number;
  stats?: StatsResponse | null;
}

export function ChoiceInput({
  choices,
  labels,
  picked,
  correctAnswer,
  onPick,
  onSubmit,
  disabled,
  state,
  wrong,
  stats,
}: Props) {
  useEffect(() => {
    if (state !== "solving") return;
    function onKey(e: KeyboardEvent) {
      if (e.key === "Enter" && picked != null) {
        e.preventDefault();
        onSubmit();
      }
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [state, picked, onSubmit]);

  const [barsAnimated, setBarsAnimated] = useState(false);
  useEffect(() => {
    if (!stats || stats.total === 0) {
      setBarsAnimated(false);
      return;
    }
    const id = requestAnimationFrame(() => setBarsAnimated(true));
    return () => cancelAnimationFrame(id);
  }, [stats]);

  const showBars = stats != null && stats.total > 0 && state !== "solving";
  return (
    <div className="flex flex-col gap-3 max-w-[620px]">
      <div className="flex items-baseline justify-between font-mono">
        <span className="text-[10px] uppercase tracking-[0.16em] text-ink-soft">
          {state === "correct" ? (
            <>
              <span className="text-accent inline-block animate-pop mr-1.5">✓</span>
              Solved
            </>
          ) : state === "failed" ? (
            "Answer"
          ) : (
            "Choose one"
          )}
        </span>
        <span className="text-[32px] leading-none text-ink-soft">
          {[0, 1, 2].map((i) => (
            <span key={i} className={`ml-2.5 ${i < wrong ? "text-accent" : "text-rule"}`}>
              {i < wrong ? "×" : "·"}
            </span>
          ))}
        </span>
      </div>

      <div className="grid grid-cols-1 sm:grid-cols-2 gap-2.5 pt-3">
        {choices.map((opt, idx) => {
          const letter = String.fromCharCode(65 + idx);
          const isPicked = picked === opt;
          const isSelected = state === "solving" && isPicked;
          const isCorrect =
            (state !== "solving" && correctAnswer === opt) ||
            (state === "correct" && isPicked);
          const isWrongPick = isPicked && state === "failed" && !isCorrect;
          const accent = isCorrect || isSelected;
          const pct =
            showBars && stats ? ((stats.counts[opt] ?? 0) / stats.total) * 100 : 0;
          return (
            <button
              key={opt}
              disabled={disabled}
              onClick={() => onPick(opt)}
              className={`relative overflow-hidden grid grid-cols-[32px_1fr] items-center gap-3.5 text-left
                border min-h-[60px] sm:min-h-[auto] py-4 sm:py-3 transition-all
                ${isCorrect ? "bg-accent-soft animate-glow-pulse" : isSelected ? "bg-paper-alt" : ""}
                ${accent ? "border-accent" : "border-rule"}
                ${isWrongPick ? "opacity-40" : "opacity-100"}
                ${disabled ? "cursor-default" : "cursor-pointer hover:bg-paper-alt active:bg-paper-alt"}`}
            >
              {showBars && (
                <span
                  aria-hidden
                  className="absolute inset-y-0 left-0 bg-accent/15 pointer-events-none transition-[width] duration-700 ease-out"
                  style={{ width: `${barsAnimated ? pct : 0}%` }}
                />
              )}
              <span
                className={`relative font-mono font-medium text-center h-full flex items-center justify-center border-r
                  ${accent ? "text-accent border-accent" : "text-ink-soft border-rule"}`}
              >
                {letter}
              </span>
              <FitText className="relative pr-3 min-w-0">
                {labels && labels[idx] ? (
                  <Latex
                    inline
                    source={labels[idx]}
                    className="font-display text-lg sm:text-xl"
                  />
                ) : (
                  <span className="font-mono font-medium text-lg sm:text-xl">
                    {opt}
                  </span>
                )}
              </FitText>
            </button>
          );
        })}
      </div>

      {state === "solving" && (
        <button
          type="button"
          onClick={onSubmit}
          className="font-mono text-[11px] uppercase tracking-[0.16em] text-paper bg-ink
            border-none cursor-pointer px-4 py-3 min-h-12 w-full sm:w-auto self-end mt-1"
        >
          Submit ↵
        </button>
      )}
    </div>
  );
}
