from datetime import date
from pathlib import Path

import pytest

from app.exceptions import PuzzleNotFound
from app.puzzles import (
    categories_for_date,
    category_for_date,
    load_category,
    to_public,
)

from .conftest import basic_levels, write_category


def test_load_category(puzzles_dir: Path):
    write_category(puzzles_dir, date(2026, 5, 2), "algebra", basic_levels())
    day = load_category(date(2026, 5, 2), "algebra", puzzles_dir)
    assert day.category == "algebra"
    assert set(day.levels.keys()) == {1, 2, 3}
    assert day.levels[2].answer == "5"


def test_load_missing_raises(puzzles_dir: Path):
    puzzles_dir.mkdir(parents=True, exist_ok=True)
    with pytest.raises(PuzzleNotFound):
        load_category(date(2099, 1, 1), "algebra", puzzles_dir)


def test_to_public_strips_answer(puzzles_dir: Path):
    """The redaction guarantee: PublicPuzzle never carries an `answer` field."""
    write_category(puzzles_dir, date(2026, 5, 2), "algebra", basic_levels())
    day = load_category(date(2026, 5, 2), "algebra", puzzles_dir)
    public = to_public(day, 2)
    dumped = public.model_dump()
    assert "answer" not in dumped
    assert "answer" not in public.model_dump_json()
    assert public.level == 2


def test_to_public_includes_level(puzzles_dir: Path):
    write_category(puzzles_dir, date(2026, 5, 2), "algebra", basic_levels())
    day = load_category(date(2026, 5, 2), "algebra", puzzles_dir)
    for lvl in (1, 2, 3):
        assert to_public(day, lvl).level == lvl


def test_missing_level_rejected(puzzles_dir: Path):
    """All three levels must be present in the YAML."""
    levels = basic_levels()
    del levels[3]
    write_category(puzzles_dir, date(2026, 5, 3), "algebra", levels)
    with pytest.raises(ValueError):
        load_category(date(2026, 5, 3), "algebra", puzzles_dir)


def test_categories_for_date(puzzles_dir: Path):
    d = date(2026, 5, 2)
    write_category(puzzles_dir, d, "algebra", basic_levels())
    write_category(puzzles_dir, d, "calculus", basic_levels())
    cats = categories_for_date(d, puzzles_dir)
    assert sorted(cats) == ["algebra", "calculus"]


def test_categories_for_empty_date(puzzles_dir: Path):
    puzzles_dir.mkdir(parents=True, exist_ok=True)
    assert categories_for_date(date(2099, 1, 1), puzzles_dir) == []


def test_category_for_date_round_robin(puzzles_dir: Path):
    # Publish all categories on two consecutive days; rotation should advance.
    all_cats = (
        "algebra",
        "geometry",
        "numbers",
        "logic",
        "probability",
        "calculus",
        "theory",
    )
    d1 = date(2026, 5, 2)
    d2 = date(2026, 5, 3)
    for c in all_cats:
        write_category(puzzles_dir, d1, c, basic_levels())
        write_category(puzzles_dir, d2, c, basic_levels())
    pick1 = category_for_date(d1, puzzles_dir)
    pick2 = category_for_date(d2, puzzles_dir)
    assert pick1 in all_cats and pick2 in all_cats
    # Adjacent days must hit adjacent slots in the canonical order.
    i1, i2 = all_cats.index(pick1), all_cats.index(pick2)
    assert (i2 - i1) % len(all_cats) == 1
    # Same date → same pick.
    assert category_for_date(d1, puzzles_dir) == pick1


def test_category_for_date_falls_through_missing(puzzles_dir: Path):
    # Only one category published; rotation must fall through to it.
    d = date(2026, 5, 2)
    write_category(puzzles_dir, d, "theory", basic_levels())
    assert category_for_date(d, puzzles_dir) == "theory"


def test_category_for_date_none_when_empty(puzzles_dir: Path):
    puzzles_dir.mkdir(parents=True, exist_ok=True)
    assert category_for_date(date(2099, 1, 1), puzzles_dir) is None


def test_choices_required(puzzles_dir: Path):
    levels = basic_levels()
    del levels[1]["choices"]
    write_category(puzzles_dir, date(2026, 5, 4), "algebra", levels)
    with pytest.raises(ValueError):
        load_category(date(2026, 5, 4), "algebra", puzzles_dir)


def test_choice_labels_length_mismatch_rejected(puzzles_dir: Path):
    levels = basic_levels()
    levels[1]["choice_labels"] = ["$5$", "$1$"]  # only 2, but choices has 4
    write_category(puzzles_dir, date(2026, 5, 5), "algebra", levels)
    with pytest.raises(ValueError):
        load_category(date(2026, 5, 5), "algebra", puzzles_dir)
