import { useLayoutEffect, useRef, type ReactNode } from "react";

interface Props {
  children: ReactNode;
  className?: string;
  minScale?: number;
}

// Shrinks font-size (binary search) so content fits within the parent's
// clientHeight. Use inside a height-bounded container (e.g. `max-h-[...]`
// with `overflow-hidden`). Unlike FitText, this rewraps lines as the
// font shrinks, so it works for multi-line prose.
export function FitBlock({ children, className, minScale = 0.4 }: Props) {
  const ref = useRef<HTMLDivElement>(null);

  useLayoutEffect(() => {
    const el = ref.current;
    const parent = el?.parentElement;
    if (!el || !parent) return;

    const measure = () => {
      el.style.removeProperty("font-size");
      const maxH = parent.clientHeight;
      if (maxH <= 0 || el.scrollHeight <= maxH) return;
      const base = parseFloat(getComputedStyle(el).fontSize);
      let lo = minScale;
      let hi = 1;
      for (let i = 0; i < 12; i++) {
        const mid = (lo + hi) / 2;
        el.style.fontSize = `${base * mid}px`;
        if (el.scrollHeight <= maxH) lo = mid;
        else hi = mid;
      }
      el.style.fontSize = `${base * lo}px`;
    };

    measure();
    window.addEventListener("resize", measure);
    return () => window.removeEventListener("resize", measure);
  }, [children, minScale]);

  return (
    <div ref={ref} className={className}>
      {children}
    </div>
  );
}
