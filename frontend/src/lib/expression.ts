// Lightweight Unicode pretty-printer for the "reads as" preview line.
// For full LaTeX rendering (hints, walkthroughs), use KaTeX in <Latex />.

const SUP: Record<string, string> = {
  "0": "⁰",
  "1": "¹",
  "2": "²",
  "3": "³",
  "4": "⁴",
  "5": "⁵",
  "6": "⁶",
  "7": "⁷",
  "8": "⁸",
  "9": "⁹",
  "+": "⁺",
  "-": "⁻",
  n: "ⁿ",
  i: "ⁱ",
  x: "ˣ",
  y: "ʸ",
};

function toSup(s: string): string {
  return [...s].map((ch) => SUP[ch] ?? "^" + ch).join("");
}

export function prettyExpression(s: string): string {
  if (!s) return "";
  let t = s;
  t = t.replace(/\^\{([^}]+)\}/g, (_, g) => toSup(g));
  t = t.replace(/\^(-?\d+)/g, (_, g) => toSup(g));
  t = t.replace(/\^([a-zA-Z])/g, (_, g) => toSup(g));
  t = t.replace(/sqrt\(([^)]+)\)/gi, (_, g) => "√(" + g + ")");
  t = t.replace(/\*/g, "·");
  t = t.replace(/\bpi\b/gi, "π");
  return t;
}
