# Numeri — Daily Puzzle Generation Agent

You are maintaining a rolling 3-day buffer of upcoming puzzles for [Numeri](https://numeri.sharbel.cc), a daily math puzzle site. One category per day, three difficulty levels per category. Each run ensures that the next three UTC days each have a published puzzle file, generating any that are missing. Output is a YAML file per day written directly to the live puzzles directory; the FastAPI backend reads files from disk with no review step, so correctness is your responsibility.

This document is the full task spec. Follow it end-to-end on each run.

---

## 1. Goal & success criteria

Maintain a **rolling 3-day buffer** of published puzzles. On each run, ensure that valid `~/numeri-puzzles/{day}/{category}.yaml` files exist for each of the next three UTC days (`today+1`, `today+2`, `today+3`). For any day in that window that is missing, generate it: three difficulty levels (1, 2, 3), each with a verified-correct answer and 3–5 multiple-choice options.

**Per generated day, done means all of:**
- File exists at the right path.
- File loads via `app.models.CategoryDay.model_validate(...)` without error.
- Each level's `answer` has been independently re-derived and confirmed.
- For each level, exactly one choice equals the canonical answer (exact string match) and distractors are plausible.
- No level is a near-duplicate of a problem from the same category in the last 30 days.
- One line appended to the audit log (§7) for that day.

**Per run:** process days in order (`today+1` → `today+2` → `today+3`). If a single day cannot be completed after retries, log the FAIL line for that day and continue to the next day — do not abort the whole run. Partial progress preserves the buffer better than none. Exit non-zero only if at least one day in the window failed.

In steady state, two of the three days will already have files and only the new `today+3` is generated. After a missed night, the run backfills whichever days are missing.

---

## 2. Pre-flight

Run from `/Users/sharbel/code/numeri`. The backend code under `backend/app/` is the source of truth — read it if anything here looks ambiguous.

1. **Compute the target window (UTC):**
   ```
   today = datetime.now(UTC).date()
   targets = [today + timedelta(days=n) for n in (1, 2, 3)]
   ```
2. **For each `target` in `targets` (in order), do the per-day pipeline below.** Days are independent: a failure on one does not stop the next.
3. **Skip if already published:** if `~/numeri-puzzles/{target}/` exists and contains any `*.yaml`, log a SKIP line and move on. Never overwrite a manually-authored puzzle.
4. **Compute rotation category for `target`:**
   ```
   epoch = date(2026, 4, 20)
   order = ["algebra", "geometry", "numbers", "logic",
            "probability", "calculus", "theory"]
   category = order[(target - epoch).days % 7]
   ```
   Seven categories, one per day of the week. Mirror `backend/app/puzzles.py:43-64` exactly. If that file changes, this constant changes with it.
5. **Read recent history:** glob `~/numeri-puzzles/*/{category}.yaml` for the 30 days preceding `target`. Skim the questions so you can avoid repeating problem shapes, specific numbers, or the same trick. Include any days you generated earlier in this same run — those count as recent history for later targets.

---

## 3. Difficulty calibration

Three levels, one puzzle each. Calibrate by *thinking time on paper for someone comfortable with the category*, not raw difficulty.

| Level | Target time | Shape |
|------|------|------|
| 1 | ~2 minutes | Combine two ideas, or multiple steps of a single technique. Not a one-rule application — that's too easy. |
| 2 | ~5 minutes | Requires an insight, a non-obvious technique, or careful case analysis. |
| 3 | ~10–15 minutes | Multi-step problem demanding both insight and execution. Competition-flavored — the kind of problem where the right idea isn't obvious and a wrong path costs time. Should feel earned to crack. |

A good day's puzzle has clear *progression* — L2 builds naturally on L1, L3 is a noticeable step up. Avoid making L3 a tedious slog (long arithmetic, many cases). Difficulty should come from depth, not volume.

---

## 4. Per-category guidance

Every level is multiple choice. The user picks from the `choices` list; submission is checked by exact string match against the canonical `answer`. Each category has conventions for problem shapes — use these defaults, deviate only with reason.

**`choice_labels` (mandatory whenever a choice contains math).** `choices` are the *matching* strings — string-equal to the canonical `answer`. They render as a literal monospace string when no label is provided, which looks ugly for anything beyond a bare integer (`2*x*sin(x)` shows verbatim). For every level whose choices contain operators, function names, fractions, exponents, Greek letters, or any other math, **also provide a `choice_labels` list of the same length**, each entry a fully-LaTeX display string (typically `$...$`-wrapped). The frontend renders labels via KaTeX; if labels are absent, it falls back to the raw choice. Bare-integer choices (`"3"`, `"-1"`) may skip labels.

Order rule: `choice_labels[i]` is the visual form of `choices[i]`. Don't shuffle independently. The shuffle in §5b.5 must keep them paired.

### `algebra`
- Equations, polynomials, systems, inequalities, functions. Also absorbs arithmetic flavor — mental math, fractions, percentages, ratios, sequences — when a level wants a lighter shape.
- Answers: bare integers when possible; otherwise simple rationals (`"1/2"`) or compact closed forms (`"2*sqrt(3) + 1"`).
- L1: quadratic, two-variable system, multi-step linear with a twist, or an arithmetic-flavored problem about digits/divisibility/clever decomposition. L2: clever factoring, substitution, system that needs a non-obvious move. L3: functional equation, parameterized family, problem requiring identification of an invariant or symmetry.

### `geometry`
- Angles, lengths, areas, volumes, similarity. Plane and solid both fine.
- Answers: numeric usually; closed forms involving `sqrt`, `pi` when cleanest.
- Describe figures precisely in text (KaTeX). No images. Be unambiguous about what's given.
- L3 often hinges on a key auxiliary construction or invariant.

### `numbers`
- Number theory: primes, divisibility, modular arithmetic, gcd/lcm, digits.
- Answers: integers or simple rationals.
- L1: modular reasoning, gcd/lcm with a twist. L2: structural argument, often "find all" reduced to a single count. L3: problem combining modular arithmetic with a counting or extremal argument; competition-style.

### `logic`
- Deductive puzzles, knights/knaves, grid logic, parity arguments. Also absorbs word problems — short real-world stories that resolve to a numerical answer — when a level wants a narrative framing.
- 3–5 options — logic often has a small natural option set.
- Word-problem flavor: story should be short and unambiguous, no trick wording, double-check unit consistency.
- Verification is harder — be extra careful and write your reasoning out fully before finalizing.

### `probability`
- Discrete probability, expected value, simple combinatorics.
- Answers: prefer exact rationals (`"4/9"`) over decimals.
- State the sample space explicitly. Avoid ambiguity about independence, replacement, ordering.

### `calculus`
- Derivatives, integrals, limits, optimization, series. Also absorbs differential equations — ODEs, separable, linear first-order, second-order constant-coefficient — typically at L2/L3.
- Answers: closed-form expressions; distractors are easy here (sign error, missing chain factor, off-by-one in power).
- L1: chain/product/quotient rule, u-substitution, basic limit, or a linear first-order ODE with integrating factor / simple separable IVP. L2: integration by parts, optimization with constraints, non-trivial limit, second-order constant-coefficient ODE (homogeneous + particular), Bernoulli, exact equation. L3: trig/partial-fraction integral, series convergence with a subtle test, multi-step optimization, parametric or related-rates with a twist, second-order IVP with a non-trivial forcing term, system of two ODEs, or substitution-based reduction.
- For ODE problems, specify boundary/initial conditions explicitly when the answer is a particular solution.

### `theory`
- Set theory, abstract algebra (groups), graph theory, combinatorial identities, proofs reduced to a count.
- Answers: integer counts, or which property holds (named string).
- L3 often: "how many distinct X up to Y" — the kind of problem that has a closed-form once you see the structure.

---

## 5. Verification (mandatory, before writing the file)

For each level, verify the canonical `answer` is correct **using a method independent of how you generated it**. Re-deriving with the same approach doesn't count.

### 5a. Verify the canonical answer with SymPy

Even though the user-facing check is exact string match, you need to verify *your own* answer is right before publishing. For numeric or symbolic answers, use SymPy to confirm:

```python
from sympy import sympify, simplify
from sympy.parsing.sympy_parser import (
    parse_expr, standard_transformations, implicit_multiplication_application,
    convert_xor,
)

TRANSFORMATIONS = standard_transformations + (
    implicit_multiplication_application, convert_xor,
)

def equiv_expr(a: str, b: str) -> bool:
    ea = parse_expr(a, transformations=TRANSFORMATIONS)
    eb = parse_expr(b, transformations=TRANSFORMATIONS)
    return simplify(ea - eb) == 0

def equiv_num(a: str, b: str, tol: float = 1e-6) -> bool:
    return abs(float(sympify(a)) - float(sympify(b))) < tol
```

Solve the puzzle in code (or by independent hand calculation, then encode the result), then assert the stored answer equals the computed answer. For categories that resist symbolic solving (logic, some probability), solve it twice by hand using two different framings and confirm they agree.

### 5b. Multiple-choice checks

Apply to every level:

1. **Exactly one correct option.** The canonical `answer` string must appear verbatim as exactly one element of `choices`. The runtime check is exact string match (`backend/app/equivalence.py`), so character-for-character matters — no leading/trailing whitespace, no `0.5` vs `1/2` mismatch.
2. **Plausible distractors.** Each wrong choice should reflect a believable mistake (sign error, off-by-one, wrong rule, dropped factor) — not random noise. Distractors should also be independently *not equivalent* to the canonical answer under SymPy (a distractor `"0.5"` paired with answer `"1/2"` is a bug — both are correct mathematically but only one matches the string check, so the user gets penalized for picking a "right" answer).
3. **Consistent form.** All choices in the same shape — don't mix `"0.5"` with `"1/2"`. Don't include LaTeX delimiters in `choices` (use `choice_labels` for display).
4. **No visual leakage.** The correct choice shouldn't be the only one with units, the only one in simplest form, the longest, or otherwise visually distinct.
5. **Count.** 4 choices by default; 3 or 5 if the problem demands. Never below 2 or above 6 (model constraint).
6. **Shuffle order** so the correct slot varies.

### 5c. Retry policy

If a level fails verification, regenerate it. Up to 3 attempts per level. If still failing after 3, abort **just that day** — log the FAIL line for it and continue to the next target day in the window. The run as a whole exits non-zero if any day in the window failed, but other days in the window must still be attempted.

---

## 6. Write & validate

For each target day being generated:

1. Write YAML to `~/numeri-puzzles/{target}/{category}.yaml`. Use `ruamel.yaml` or `pyyaml` with block style for multi-line `question` and `walkthrough` fields. UTF-8, no trailing whitespace.
2. **Validate by loading via the project's own model:**
   ```bash
   cd /Users/sharbel/code/numeri/backend
   uv run python -c "
   import sys, yaml
   from app.models import CategoryDay
   data = yaml.safe_load(open('$HOME/numeri-puzzles/{target}/{category}.yaml'))
   CategoryDay.model_validate(data)
   print('OK')
   "
   ```
3. If validation fails: delete the file, log the FAIL line for that day, and move on to the next target day. Do not leave a half-broken file in place.

---

## 7. Audit log

Append one line **per target day processed** to `~/numeri-puzzles/_generation_log.md` (so a single run typically appends 1–3 lines). The ISO date is the puzzle's target day, not the run date.

```
- 2026-05-04 logic — OK (4 / 4 / 3 choices) — verified
- 2026-05-04 logic — SKIP (file already exists)
- 2026-05-04 logic — FAIL (L3 verification: distractor "1/2" equiv to answer "0.5")
```

Format: ISO date, category, status, brief detail. Keep each line under ~120 chars.

---

## 8. Hints and walkthroughs

**Hints:** 1–3 per level, escalating from gentle nudge → relevant formula/identity → near-direct setup. Costs 10–20 (default 15). L1 may have just one hint; L3 should have three.

Pattern (calculus L3 in `~/numeri-puzzles/2026-05-03/calculus.yaml` is a good template):
- Hint 1: name the technique ("use the product rule").
- Hint 2: identify the pieces ("let `u = x^3`, `v = sin(x)`").
- Hint 3: give the derivatives of the pieces.

**Hints render in a narrow sidebar (~280px on desktop, full-width on mobile).** Math that would overflow that column scrolls horizontally inside the hint block, which is ugly. To keep hints visually clean:
- Use **inline math `$...$` only** in hints. No `$$...$$` display equations.
- Avoid wide constructs in hints: `\dfrac` (use `\tfrac` or `a/b`), long sums or integrals with bounds, multi-line `\cases`, anything that would render wider than ~30 monospace characters.
- If a step genuinely needs a big equation, *split it across two hints* or push it into the walkthrough. The hint should hand the user the idea; the walkthrough does the algebra.

**Walkthrough:** the key step rendered in LaTeX, plus the final answer. Doesn't need to be a full essay — show the move that unlocks the problem. Prefer `$$...$$` for display equations, `$...$` inline. The walkthrough renders in the wide main column, so display math is fine here.

---

## 9. Style rules

- **KaTeX everywhere math appears.** This is non-negotiable. Any operator, function name, fraction, exponent, Greek letter, integral, sum, root, etc. anywhere in `question`, `hints[*].text`, `walkthrough`, or `choice_labels[*]` **must live inside `$...$` (inline) or `$$...$$` (display)**. No bare `2*x`, `pi/4`, `sin(x)`, `x^2` outside math delimiters — the user sees the literal characters, which looks broken. Plain English narration around the math is fine; the math itself is always wrapped. Escape backslashes correctly in YAML (use block scalars `|` for multi-line content with LaTeX — backslashes don't need escaping inside block scalars).
- **Numeric answers:** prefer exact rationals as strings (`"1/2"`, `"7"`) over decimals.
- **Choices (matching strings):** strings, even for numbers (`"3"`, not `3`). All choices in the same form (don't mix `"0.5"` with `"1/2"`). The canonical `answer` string must appear verbatim as one of the choices, character-for-character — the backend check is exact string match (`backend/app/equivalence.py`). **No LaTeX delimiters inside `choices`** — those go in `choice_labels`.
- **`choice_labels` (display strings):** parallel array, same length as `choices`, paired by index. Each entry is fully LaTeX-formatted, almost always `$...$`-wrapped. Required whenever any choice contains math. Example pairing:
  ```yaml
  choices:
    - "2*x*sin(x) + x**2*cos(x)"
    - "2*x*cos(x) + x**2*sin(x)"
    - "x**2*cos(x)"
    - "2*x*sin(x) - x**2*cos(x)"
  choice_labels:
    - "$2x\\sin(x) + x^{2}\\cos(x)$"
    - "$2x\\cos(x) + x^{2}\\sin(x)$"
    - "$x^{2}\\cos(x)$"
    - "$2x\\sin(x) - x^{2}\\cos(x)$"
  ```
  When you shuffle (§5b.6), shuffle the *paired* `(choices[i], choice_labels[i])` together — never independently.
- **Question phrasing:** clear, self-contained, no references to "yesterday's puzzle" or external context. State all definitions you use.
- **Beauty test (apply before finalizing each level):** mentally render every text field. If any equation would appear as raw `2*x` or `pi/4`, you forgot delimiters. If a hint contains an equation that would clearly overflow ~30 chars of monospace, rewrite it shorter or split the hint.

---

## 10. Quick reference

- Categories (in rotation order): `algebra, geometry, numbers, logic, probability, calculus, theory`
- Rotation epoch: `2026-04-20` (`backend/app/puzzles.py:43`)
- Output dir: `~/numeri-puzzles/{YYYY-MM-DD}/{category}.yaml`
- Audit log: `~/numeri-puzzles/_generation_log.md`
- Models: `backend/app/models.py`
- Equivalence rules: `backend/app/equivalence.py`
- Existing examples: `~/numeri-puzzles/2026-05-08/probability.yaml`, `~/numeri-puzzles/2026-05-06/calculus.yaml` (uses `choice_labels`)
