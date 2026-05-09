from app.equivalence import check
from app.models import Difficulty


def _diff(answer: str, choices: list[str] | None = None) -> Difficulty:
    return Difficulty(
        question="placeholder",
        answer=answer,
        choices=choices or [answer, "0", "1", "2"],
    )


def test_exact_match():
    assert check(_diff("5"), "5")


def test_whitespace_tolerated():
    assert check(_diff("13"), "  13  ")


def test_wrong():
    assert not check(_diff("5"), "6")


def test_does_not_normalize():
    # Choice mode is exact string match — equivalent forms aren't equal.
    assert not check(_diff("1/2"), "0.5")
