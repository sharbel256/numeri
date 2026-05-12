"""In-memory tally of submitted answers for today's puzzles only.

Wipes on UTC date rollover. Lost on process restart. Single-process only —
if you ever run multiple workers, swap this for shared storage (sqlite, redis).
"""

from __future__ import annotations

import threading
from collections import defaultdict
from datetime import date as date_type

from app.models import Category, Level

_lock = threading.Lock()
_current_date: date_type | None = None
_counts: dict[tuple[Category, Level], dict[str, int]] = defaultdict(lambda: defaultdict(int))


def _roll_if_stale(d: date_type) -> None:
    global _current_date
    if _current_date != d:
        _counts.clear()
        _current_date = d


def record(d: date_type, category: Category, level: Level, choice: str) -> None:
    with _lock:
        _roll_if_stale(d)
        _counts[(category, level)][choice] += 1


def get(d: date_type, category: Category, level: Level) -> dict[str, int]:
    with _lock:
        _roll_if_stale(d)
        return dict(_counts.get((category, level), {}))
