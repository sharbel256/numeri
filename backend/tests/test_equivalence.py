import pytest

from app.equivalence import check
from app.exceptions import InvalidExpression
from app.models import Difficulty, FreeInput


def _diff(answer: str, kind: str = "expression", choices: list[str] | None = None) -> Difficulty:
    free_input = FreeInput(kind=kind) if kind in {"numeric", "expression"} else None
    default_mode = "free" if free_input else "choice"
    return Difficulty(
        question="placeholder",
        answer=answer,
        free_input=free_input,
        choices=choices,
        default_mode=default_mode,
    )


def test_numeric_exact_match():
    assert check(_diff("5", kind="numeric"), "5")


def test_numeric_fractional_equivalence():
    assert check(_diff("1/2", kind="numeric"), "0.5")
    assert check(_diff("0.5", kind="numeric"), "1/2")


def test_numeric_whitespace_tolerated():
    assert check(_diff("13", kind="numeric"), "  13  ")


def test_numeric_wrong():
    assert not check(_diff("5", kind="numeric"), "6")


def test_expression_caret_and_implicit_mul():
    assert check(_diff("3*x**2 + 2"), "3x^2+2")


def test_expression_factored_equivalence():
    assert check(_diff("2*x + 4"), "2*(x+2)")


def test_expression_commutative():
    assert check(_diff("3*x**2 + 2"), "2 + 3x^2")


def test_expression_trig_identity():
    assert check(_diff("1"), "sin(x)**2 + cos(x)**2")


def test_expression_wrong():
    assert not check(_diff("x**2"), "x")


def test_choice_exact_match():
    p = _diff("101", kind="none", choices=["100", "101", "103", "107"])
    assert check(p, "101")
    assert not check(p, "100")


def test_invalid_expression_raises():
    with pytest.raises(InvalidExpression):
        check(_diff("x**2"), "((((")
