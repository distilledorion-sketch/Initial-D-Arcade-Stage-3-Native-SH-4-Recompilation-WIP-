#!/usr/bin/env python3
"""Build a labeled GIF from native diagnostic frame captures."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=Path)
    parser.add_argument(
        "frames",
        nargs="+",
        metavar="LABEL=PATH",
        help="frame label and source bitmap",
    )
    parser.add_argument("--duration", type=int, default=1400)
    args = parser.parse_args()

    font = ImageFont.load_default(size=18)
    rendered: list[Image.Image] = []
    for item in args.frames:
        label, separator, path_text = item.partition("=")
        if not separator:
            parser.error(f"frame must be LABEL=PATH: {item}")
        path = Path(path_text)
        with Image.open(path) as source:
            frame = source.convert("RGB")
        canvas = Image.new("RGB", (frame.width, frame.height + 42), "#222327")
        canvas.paste(frame, (0, 42))
        draw = ImageDraw.Draw(canvas)
        draw.text((12, 10), label, font=font, fill="white")
        rendered.append(canvas.quantize(colors=256, method=Image.Quantize.MEDIANCUT))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    durations = [args.duration] * len(rendered)
    durations[-1] = args.duration * 2
    rendered[0].save(
        args.output,
        save_all=True,
        append_images=rendered[1:],
        duration=durations,
        loop=0,
        disposal=2,
        optimize=False,
    )
    print(f"wrote {args.output} ({len(rendered)} frames)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
