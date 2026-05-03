from sympy import Rational, Symbol, simplify
from sympy.parsing.sympy_parser import (
    convert_xor,
    implicit_multiplication_application,
    parse_expr,
    standard_transformations,
)

from app.exceptions import InvalidExpression
from app.models import Difficulty

# Allow `x^2` instead of `x**2` and `3x` instead of `3*x` for natural authoring.
_TRANSFORMATIONS = standard_transformations + (
    convert_xor,
    implicit_multiplication_application,
)

_LOCALS = {ch: Symbol(ch) for ch in "abcnxyzt"} | {
    "C": Symbol("C"),
    "e": Symbol("e"),
    "pi": Symbol("pi"),
}


def _parse(expr: str):
    try:
        return parse_expr(
            expr,
            local_dict=_LOCALS,
            transformations=_TRANSFORMATIONS,
            evaluate=True,
        )
    except Exception as exc:
        raise InvalidExpression(expr, str(exc)) from exc


def check(difficulty: Difficulty, submitted: str) -> bool:
    """Compare a user submission against a difficulty's canonical answer."""
    submitted = submitted.strip()
    canonical = difficulty.answer.strip()

    if difficulty.default_mode == "choice" and difficulty.free_input is None:
        return submitted == canonical

    if submitted == canonical:
        return True

    kind = difficulty.free_input.kind if difficulty.free_input else "expression"

    if kind == "numeric":
        try:
            return Rational(submitted) == Rational(canonical)
        except (TypeError, ValueError):
            try:
                return abs(float(submitted) - float(canonical)) < 1e-6
            except ValueError:
                return False

    parsed_submitted = _parse(submitted)
    parsed_canonical = _parse(canonical)
    return simplify(parsed_submitted - parsed_canonical) == 0
