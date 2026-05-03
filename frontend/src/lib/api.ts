import type { Category, CheckResponse, Level, PublicPuzzle, TodaySummary } from "./types";

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(path, {
    ...init,
    headers: { "Content-Type": "application/json", ...(init?.headers ?? {}) },
  });
  if (!res.ok) {
    const body = await res.json().catch(() => ({}));
    throw new ApiError(res.status, body.error ?? "request_failed", body);
  }
  return res.json() as Promise<T>;
}

export class ApiError extends Error {
  constructor(
    public status: number,
    public code: string,
    public body: unknown,
  ) {
    super(`${status} ${code}`);
  }
}

export const api = {
  today: () => request<TodaySummary>("/api/today"),
  puzzle: (date: string, category: Category, level: Level) =>
    request<PublicPuzzle>(`/api/puzzle/${date}/${category}/${level}`),
  check: (date: string, category: Category, level: Level, answer: string) =>
    request<CheckResponse>("/api/check", {
      method: "POST",
      body: JSON.stringify({ date, category, level, answer }),
    }),
};
