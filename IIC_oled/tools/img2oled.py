#!/usr/bin/env python3
"""
img2oled.py — 图片取模工具，输出 SSD1306 128×64 垂直字节格式

用法：
  python3 img2oled.py <图片路径> [选项]

选项：
  -o, --output   输出文件名（不含扩展名），默认与输入同名
  -n, --name     C 数组变量名，默认 gBitmap
  -t, --threshold 二值化阈值 0-255，默认 128
  -i, --invert   反色（黑底白字 → 白底黑字）
  -p, --preview  在终端预览结果（用 █/空格 显示）
  --fit          缩放模式：contain(默认,保持比例留黑边) / stretch(拉伸) / cover(裁剪)

输出：
  <name>.h  — C 头文件，含数组定义和宽高宏
  <name>.bin — 原始二进制位图（可选调试用）
"""

import argparse
import sys
from pathlib import Path
from PIL import Image, ImageOps


def load_and_resize(path: str, fit: str, threshold: int, invert: bool) -> Image.Image:
    img = Image.open(path).convert("L")  # 灰度

    W, H = 128, 64
    if fit == "stretch":
        img = img.resize((W, H), Image.LANCZOS)
    elif fit == "cover":
        img = ImageOps.fit(img, (W, H), Image.LANCZOS)
    else:  # contain
        img.thumbnail((W, H), Image.LANCZOS)
        canvas = Image.new("L", (W, H), 0)
        ox = (W - img.width) // 2
        oy = (H - img.height) // 2
        canvas.paste(img, (ox, oy))
        img = canvas

    img = img.point(lambda p: 255 if p >= threshold else 0, "1")

    if invert:
        img = ImageOps.invert(img.convert("L")).convert("1")

    return img


def image_to_ssd1306(img: Image.Image) -> bytes:
    """
    转换为 SSD1306 垂直字节格式：
    buf[page * 128 + col], page=0..7, col=0..127
    每字节 bit0 = 上方像素，bit7 = 下方像素
    """
    W, H = 128, 64
    pages = H // 8
    buf = bytearray(pages * W)

    for page in range(pages):
        for col in range(W):
            byte = 0
            for bit in range(8):
                y = page * 8 + bit
                x = col
                pixel = img.getpixel((x, y))
                # PIL 1-bit: 0=black, 255=white；白色像素点亮
                if pixel:
                    byte |= (1 << bit)
            buf[page * W + col] = byte

    return bytes(buf)


def to_c_header(data: bytes, name: str, w: int = 128, h: int = 64) -> str:
    guard = name.upper() + "_H_"
    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <stdint.h>",
        "",
        f"#define {name.upper()}_WIDTH   {w}",
        f"#define {name.upper()}_HEIGHT  {h}",
        f"#define {name.upper()}_PAGES   {h // 8}",
        "",
        f"/* SSD1306 垂直字节格式: data[page * {w} + col] */",
        f"static const uint8_t {name}[{len(data)}] = {{",
    ]

    row_width = w  # 每行输出 128 字节（对应一个 page）
    for i in range(0, len(data), row_width):
        chunk = data[i:i + row_width]
        page = i // row_width
        hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
        lines.append(f"    /* page {page} */ {hex_str},")

    lines += [
        "};",
        "",
        f"#endif /* {guard} */",
        "",
    ]
    return "\n".join(lines)


def terminal_preview(img: Image.Image):
    W, H = 128, 64
    # 每两行合并为一行终端输出（终端字符高约为宽的 2 倍）
    print("+" + "-" * 64 + "+")
    for y in range(0, H, 2):
        row = ""
        for x in range(0, W, 2):  # 横向也缩小 2 倍
            p = img.getpixel((x, y))
            row += "█" if p else " "
        print("|" + row + "|")
    print("+" + "-" * 64 + "+")


def main():
    parser = argparse.ArgumentParser(description="图片取模 → SSD1306 128×64 C数组")
    parser.add_argument("image", help="输入图片路径")
    parser.add_argument("-o", "--output", default=None, help="输出文件名（不含扩展名）")
    parser.add_argument("-n", "--name", default="gBitmap", help="C 数组变量名")
    parser.add_argument("-t", "--threshold", type=int, default=128,
                        help="二值化阈值 0-255（默认 128）")
    parser.add_argument("-i", "--invert", action="store_true", help="反色")
    parser.add_argument("-p", "--preview", action="store_true", help="终端预览")
    parser.add_argument("--fit", choices=["contain", "stretch", "cover"],
                        default="contain", help="缩放模式（默认 contain）")
    args = parser.parse_args()

    img_path = Path(args.image)
    if not img_path.exists():
        print(f"错误：文件不存在 {img_path}", file=sys.stderr)
        sys.exit(1)

    out_stem = args.output or img_path.stem
    out_dir = Path(__file__).parent  # 输出到 tools/ 同目录

    print(f"加载图片: {img_path}")
    img = load_and_resize(str(img_path), args.fit, args.threshold, args.invert)
    print(f"处理完成: {img.width}×{img.height} 像素，模式={args.fit}，阈值={args.threshold}")

    if args.preview:
        terminal_preview(img)

    data = image_to_ssd1306(img)

    # 写 .h 文件
    h_path = out_dir / f"{out_stem}.h"
    h_path.write_text(to_c_header(data, args.name), encoding="utf-8")
    print(f"输出头文件: {h_path}")

    # 写 .bin 文件（调试用）
    bin_path = out_dir / f"{out_stem}.bin"
    bin_path.write_bytes(data)
    print(f"输出二进制: {bin_path}  ({len(data)} 字节)")

    print(f"\n在 empty.c 中使用方法：")
    print(f'  #include "tools/{out_stem}.h"')
    print(f'  OLED_drawBitmap(0, 0, {args.name.upper()}_WIDTH, {args.name.upper()}_HEIGHT, {args.name});')
    print(f'  OLED_display();')


if __name__ == "__main__":
    main()
