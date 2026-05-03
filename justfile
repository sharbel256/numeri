# numeri – development task runner
# Usage: just <recipe>

set dotenv-load := true

# ── Setup ────────────────────────────────────────────────

# First-time local development setup
setup:
    @echo "setting up numeri local development..."
    just setup-backend
    just setup-frontend
    @if [ ! -f .env ]; then cp .env.example .env && echo "created .env from .env.example"; fi
    @echo ""
    @echo "setup complete. run: just dev"

# Set up the backend (Python / FastAPI via uv)
setup-backend:
    @echo "-- backend --"
    cd backend && uv sync
    @echo "backend dependencies installed"

# Set up the frontend (React / Vite via bun)
setup-frontend:
    @echo "-- frontend --"
    cd frontend && bun install
    @echo "frontend dependencies installed"

# ── Development ──────────────────────────────────────────

# Run both backend and frontend in parallel
dev:
    just dev-backend &
    just dev-frontend &
    wait

# Run backend dev server (uvicorn :8000)
dev-backend:
    cd backend && uv run uvicorn app.main:app --reload --host 0.0.0.0 --port 8000

# Run frontend dev server (vite :5173, proxies /api to backend)
dev-frontend:
    cd frontend && bun run dev

# ── Build ────────────────────────────────────────────────

# Build frontend for production (outputs to frontend/dist)
build-frontend:
    cd frontend && bun run build
    @echo "frontend built to frontend/dist"

# Build frontend then run backend serving the built dist
build-and-serve:
    just build-frontend
    cd backend && STATIC_DIR=../frontend/dist uv run uvicorn app.main:app --host 0.0.0.0 --port 8000

# ── Format ───────────────────────────────────────────────

# Format backend (ruff)
fmt-backend:
    cd backend && uv run ruff check . --fix && uv run ruff format .

# Format frontend (prettier)
fmt-frontend:
    cd frontend && bun run format

# Format everything
fmt: fmt-backend fmt-frontend

# ── Quality ──────────────────────────────────────────────

# Lint backend
lint-backend:
    cd backend && uv run ruff check .

# Lint frontend
lint-frontend:
    cd frontend && bun run lint

# Type-check backend
typecheck-backend:
    cd backend && uv run pyright app/

# Type-check frontend
typecheck-frontend:
    cd frontend && bun run typecheck

# Check backend (lint + format + types)
check-backend:
    cd backend && uv run ruff check . && uv run ruff format --check . && uv run pyright app/

# Check frontend (format + lint + types)
check-frontend:
    cd frontend && bun run format:check && bun run lint && bun run typecheck

# Run all checks
check: check-backend check-frontend

# ── Testing ──────────────────────────────────────────────

# Run backend tests
test-backend:
    cd backend && uv run pytest

# Run frontend tests
test-frontend:
    cd frontend && bun run test

# Run all tests
test: test-backend test-frontend

# ── Deploy ─────────────────────────────────────────────────

# Deploy production (binds 127.0.0.1:4300)
deploy-prod:
    docker compose -f docker-compose.prod.yml up --build -d
    @echo "production deploy running on http://127.0.0.1:4300"

# Stop production deployment
deploy-prod-down:
    docker compose -f docker-compose.prod.yml down

# View logs (usage: just logs or just logs prod)
logs target="prod":
    docker compose -f docker-compose.prod.yml logs -f

# ── Utilities ────────────────────────────────────────────

# Clean build artifacts
clean:
    rm -rf backend/__pycache__ backend/.pytest_cache backend/.ruff_cache
    rm -rf frontend/node_modules frontend/dist
    @echo "cleaned"
