import { renderToString } from "react-dom/server";
import { describe, expect, it } from "vitest";

import { Latex } from "./Latex";

function flatText(html: string): string {
  return html.replace(/<[^>]+>/g, "").replace(/\s+/g, " ").trim();
}

describe("Latex tokenizer", () => {
  it("treats \\$ inside math as escaped dollar, not a delimiter", () => {
    const src =
      "A bookstore buys a book wholesale for $\\$20$. It marks the price up by $50\\%$.";
    const html = renderToString(<Latex source={src} />);
    const text = flatText(html);
    expect(text).toContain("A bookstore buys a book wholesale for");
    expect(text).toContain("It marks the price up by");
    expect(text).toMatch(/\.\s*It marks/);
  });

  it("handles $$...$$ display blocks containing escaped dollars", () => {
    const src = "before $$\\$x = 5$$ after";
    const html = renderToString(<Latex source={src} />);
    const text = flatText(html);
    expect(text).toContain("before");
    expect(text).toContain("after");
  });

  it("leaves an unterminated $ as literal text", () => {
    const src = "cost is $5 only";
    const html = renderToString(<Latex source={src} />);
    const text = flatText(html);
    expect(text).toContain("cost is");
    expect(text).toContain("only");
  });
});
