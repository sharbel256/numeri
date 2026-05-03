from datetime import UTC, datetime
from datetime import date as date_type
from pathlib import Path

import yaml

from app.exceptions import PuzzleNotFound
from app.models import Category, CategoryDay, Level, PublicPuzzle


def today_utc() -> date_type:
    return datetime.now(UTC).date()


def _path_for(date: date_type, category: Category, puzzles_dir: Path) -> Path:
    return puzzles_dir / date.isoformat() / f"{category}.yaml"


def load_category(date: date_type, category: Category, puzzles_dir: Path) -> CategoryDay:
    path = _path_for(date, category, puzzles_dir)
    if not path.is_file():
        raise PuzzleNotFound(f"{date.isoformat()}/{category}")
    with path.open("r", encoding="utf-8") as f:
        data = yaml.safe_load(f)
    return CategoryDay.model_validate(data)


def categories_for_date(date: date_type, puzzles_dir: Path) -> list[Category]:
    """Categories that have a published puzzle file for this date."""
    day_dir = puzzles_dir / date.isoformat()
    if not day_dir.is_dir():
        return []
    found: list[Category] = []
    for entry in sorted(day_dir.iterdir()):
        if entry.suffix == ".yaml":
            # The Category Literal narrows valid filenames; ignore unrecognized ones.
            stem = entry.stem
            if stem in Category.__args__:  # type: ignore[attr-defined]
                found.append(stem)  # type: ignore[arg-type]
    return found


_ROTATION_EPOCH = date_type(2026, 4, 20)
_CATEGORY_ORDER: tuple[Category, ...] = Category.__args__  # type: ignore[attr-defined]


def category_for_date(date: date_type, puzzles_dir: Path) -> Category | None:
    """Round-robin through the canonical category order, one slot per day.

    Why: predictable cadence — users can anticipate tomorrow's category and the
    rotation is identical for everyone on a given UTC date.
    How to apply: if the rotation lands on a category with no puzzle published
    for `date`, fall through to the next available category so the day still
    has content rather than going dark.
    """
    available = set(categories_for_date(date, puzzles_dir))
    if not available:
        return None
    offset = (date - _ROTATION_EPOCH).days % len(_CATEGORY_ORDER)
    for i in range(len(_CATEGORY_ORDER)):
        candidate = _CATEGORY_ORDER[(offset + i) % len(_CATEGORY_ORDER)]
        if candidate in available:
            return candidate
    return None


def to_public(day: CategoryDay, level: Level) -> PublicPuzzle:
    """Strip the answer for one specific level before sending."""
    if level not in day.levels:
        raise PuzzleNotFound(f"{day.date.isoformat()}/{day.category}/{level}")
    d = day.levels[level]
    return PublicPuzzle(
        date=day.date,
        category=day.category,
        level=level,
        question=d.question,
        free_input=d.free_input,
        choices=d.choices,
        default_mode=d.default_mode,
        hints=d.hints,
        walkthrough=d.walkthrough,
    )
