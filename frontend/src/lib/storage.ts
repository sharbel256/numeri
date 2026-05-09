// localStorage-backed streak + per-(date,category,level) results. Single-player only.

import type { Category, Level } from "./types";

const KEY = "numeri.state.v3";

export interface Result {
  date: string;
  category: Category;
  level: Level;
  correct: boolean;
  hints: number;
  wrong: number;
  score: number;
  answer: string;
}

export interface Progress {
  hints: number;
  wrong: number;
}

interface State {
  results: Record<string, Result>; // key = `${date}:${category}:${level}`
  progress: Record<string, Progress>;
  streak: number;
  lastSolvedDate: string | null;
}

const empty: State = { results: {}, progress: {}, streak: 0, lastSolvedDate: null };

function read(): State {
  try {
    const raw = localStorage.getItem(KEY);
    return raw ? { ...empty, ...JSON.parse(raw) } : { ...empty };
  } catch {
    return { ...empty };
  }
}

function write(s: State): void {
  localStorage.setItem(KEY, JSON.stringify(s));
}

function isConsecutive(prev: string | null, next: string): boolean {
  if (!prev) return false;
  const a = new Date(prev + "T00:00:00Z").getTime();
  const b = new Date(next + "T00:00:00Z").getTime();
  return b - a === 86_400_000;
}

export function key(date: string, category: Category, level: Level): string {
  return `${date}:${category}:${level}`;
}

export const storage = {
  get(): State {
    return read();
  },
  resultFor(date: string, category: Category, level: Level): Result | null {
    return read().results[key(date, category, level)] ?? null;
  },
  progressFor(date: string, category: Category, level: Level): Progress | null {
    return read().progress[key(date, category, level)] ?? null;
  },
  recordProgress(date: string, category: Category, level: Level, p: Progress): void {
    const s = read();
    s.progress[key(date, category, level)] = p;
    write(s);
  },
  recordResult(r: Result): State {
    const s = read();
    s.results[key(r.date, r.category, r.level)] = r;
    delete s.progress[key(r.date, r.category, r.level)];
    if (r.correct) {
      // Per-day streak: any solved puzzle counts that calendar day once.
      if (s.lastSolvedDate === r.date) {
        // Same day, already counted — no change.
      } else if (isConsecutive(s.lastSolvedDate, r.date)) {
        s.streak += 1;
      } else {
        s.streak = 1;
      }
      s.lastSolvedDate = r.date;
    }
    write(s);
    return s;
  },
};

export function scoreFor(hints: number, wrong: number): number {
  return Math.max(0, 100 - hints * 15 - wrong * 10);
}
