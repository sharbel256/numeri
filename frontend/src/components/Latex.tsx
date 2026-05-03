import katex from "katex";
import { useMemo } from "react";

interface Props {
  source: string;
  className?: string;
}

// Renders text containing inline ($...$) and block ($$...$$) LaTeX via KaTeX.
// Plain prose between math is rendered as-is, preserving line breaks.
export function Latex({ source, className }: Props) {
  const html = useMemo(() => renderMixed(source), [source]);
  return (
    <div
      className={className}
      dangerouslySetInnerHTML={{ __html: html }}
      style={{ whiteSpace: "pre-wrap" }}
    />
  );
}

function renderMixed(src: string): string {
  // Tokenize: $$...$$ (display), then $...$ (inline), then plain text.
  const out: string[] = [];
  let i = 0;
  while (i < src.length) {
    if (src.startsWith("$$", i)) {
      const end = src.indexOf("$$", i + 2);
      if (end === -1) {
        out.push(escapeHtml(src.slice(i)));
        break;
      }
      out.push(renderTex(src.slice(i + 2, end), true));
      i = end + 2;
      continue;
    }
    if (src[i] === "$") {
      const end = src.indexOf("$", i + 1);
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

function nextDelim(s: string, from: number): number {
  for (let j = from; j < s.length; j++) {
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
