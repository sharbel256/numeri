from datetime import date as date_type
from typing import Annotated, cast

import structlog
from fastapi import APIRouter, Depends, Path

from app import stats
from app.config import Settings, get_settings
from app.equivalence import check as check_answer
from app.exceptions import PuzzleNotFound
from app.models import (
    Category,
    CheckRequest,
    CheckResponse,
    Level,
    PublicPuzzle,
    StatsResponse,
    TodaySummary,
)
from app.puzzles import category_for_date, load_category, to_public, today_utc

router = APIRouter(prefix="/api")
log = structlog.get_logger()


@router.get("/today", response_model=TodaySummary)
def get_today(settings: Settings = Depends(get_settings)) -> TodaySummary:
    today = today_utc()
    category = category_for_date(today, settings.puzzles_dir)
    log.info("today.fetched", date=today.isoformat(), category=category)
    return TodaySummary(date=today, category=category)


@router.get("/puzzle/{date}/{category}/{level}", response_model=PublicPuzzle)
def get_puzzle(
    date: date_type,
    category: Category,
    level: Annotated[int, Path(ge=1, le=3)],
    settings: Settings = Depends(get_settings),
) -> PublicPuzzle:
    day = load_category(date, category, settings.puzzles_dir)
    typed_level = cast(Level, level)
    log.info(
        "puzzle.fetched",
        date=date.isoformat(),
        category=category,
        level=typed_level,
    )
    return to_public(day, typed_level)


@router.post("/check", response_model=CheckResponse)
def check(req: CheckRequest, settings: Settings = Depends(get_settings)) -> CheckResponse:
    day = load_category(req.date, req.category, settings.puzzles_dir)
    if req.level not in day.levels:
        raise PuzzleNotFound(f"{req.date.isoformat()}/{req.category}/{req.level}")
    correct = check_answer(day.levels[req.level], req.answer)
    stats.record(req.date, req.category, req.level, req.answer)
    log.info(
        "answer.checked",
        date=req.date.isoformat(),
        category=req.category,
        level=req.level,
        correct=correct,
    )
    return CheckResponse(correct=correct)


@router.get("/stats/{date}/{category}/{level}", response_model=StatsResponse)
def get_stats(
    date: date_type,
    category: Category,
    level: Annotated[int, Path(ge=1, le=3)],
) -> StatsResponse:
    typed_level = cast(Level, level)
    counts = stats.get(date, category, typed_level)
    return StatsResponse(counts=counts, total=sum(counts.values()))
