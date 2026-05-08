import type { Category } from "./types";

export interface PaletteKey {
  label: string;
  insert: string;
}

export const PALETTES: Partial<Record<Category, PaletteKey[]>> = {
  calculus: [
    { label: "π", insert: "pi" },
    { label: "e", insert: "e" },
    { label: "√", insert: "sqrt(" },
    { label: "x²", insert: "^2" },
    { label: "sin", insert: "sin(" },
    { label: "cos", insert: "cos(" },
    { label: "ln", insert: "ln(" },
  ],
  algebra: [
    { label: "√", insert: "sqrt(" },
    { label: "x²", insert: "^2" },
    { label: "π", insert: "pi" },
  ],
  geometry: [
    { label: "π", insert: "pi" },
    { label: "√", insert: "sqrt(" },
    { label: "x²", insert: "^2" },
  ],
};
