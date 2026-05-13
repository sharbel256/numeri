from datetime import date

from fastapi.testclient import TestClient

from .conftest import write_category


def _seed(puzzles_dir, d=date(2026, 5, 2), category="algebra"):
    levels = {
        1: {
            "question": "easy",
            "answer": "5",
            "choices": ["3", "5", "7", "8"],
        },
        2: {
            "question": "medium",
            "answer": "5",
            "choices": ["3", "5", "7", "8"],
            "pitfalls": ["don't subtract 7"],
        },
        3: {
            "question": "hard",
            "answer": "3",
            "choices": ["1", "2", "3", "4"],
        },
    }
    write_category(puzzles_dir, d, category, levels)


def test_get_today_picks_one_category(client: TestClient, puzzles_dir, monkeypatch):
    # routes.py imports today_chicago into its namespace; patch the bound name.
    from app import routes as routes_mod

    monkeypatch.setattr(routes_mod, "today_chicago", lambda: date(2026, 5, 2))
    _seed(puzzles_dir, date(2026, 5, 2), "algebra")
    _seed(puzzles_dir, date(2026, 5, 2), "calculus")
    res = client.get("/api/today")
    assert res.status_code == 200
    body = res.json()
    assert body["date"] == "2026-05-02"
    assert body["category"] in ("algebra", "calculus")
    # Deterministic by date: a second call returns the same choice.
    assert client.get("/api/today").json()["category"] == body["category"]


def test_get_today_no_puzzles(client: TestClient, puzzles_dir, monkeypatch):
    from app import routes as routes_mod

    monkeypatch.setattr(routes_mod, "today_chicago", lambda: date(2099, 1, 1))
    res = client.get("/api/today")
    assert res.status_code == 200
    assert res.json() == {"date": "2099-01-01", "category": None}


def test_get_puzzle(client: TestClient, puzzles_dir):
    _seed(puzzles_dir)
    res = client.get("/api/puzzle/2026-05-02/algebra/2")
    assert res.status_code == 200
    body = res.json()
    assert body["category"] == "algebra"
    assert body["level"] == 2
    assert body["choices"] == ["3", "5", "7", "8"]
    assert "answer" not in body


def test_get_puzzle_404(client: TestClient, puzzles_dir):
    res = client.get("/api/puzzle/2099-01-01/algebra/1")
    assert res.status_code == 404
    assert res.json()["error"] == "puzzle_not_found"


def test_check_correct(client: TestClient, puzzles_dir):
    _seed(puzzles_dir)
    res = client.post(
        "/api/check",
        json={"date": "2026-05-02", "category": "algebra", "level": 2, "answer": "5"},
    )
    assert res.status_code == 200
    assert res.json() == {"correct": True}


def test_check_wrong(client: TestClient, puzzles_dir):
    _seed(puzzles_dir)
    res = client.post(
        "/api/check",
        json={"date": "2026-05-02", "category": "algebra", "level": 2, "answer": "6"},
    )
    assert res.status_code == 200
    assert res.json() == {"correct": False}


def test_request_id_propagated(client: TestClient, puzzles_dir):
    _seed(puzzles_dir)
    res = client.get(
        "/api/puzzle/2026-05-02/algebra/2",
        headers={"X-Request-ID": "test-id-123"},
    )
    assert res.headers.get("X-Request-ID") == "test-id-123"


def test_request_id_generated(client: TestClient, puzzles_dir):
    _seed(puzzles_dir)
    res = client.get("/api/puzzle/2026-05-02/algebra/2")
    assert res.headers.get("X-Request-ID")
