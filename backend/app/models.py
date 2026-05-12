from datetime import date as date_type
from typing import Literal

from pydantic import BaseModel, Field, model_validator

Category = Literal[
    "algebra",
    "geometry",
    "numbers",
    "logic",
    "probability",
    "calculus",
    "theory",
]

Level = Literal[1, 2, 3]


class Hint(BaseModel):
    text: str
    cost: int = Field(default=15, ge=0, le=100)


class Difficulty(BaseModel):
    """A puzzle at a single difficulty level. Holds the answer."""

    question: str
    answer: str
    choices: list[str] = Field(min_length=2, max_length=6)
    choice_labels: list[str] | None = None
    hints: list[Hint] = Field(default_factory=list)
    walkthrough: str | None = None

    @model_validator(mode="after")
    def _validate_choice_labels(self) -> "Difficulty":
        if self.choice_labels is not None and len(self.choice_labels) != len(self.choices):
            raise ValueError("choice_labels must have the same length as choices")
        return self


class CategoryDay(BaseModel):
    """All three difficulty levels for a single (date, category)."""

    date: date_type
    category: Category
    levels: dict[Level, Difficulty]

    @model_validator(mode="after")
    def _all_levels_present(self) -> "CategoryDay":
        if set(self.levels.keys()) != {1, 2, 3}:
            raise ValueError("levels must contain exactly 1, 2, 3")
        return self


class PublicPuzzle(BaseModel):
    """One difficulty as the client sees it. Answer omitted by construction."""

    date: date_type
    category: Category
    level: Level
    question: str
    choices: list[str]
    choice_labels: list[str] | None
    hints: list[Hint]
    walkthrough: str | None


class TodaySummary(BaseModel):
    date: date_type
    category: Category | None


class CheckRequest(BaseModel):
    date: date_type
    category: Category
    level: Level
    answer: str = Field(min_length=1, max_length=512)


class CheckResponse(BaseModel):
    correct: bool


class StatsResponse(BaseModel):
    counts: dict[str, int]
    total: int
