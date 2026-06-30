#!/usr/bin/env python3
"""
img2st7735.py — convert an image to an ST7735S RGB565 C array.

Output format matches ST7735_drawRGBBitmap():
  - row-major (top-to-bottom, left-to-right)
  - one uint16_t per pixel, RGB565
  - the C array stores each pixel as a 16-bit value; the driver sends the
    high byte first over SPI.

Usage:
  python3 img2st7735.py <image> [options]

Options:
  -o, --output    output file stem (no extension), default = input stem
  -n, --name      C array variable name, default = gImage
  -W, --width     target width  (default 128)
  -H, --height    target height (default 160)
  --fit           contain (default, keep ratio, pad black) / stretch / cover
  -p, --preview   print a small terminal preview

Produces <stem>.h with the array and WIDTH/HEIGHT macros.
"""

import argparse
import sys
from pathlib import Path
from PIL import Image, ImageOps


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def load_and_resize(path, fit, W, H):
    img = Image.open(path).convert("RGB")
    if fit == "stretch":
        img = img.resize((W, H), Image.LANCZOS)
    elif fit == "cover":
        img = ImageOps.fit(img, (W, H), Image.LANCZOS)
    else:  # contain
        img.thumbnail((W, H), Image.LANCZOS)
        canvas = Image.new("RGB", (W, H), (0, 0, 0))
        canvas.paste(img, ((W - img.width) // 2, (H - img.height) // 2))
        img = canvas
    return img


def to_c_header(img, name, W, H):
    guard = name.upper() + "_H_"
    out = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <stdint.h>",
        "",
        f"#define {name.upper()}_WIDTH   {W}",
        f"#define {name.upper()}_HEIGHT  {H}",
        "",
        "/* RGB565, row-major. Use with ST7735_drawRGBBitmap(). */",
        f"static const uint16_t {name}[{W * H}] = {{",
    ]
    px = img.load()
    for y in range(H):
        row = []
        for x in range(W):
            r, g, b = px[x, y]
            row.append(f"0x{rgb565(r, g, b):04X}")
        out.append("    " + ",".join(row) + ",")
    out += ["};", "", f"#endif /* {guard} */", ""]
    return "\n".join(out)


def preview(img, W, H):
    small = img.resize((min(W, 64), min(H, 32)))
    px = small.load()
    print("+" + "-" * small.width + "+")
    for y in range(small.height):
        line = ""
        for x in range(small.width):
            r, g, b = px[x, y]
            lum = (r * 30 + g * 59 + b * 11) // 100
            line += " .:-=+*#%@"[min(9, lum * 10 // 256)]
        print("|" + line + "|")
    print("+" + "-" * small.width + "+")


def main():
    ap = argparse.ArgumentParser(description="image -> ST7735 RGB565 C array")
    ap.add_argument("image")
    ap.add_argument("-o", "--output", default=None)
    ap.add_argument("-n", "--name", default="gImage")
    ap.add_argument("-W", "--width", type=int, default=128)
    ap.add_argument("-H", "--height", type=int, default=160)
    ap.add_argument("--fit", choices=["contain", "stretch", "cover"],
                    default="contain")
    ap.add_argument("-p", "--preview", action="store_true")
    args = ap.parse_args()

    src = Path(args.image)
    if not src.exists():
        print(f"error: file not found: {src}", file=sys.stderr)
        sys.exit(1)

    out_dir = Path(__file__).parent
    stem = args.output or src.stem

    print(f"loading {src}")
    img = load_and_resize(str(src), args.fit, args.width, args.height)
    print(f"resized to {img.width}x{img.height}, fit={args.fit}")
    if args.preview:
        preview(img, args.width, args.height)

    h_path = out_dir / f"{stem}.h"
    h_path.write_text(to_c_header(img, args.name, args.width, args.height),
                      encoding="utf-8")
    print(f"wrote {h_path}  ({args.width * args.height * 2} bytes of pixel data)")
    print("\nUsage in your code:")
    print(f'  #include "tools/{stem}.h"')
    print(f"  ST7735_drawRGBBitmap(0, 0, {args.name.upper()}_WIDTH, "
          f"{args.name.upper()}_HEIGHT, {args.name});")


if __name__ == "__main__":
    main()
