import { useEffect, useState } from "react";

import { ChoiceInput } from "../components/ChoiceInput";
import { HintTrail } from "../components/HintTrail";
import { Latex } from "../components/Latex";
import { api, ApiError } from "../lib/api";
import { scoreFor, storage, type Result } from "../lib/storage";
import type {
  Category,
  Level,
  PublicPuzzle,
  StatsResponse,
  TodaySummary,
} from "../lib/types";

const CATEGORY_NAMES: Record<Category, string> = {
  algebra: "Algebra",
  geometry: "Geometry",
  numbers: "Number Theory",
  logic: "Logic & Sequences",
  probability: "Probability",
  calculus: "Calculus",
  theory: "Theory",
};

const LEVELS: Level[] = [1, 2, 3];
const ROMAN: Record<Level, string> = { 1: "I", 2: "II", 3: "III" };

function isTypingTarget(target: EventTarget | null): boolean {
  if (!(target instanceof HTMLElement)) return false;
  const tag = target.tagName;
  if (tag === "TEXTAREA") return true;
  if (tag === "INPUT") return !(target as HTMLInputElement).disabled;
  return target.isContentEditable;
}

function readLevelFromHash(): Level {
  const n = Number(window.location.hash.replace(/^#/, ""));
  return n === 2 || n === 3 ? n : 1;
}

function formatHeaderDate(iso: string): string {
  const [y, m, d] = iso.split("-").map(Number);
  return new Date(y, m - 1, d).toLocaleDateString("en-US", {
    weekday: "long",
    month: "long",
    day: "numeric",
    year: "numeric",
  });
}

export default function Home() {
  const [today, setToday] = useState<TodaySummary | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [level, setLevelState] = useState<Level>(readLevelFromHash);

  const setLevel = (l: Level) => {
    setLevelState(l);
    window.history.replaceState(null, "", `#${l}`);
  };

  useEffect(() => {
    api
      .today()
      .then(setToday)
      .catch((e: ApiError) => setError(e.code));
  }, []);

  useEffect(() => {
    function onHashChange() {
      setLevelState(readLevelFromHash());
    }
    window.addEventListener("hashchange", onHashChange);
    return () => window.removeEventListener("hashchange", onHashChange);
  }, []);

  useEffect(() => {
    function onKey(e: KeyboardEvent) {
      if (e.metaKey || e.ctrlKey || e.altKey) return;
      if (isTypingTarget(e.target)) return;
      if (e.key === "1") setLevel(1);
      else if (e.key === "2") setLevel(2);
      else if (e.key === "3") setLevel(3);
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  if (error) {
    return (
      <div className="px-5 sm:px-8 lg:px-12 py-12 font-display italic text-ink-soft">
        Couldn't load today's set.
      </div>
    );
  }

  if (!today) return null;

  if (!today.category) {
    return (
      <div className="px-5 sm:px-8 lg:px-12 py-12 font-display italic text-ink-soft">
        No puzzles published for today yet.
      </div>
    );
  }

  return (
    <div className="flex flex-col" style={{ containerType: "inline-size" }}>
      <Header
        category={today.category}
        date={today.date}
        level={level}
        onPickLevel={setLevel}
      />
      <SolveOne
        key={`${today.date}/${today.category}/${level}`}
        date={today.date}
        category={today.category}
        level={level}
        onAdvance={() => {
          if (level < 3) setLevel((level + 1) as Level);
        }}
      />
    </div>
  );
}

function Header({
  category,
  date,
  level,
  onPickLevel,
}: {
  category: Category;
  date: string;
  level: Level;
  onPickLevel: (l: Level) => void;
}) {
  const state = storage.get();
  return (
    <div className="grid grid-cols-[1fr_auto] items-baseline gap-3 border-b border-rule px-5 sm:px-8 lg:px-12 py-3.5">
      <div className="font-mono text-[10px] uppercase tracking-[0.08em] text-ink-soft truncate">
        {formatHeaderDate(date)} <span className="text-ink-faint">·</span>{" "}
        {CATEGORY_NAMES[category] ?? category}
      </div>
      <div className="flex items-baseline gap-5 sm:gap-7">
        {LEVELS.map((lvl) => {
          const r = state.results[`${date}:${category}:${lvl}`];
          const solved = r?.correct === true;
          const attempted = r != null;
          const isCurrent = lvl === level;
          return (
            <button
              key={lvl}
              type="button"
              onClick={() => onPickLevel(lvl)}
              aria-current={isCurrent ? "true" : undefined}
              aria-label={`Level ${lvl}${solved ? " (solved)" : ""}`}
              className={`font-mono text-xs uppercase tracking-[0.16em] transition-colors
                hover:underline underline-offset-4 decoration-accent
                ${isCurrent ? "underline decoration-accent" : ""}
                ${solved ? "text-accent" : attempted ? "text-ink-soft" : "text-ink"}`}
            >
              {ROMAN[lvl]}
            </button>
          );
        })}
      </div>
    </div>
  );
}

type SolveState = "solving" | "correct" | "failed";

function buildShareText(
  date: string,
  category: Category,
  results: Record<string, Result>,
  streak: number,
): string {
  let total = 0;
  const grid = LEVELS.map((lvl) => {
    const r = results[`${date}:${category}:${lvl}`];
    if (!r) return "⬜";
    total += r.score;
    if (!r.correct) return "🟥";
    return r.hints === 0 ? "🟩" : "🟨";
  }).join("");
  const scoreLine =
    streak > 0
      ? `${grid}  ${total}/300  · streak ${streak}`
      : `${grid}  ${total}/300`;
  return [
    `${CATEGORY_NAMES[category]} · ${date}`,
    scoreLine,
    `math.sharbel.cc`,
  ].join("\n");
}

const ACTION_BTN =
  "font-mono text-[11px] uppercase tracking-[0.16em] text-ink " +
  "border border-ink px-3 py-2 cursor-pointer " +
  "hover:bg-ink hover:text-paper transition-colors";

function SolveOne({
  date,
  category,
  level,
  onAdvance,
}: {
  date: string;
  category: Category;
  level: Level;
  onAdvance: () => void;
}) {
  const [puzzle, setPuzzle] = useState<PublicPuzzle | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [picked, setPicked] = useState<string | null>(null);
  const [hintsRevealed, setHintsRevealed] = useState(0);
  const [wrong, setWrong] = useState(0);
  const [state, setState] = useState<SolveState>("solving");
  const [shake, setShake] = useState(false);
  const [showWalkthrough, setShowWalkthrough] = useState(false);
  const [copied, setCopied] = useState(false);
  const [stats, setStats] = useState<StatsResponse | null>(null);

  async function doShare() {
    const s = storage.get();
    const text = buildShareText(date, category, s.results, s.streak);
    try {
      await navigator.clipboard.writeText(text);
      setCopied(true);
      setTimeout(() => setCopied(false), 1500);
    } catch {
      /* clipboard unavailable */
    }
  }

  useEffect(() => {
    api
      .puzzle(date, category, level)
      .then((p) => {
        setPuzzle(p);
        const prior = storage.resultFor(date, category, level);
        if (prior) {
          setHintsRevealed(prior.hints);
          setWrong(prior.wrong);
          setState(prior.correct ? "correct" : "failed");
          setPicked(prior.answer);
        } else {
          const progress = storage.progressFor(date, category, level);
          if (progress) {
            setHintsRevealed(progress.hints);
            setWrong(progress.wrong);
          }
        }
      })
      .catch((e: ApiError) => setError(e.code));
  }, [date, category, level]);

  useEffect(() => {
    if (state === "solving") return;
    let cancelled = false;
    api
      .stats(date, category, level)
      .then((s) => {
        if (!cancelled) setStats(s);
      })
      .catch(() => {
        /* stats are optional — silently skip on failure */
      });
    return () => {
      cancelled = true;
    };
  }, [state, date, category, level]);

  useEffect(() => {
    function onKey(e: KeyboardEvent) {
      if (e.metaKey || e.ctrlKey || e.altKey) return;
      if (!puzzle) return;

      if (e.key === "Enter" && state === "correct" && level < 3) {
        e.preventDefault();
        onAdvance();
        return;
      }

      if (isTypingTarget(e.target)) return;

      const k = e.key.toLowerCase();

      if (k === "h" && state === "solving" && hintsRevealed < puzzle.hints.length) {
        e.preventDefault();
        const next = hintsRevealed + 1;
        setHintsRevealed(next);
        storage.recordProgress(date, category, level, { hints: next, wrong });
        return;
      }

      if (state === "correct") {
        if (k === "w" && puzzle.walkthrough && !showWalkthrough) {
          e.preventDefault();
          setShowWalkthrough(true);
          return;
        }
        if (k === "c") {
          e.preventDefault();
          doShare();
          return;
        }
      }

      if (state === "solving") {
        const idx = "abcdef".indexOf(k);
        if (idx >= 0 && idx < puzzle.choices.length) {
          e.preventDefault();
          setPicked(puzzle.choices[idx]);
        }
      }
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [puzzle, state, level, hintsRevealed, wrong, onAdvance, date, category, showWalkthrough]);

  if (error) {
    return (
      <div className="px-5 sm:px-8 lg:px-12 py-12 font-display italic text-ink-soft">
        {error === "puzzle_not_found" ? "Puzzle not found." : "Failed to load."}
      </div>
    );
  }

  if (!puzzle) return null;

  const score = scoreFor(hintsRevealed, wrong);

  async function trySubmit(candidate: string) {
    if (!puzzle) return;
    try {
      const { correct } = await api.check(date, category, level, candidate);
      if (correct) {
        setState("correct");
        storage.recordResult({
          date,
          category,
          level,
          correct: true,
          hints: hintsRevealed,
          wrong,
          score,
          answer: candidate,
        });
      } else {
        const newWrong = wrong + 1;
        setWrong(newWrong);
        setShake(true);
        setTimeout(() => setShake(false), 400);
        storage.recordProgress(date, category, level, {
          hints: hintsRevealed,
          wrong: newWrong,
        });
        if (newWrong >= 3) {
          setState("failed");
          storage.recordResult({
            date,
            category,
            level,
            correct: false,
            hints: hintsRevealed,
            wrong: newWrong,
            score: 0,
            answer: candidate,
          });
        } else {
          setPicked(null);
        }
      }
    } catch {
      setShake(true);
      setTimeout(() => setShake(false), 400);
    }
  }

  function submitChoice() {
    if (picked != null) trySubmit(picked);
  }

  function askHint() {
    if (puzzle && hintsRevealed < puzzle.hints.length) {
      const next = hintsRevealed + 1;
      setHintsRevealed(next);
      storage.recordProgress(date, category, level, { hints: next, wrong });
    }
  }

  return (
    <>
      <div className="grid grid-cols-[auto_1fr] items-baseline gap-3 px-5 sm:px-8 lg:px-12 pt-3.5">
        <div className="font-mono text-[10px] uppercase tracking-[0.08em] text-ink-soft">
          Problem {ROMAN[level]}
        </div>
        <div className="font-mono text-[10px] tracking-wider justify-self-end">
          <span className={state === "correct" ? "text-accent" : "text-ink"}>{score}</span>
          <span className="text-ink-faint"> / 100</span>
        </div>
      </div>

      <div className="flex-1 flex flex-col lg:grid lg:grid-cols-[1fr_320px] gap-7 lg:gap-14 px-5 sm:px-8 lg:px-12 py-6 sm:py-7 lg:py-9">
        <div className="flex flex-col gap-5 lg:gap-8">
          <Latex
            source={puzzle.question}
            className={`font-display font-normal tracking-tight max-w-[620px]
              text-[22px] sm:text-[28px] lg:text-[36px] leading-[1.35] lg:leading-[1.3]
              transition-opacity duration-500 ${state === "correct" ? "opacity-50" : "opacity-100"}`}
          />

          <div className={`flex flex-col gap-3 ${shake ? "animate-shake" : ""}`}>
            <ChoiceInput
              choices={puzzle.choices}
              labels={puzzle.choice_labels}
              picked={picked}
              correctAnswer={null}
              onPick={setPicked}
              onSubmit={submitChoice}
              disabled={state !== "solving"}
              state={state}
              wrong={wrong}
              stats={stats}
            />

            {state === "correct" && (
              <div className="flex flex-col gap-3 pt-1">
                <div className="font-display italic text-ink-soft text-sm">
                  Solved with {hintsRevealed} hint{hintsRevealed !== 1 ? "s" : ""} and {wrong} wrong
                  attempt{wrong !== 1 ? "s" : ""}.
                </div>
                <div className="flex flex-wrap items-center gap-3">
                  {level < 3 && (
                    <button type="button" onClick={onAdvance} className={ACTION_BTN}>
                      Continue to {ROMAN[(level + 1) as Level]}{" "}
                      <span className="opacity-60 ml-1.5">↵</span>
                    </button>
                  )}
                  {puzzle.walkthrough && !showWalkthrough && (
                    <button
                      type="button"
                      onClick={() => setShowWalkthrough(true)}
                      className={ACTION_BTN}
                    >
                      Show walkthrough <span className="opacity-60 ml-1.5">W</span>
                    </button>
                  )}
                  <button type="button" onClick={doShare} className={ACTION_BTN}>
                    {copied ? "Copied" : "Copy"}{" "}
                    <span className="opacity-60 ml-1.5">C</span>
                  </button>
                </div>
              </div>
            )}

            {((state === "failed") || (state === "correct" && showWalkthrough)) && puzzle.walkthrough && (
              <div className="bg-paper-alt border-l-2 border-accent px-5 py-4 mt-3">
                <div className="font-mono text-[10px] uppercase tracking-[0.16em] text-ink-soft mb-2">
                  Walkthrough
                </div>
                <Latex
                  source={puzzle.walkthrough}
                  className="font-display text-[17px] leading-relaxed"
                />
              </div>
            )}
          </div>
        </div>

        <HintTrail
          hints={puzzle.hints}
          revealed={hintsRevealed}
          onRequest={askHint}
          canRequest={hintsRevealed < puzzle.hints.length}
          state={state}
        />
      </div>
    </>
  );
}
