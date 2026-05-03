# numeri

A daily math puzzle. One category, one question, today only.

## Stack

- **Frontend** — Vite + React + TypeScript + Tailwind v4 + KaTeX
- **Backend** — FastAPI on `uv`, SymPy for symbolic answer checking, structlog
- **Storage** — YAML puzzle files on disk (LaTeX inside text fields)
- **Hosting** — Docker container on this machine, fronted by Cloudflare Tunnel at `numeri.sharbel.cc`

## Local development

```bash
just setup    # uv sync + bun install + .env scaffolding
just dev      # uvicorn on :8000, vite on :5173 (proxied to backend)
```

## Quality

```bash
just fmt      # ruff format + prettier
just check    # lint + format-check + types (backend & frontend)
just test     # pytest + vitest
```

## Production

```bash
just deploy-prod        # docker compose up on 127.0.0.1:4300
just deploy-prod-down
just logs prod
```

The Cloudflare Tunnel routes `numeri.sharbel.cc` → `http://127.0.0.1:4300`.

## Project layout

```
backend/
  app/                 # FastAPI app
  tests/

# Puzzle content lives outside the repo (default: ~/numeri-puzzles).
# Configure the host path via PUZZLES_DIR (dev) or PUZZLES_HOST_DIR (prod compose).
frontend/
  src/                 # Vite + React
docker-compose.prod.yml
Dockerfile
justfile
```
