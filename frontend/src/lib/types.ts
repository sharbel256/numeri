// Mirrors backend/app/models.py — keep in sync.

export type Category =
  | "algebra"
  | "geometry"
  | "numbers"
  | "logic"
  | "probability"
  | "calculus"
  | "theory";

export type InputKind = "numeric" | "expression";
export type Mode = "free" | "choice";
export type Level = 1 | 2 | 3;

export interface FreeInput {
  kind: InputKind;
  placeholder: string | null;
}

export interface Hint {
  text: string;
  cost: number;
}

export interface PublicPuzzle {
  date: string;
  category: Category;
  level: Level;
  question: string;
  free_input: FreeInput | null;
  choices: string[] | null;
  choice_labels: string[] | null;
  default_mode: Mode;
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
