// Mirrors backend/app/models.py — keep in sync.

export type Category =
  | "algebra"
  | "geometry"
  | "numbers"
  | "logic"
  | "probability"
  | "calculus"
  | "theory";

export type Level = 1 | 2 | 3;

export interface Hint {
  text: string;
  cost: number;
}

export interface PublicPuzzle {
  date: string;
  category: Category;
  level: Level;
  question: string;
  choices: string[];
  choice_labels: string[] | null;
  hints: Hint[];
  walkthrough: string | null;
}

export interface TodaySummary {
  date: string;
  category: Category | null;
}

export interface CheckResponse {
  correct: boolean;
}
