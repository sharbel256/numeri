interface Props {
  choices: string[];
  picked: string | null;
  correctAnswer: string | null;
  onPick: (choice: string) => void;
  disabled: boolean;
  state: "solving" | "correct" | "failed";
  wrong: number;
}

export function ChoiceInput({
  choices,
  picked,
  correctAnswer,
  onPick,
  disabled,
  state,
  wrong,
}: Props) {
  return (
    <div className="flex flex-col gap-3 max-w-[620px]">
      <div className="flex items-baseline justify-between font-mono">
        <span className="text-[10px] uppercase tracking-[0.16em] text-ink-soft">
          {state === "correct" ? (
            <>
              <span className="text-accent inline-block animate-pop mr-1.5">✓</span>
              Solved
            </>
          ) : state === "failed" ? (
            "Answer"
          ) : (
            "Choose one"
          )}
        </span>
        <span className="text-[11px] text-ink-soft">
          {[0, 1, 2].map((i) => (
            <span key={i} className={`ml-1.5 ${i < wrong ? "text-accent" : "text-rule"}`}>
              {i < wrong ? "×" : "·"}
            </span>
          ))}
        </span>
      </div>

      <div className="grid grid-cols-1 sm:grid-cols-2 gap-2.5 pt-3">
        {choices.map((opt, idx) => {
          const letter = String.fromCharCode(65 + idx);
          const isPicked = picked === opt;
          const isCorrect =
            (state !== "solving" && correctAnswer === opt) ||
            (state === "correct" && isPicked);
          const isWrongPick = isPicked && state !== "correct" && !isCorrect;
          return (
            <button
              key={opt}
              disabled={disabled}
              onClick={() => onPick(opt)}
              className={`grid grid-cols-[32px_1fr] items-center gap-3.5 text-left
                border min-h-[60px] sm:min-h-[auto] py-4 sm:py-3 transition-all
                ${isCorrect ? "bg-accent-soft border-accent animate-glow-pulse" : "border-rule"}
                ${isWrongPick ? "opacity-40" : "opacity-100"}
                ${disabled ? "cursor-default" : "cursor-pointer hover:bg-paper-alt active:bg-paper-alt"}`}
            >
              <span
                className={`font-mono font-medium text-center h-full flex items-center justify-center border-r
                  ${isCorrect ? "text-accent border-accent" : "text-ink-soft border-rule"}`}
              >
                {letter}
              </span>
              <span className="font-mono font-medium text-lg sm:text-xl pr-3">{opt}</span>
            </button>
          );
        })}
      </div>
    </div>
  );
}
