from fastapi import Request
from fastapi.responses import JSONResponse
from structlog.contextvars import get_contextvars


class PuzzleNotFound(Exception):
    def __init__(self, date: str) -> None:
        super().__init__(f"puzzle not found: {date}")
        self.date = date


def _request_id() -> str | None:
    return get_contextvars().get("request_id")


async def puzzle_not_found_handler(_: Request, exc: Exception) -> JSONResponse:
    assert isinstance(exc, PuzzleNotFound)
    return JSONResponse(
        status_code=404,
        content={"error": "puzzle_not_found", "date": exc.date, "request_id": _request_id()},
    )


async def unhandled_exception_handler(_: Request, __: Exception) -> JSONResponse:
    return JSONResponse(
        status_code=500,
        content={"error": "internal_error", "request_id": _request_id()},
    )
