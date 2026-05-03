from datetime import date as date_type
from typing import Literal

from pydantic import BaseModel, Field, model_validator

Category = Literal[
    "arithmetic",
    "algebra",
    "geometry",
    "numbers",
    "logic",
    "probability",
    "calculus",
    "words",
    "diffeq",
    "theory",
]

InputKind = Literal["numeric", "expression"]
Mode = Literal["free", "choice"]
Level = Literal[1, 2, 3]


class FreeInput(BaseModel):
    kind: InputKind
    placeholder: str | None = None


class Hint(BaseModel):
    text: str
    cost: int = Field(default=15, ge=0, le=100)


class Difficulty(BaseModel):
    """A puzzle at a single difficulty level. Holds the answer."""

    question: str
    answer: str
    free_input: FreeInput | None = None
    choices: list[str] | None = Field(default=None, min_length=2, max_length=6)
    default_mode: Mode
    hints: list[Hint] = Field(default_factory=list)
    walkthrough: str | None = None

    @model_validator(mode="after")
    def _validate_modes(self) -> "Difficulty":
        if self.free_input is None and self.choices is None:
            raise ValueError("difficulty must define at least one of free_input or choices")
        if self.default_mode == "free" and self.free_input is None:
            raise ValueError("default_mode='free' requires free_input")
        if self.default_mode == "choice" and self.choices is None:
            raise ValueError("default_mode='choice' requires choices")
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
    free_input: FreeInput | None
    choices: list[str] | None
    default_mode: Mode
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
