import { describe, expect, it } from "vitest";

import { prettyExpression } from "./expression";

describe("prettyExpression", () => {
  it("converts ^N to unicode superscripts", () => {
    expect(prettyExpression("x^2 + 1")).toBe("x² + 1");
  });

  it("handles grouped exponents", () => {
    expect(prettyExpression("x^{n+1}")).toBe("xⁿ⁺¹");
  });

  it("renders sqrt", () => {
    expect(prettyExpression("sqrt(x+1)")).toBe("√(x+1)");
  });

  it("substitutes pi and dot multiplication", () => {
    expect(prettyExpression("2*pi")).toBe("2·π");
  });

  it("returns empty for empty input", () => {
    expect(prettyExpression("")).toBe("");
  });
});
