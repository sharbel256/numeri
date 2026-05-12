import { useLayoutEffect, useRef, useState, type ReactNode } from "react";

interface Props {
  children: ReactNode;
  className?: string;
}

export function FitText({ children, className }: Props) {
  const outerRef = useRef<HTMLDivElement>(null);
  const innerRef = useRef<HTMLDivElement>(null);
  const [scale, setScale] = useState(1);

  useLayoutEffect(() => {
    const outer = outerRef.current;
    const inner = innerRef.current;
    if (!outer || !inner) return;

    const measure = () => {
      const cs = getComputedStyle(outer);
      const padX =
        (parseFloat(cs.paddingLeft) || 0) + (parseFloat(cs.paddingRight) || 0);
      const ow = outer.clientWidth - padX;
      const iw = inner.scrollWidth;
      if (ow <= 0 || iw === 0) return;
      setScale(iw > ow ? ow / iw : 1);
    };

    measure();
    const ro = new ResizeObserver(measure);
    ro.observe(outer);
    ro.observe(inner);
    return () => ro.disconnect();
  }, [children]);

  return (
    <div ref={outerRef} className={className} style={{ overflow: "hidden" }}>
      <div
        ref={innerRef}
        style={{
          display: "inline-block",
          whiteSpace: "nowrap",
          transformOrigin: "left center",
          transform: scale < 1 ? `scale(${scale})` : undefined,
        }}
      >
        {children}
      </div>
    </div>
  );
}
