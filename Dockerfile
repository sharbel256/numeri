# syntax=docker/dockerfile:1.7

# ---------- Stage 1: build the frontend ----------
FROM oven/bun:1 AS frontend
WORKDIR /build
COPY frontend/package.json frontend/bun.lock* ./
RUN bun install --frozen-lockfile || bun install
COPY frontend/ .
RUN bun run build

# ---------- Stage 2: backend image ----------
FROM python:3.12-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    curl ca-certificates && rm -rf /var/lib/apt/lists/* \
 && curl -LsSf https://astral.sh/uv/install.sh | sh \
 && ln -s /root/.local/bin/uv /usr/local/bin/uv

WORKDIR /app

COPY backend/pyproject.toml backend/uv.lock* ./
RUN uv sync --frozen --no-dev || uv sync --no-dev

COPY backend/app ./app
COPY --from=frontend /build/dist ./static

ENV ENVIRONMENT=prod \
    LOG_FORMAT=json \
    LOG_LEVEL=INFO \
    PUZZLES_DIR=/app/puzzles \
    STATIC_DIR=/app/static \
    PYTHONDONTWRITEBYTECODE=1

EXPOSE 8000

CMD ["uv", "run", "uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000", "--proxy-headers"]
