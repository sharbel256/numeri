import { Latex } from "./Latex";

interface Props {
  pitfalls: string[];
  revealed: number;
  onRequest: () => void;
  canRequest: boolean;
  state: "solving" | "correct" | "failed";
}

export function PitfallTrail({ pitfalls, revealed, onRequest, canRequest, state }: Props) {
  if (pitfalls.length === 0) return null;
  return (
    <aside className="flex flex-col gap-3.5 border-t border-rule pt-5 lg:border-t-0 lg:border-l lg:pl-8 lg:pt-0">
      <div className="flex justify-between font-mono text-[10px] uppercase tracking-[0.16em] text-ink-soft">
        <span>Hints</span>
        <span>
          {revealed} / {pitfalls.length}
        </span>
      </div>

      {revealed === 0 && state === "solving" && (
        <div className="font-display italic text-ink-soft text-sm leading-relaxed">
          stuck? ask for a hint — we'll tell you what <em>not</em> to try.
        </div>
      )}

      <div className="flex flex-col">
        {pitfalls.slice(0, revealed).map((p, i) => (
          <div
            key={i}
            className={`pb-4 mb-4 last:pb-0 last:mb-0 animate-fade-in ${
              i < revealed - 1 ? "border-b border-dashed border-rule" : ""
            }`}
          >
            <div className="font-mono text-[10px] tracking-[0.08em] text-accent mb-1.5">
              Hint {i + 1}
            </div>
            <Latex
              source={p}
              className="hint-prose font-display text-base leading-relaxed text-ink"
            />
          </div>
        ))}
      </div>

      {state === "solving" && canRequest && (
        <button
          onClick={onRequest}
          className="font-mono text-[11px] uppercase tracking-[0.12em] text-ink
            border border-rule bg-transparent hover:bg-paper-alt active:bg-paper-alt
            cursor-pointer px-4 py-3.5 min-h-12 lg:mt-auto
            flex items-center justify-center gap-2"
        >
          <span>ask for a hint</span>
          <span className="text-ink-faint normal-case tracking-normal">(h)</span>
        </button>
      )}
    </aside>
  );
}
