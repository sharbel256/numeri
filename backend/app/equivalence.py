from app.models import Difficulty


def check(difficulty: Difficulty, submitted: str) -> bool:
    """Compare a user submission against a difficulty's canonical answer."""
    return submitted.strip() == difficulty.answer.strip()
