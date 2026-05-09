from collections.abc import AsyncIterator
from contextlib import asynccontextmanager

import structlog
from fastapi import FastAPI, Request
from fastapi.staticfiles import StaticFiles
from starlette.middleware.base import BaseHTTPMiddleware
from structlog.contextvars import bind_contextvars, clear_contextvars
from ulid import ULID

from app.config import get_settings
from app.exceptions import (
    PuzzleNotFound,
    puzzle_not_found_handler,
    unhandled_exception_handler,
)
from app.logging import configure_logging
from app.routes import router

REQUEST_ID_HEADER = "X-Request-ID"


class RequestIdMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):  # type: ignore[override]
        rid = request.headers.get(REQUEST_ID_HEADER) or request.headers.get("cf-ray") or str(ULID())
        clear_contextvars()
        bind_contextvars(request_id=rid)
        try:
            response = await call_next(request)
        finally:
            response_id = rid
        response.headers[REQUEST_ID_HEADER] = response_id
        return response


@asynccontextmanager
async def lifespan(_: FastAPI) -> AsyncIterator[None]:
    settings = get_settings()
    configure_logging(settings)
    log = structlog.get_logger()
    log.info(
        "app.startup",
        environment=settings.environment,
        puzzles_dir=str(settings.puzzles_dir),
        static_dir=str(settings.static_dir) if settings.static_dir else None,
    )
    yield
    log.info("app.shutdown")


def create_app() -> FastAPI:
    settings = get_settings()
    app = FastAPI(title="numeri", lifespan=lifespan)

    app.add_middleware(RequestIdMiddleware)

    app.add_exception_handler(PuzzleNotFound, puzzle_not_found_handler)
    app.add_exception_handler(Exception, unhandled_exception_handler)

    app.include_router(router)

    if settings.static_dir is not None:
        app.mount(
            "/",
            StaticFiles(directory=settings.static_dir, html=True),
            name="static",
        )

    return app


app = create_app()
