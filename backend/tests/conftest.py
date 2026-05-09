from collections.abc import Iterator
from datetime import date
from pathlib import Path

import pytest
from fastapi.testclient import TestClient
from ruamel.yaml import YAML

from app.config import Settings, get_settings
from app.main import create_app

_yaml = YAML(typ="safe")


@pytest.fixture
def puzzles_dir(tmp_path: Path) -> Path:
    return tmp_path / "puzzles"


@pytest.fixture
def settings(puzzles_dir: Path, monkeypatch: pytest.MonkeyPatch) -> Settings:
    puzzles_dir.mkdir(parents=True, exist_ok=True)
    s = Settings(puzzles_dir=puzzles_dir, log_format="console")
    get_settings.cache_clear()
    monkeypatch.setattr("app.config.get_settings", lambda: s)
    return s


@pytest.fixture
def client(settings: Settings) -> Iterator[TestClient]:
    app = create_app()
    app.dependency_overrides[get_settings] = lambda: settings
    with TestClient(app) as c:
        yield c


def write_category(puzzles_dir: Path, d: date, category: str, levels: dict[int, dict]) -> Path:
    day_dir = puzzles_dir / d.isoformat()
    day_dir.mkdir(parents=True, exist_ok=True)
    path = day_dir / f"{category}.yaml"
    payload = {
        "date": d.isoformat(),
        "category": category,
        "levels": levels,
    }
    with path.open("w", encoding="utf-8") as f:
        _yaml.dump(payload, f)
    return path


def basic_levels(answer: str = "5") -> dict[int, dict]:
    """Three minimal valid difficulty entries."""
    choices = [answer, "1", "2", "3"] if answer not in {"1", "2", "3"} else [answer, "9", "8", "7"]
    return {
        1: {"question": "easy", "answer": answer, "choices": choices},
        2: {"question": "medium", "answer": answer, "choices": choices},
        3: {"question": "hard", "answer": answer, "choices": choices},
    }
