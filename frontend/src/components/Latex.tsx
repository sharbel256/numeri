import katex from "katex";
import { useMemo } from "react";

interface Props {
  source: string;
  className?: string;
  inline?: boolean;
}

// Renders text containing inline ($...$) and block ($$...$$) LaTeX via KaTeX.
// Plain prose between math is rendered as-is, preserving line breaks.
// Pass `inline` for single-line contexts (e.g. a choice cell) so KaTeX bases
// can't soft-wrap between operators.
export function Latex({ source, className, inline }: Props) {
  const html = useMemo(() => renderMixed(source), [source]);
  return (
    <div
      className={className}
      dangerouslySetInnerHTML={{ __html: html }}
      style={{ whiteSpace: inline ? "nowrap" : "pre-wrap" }}
    />
  );
}

function renderMixed(src: string): string {
  // Tokenize: $$...$$ (display), then $...$ (inline), then plain text. Backslash
  // escapes (e.g. `\$` for a literal dollar inside math) are skipped so the
  // pairing of `$` delimiters survives them.
  const out: string[] = [];
  let i = 0;
  while (i < src.length) {
    if (src.startsWith("$$", i)) {
      const end = findClosing(src, i + 2, true);
      if (end === -1) {
        out.push(escapeHtml(src.slice(i)));
        break;
      }
      out.push(renderTex(src.slice(i + 2, end), true));
      i = end + 2;
      continue;
    }
    if (src[i] === "$") {
      const end = findClosing(src, i + 1, false);
      if (end === -1) {
        out.push(escapeHtml(src.slice(i)));
        break;
      }
      out.push(renderTex(src.slice(i + 1, end), false));
      i = end + 1;
      continue;
    }
    const next = nextDelim(src, i);
    out.push(escapeHtml(src.slice(i, next)));
    i = next;
  }
  return out.join("");
}

function findClosing(src: string, from: number, doubleDollar: boolean): number {
  for (let j = from; j < src.length; j++) {
    if (src[j] === "\\" && j + 1 < src.length) {
      j++;
      continue;
    }
    if (doubleDollar) {
      if (src.startsWith("$$", j)) return j;
    } else if (src[j] === "$") {
      return j;
    }
  }
  return -1;
}

function nextDelim(s: string, from: number): number {
  for (let j = from; j < s.length; j++) {
    if (s[j] === "\\" && j + 1 < s.length) {
      j++;
      continue;
    }
    if (s[j] === "$") return j;
  }
  return s.length;
}

function renderTex(tex: string, displayMode: boolean): string {
  try {
    return katex.renderToString(tex, { throwOnError: false, displayMode });
  } catch {
    return escapeHtml(tex);
  }
}

function escapeHtml(s: string): string {
  return s
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}
