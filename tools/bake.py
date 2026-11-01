#!/usr/bin/env python3
"""Bake bundled fonts into alpha atlases.

Runs in CI, not on the phone. Output per font:

    resources/fonts/<id>/atlas.txt    grid + glyph metrics
    resources/fonts/<id>/atlas.raw    8-bit alpha, row major
    resources/licenses/<id>/OFL.txt   licence, required by the SIL OFL

Atlas format (all integers, space separated):

    <atlas_w> <atlas_h> <baseline>
    <codepoint> <x> <y> <w> <h> <advance>      (one line per glyph)

Everything is ASCII 32..126. We do not need Unicode: the string being drawn
is always the account name, which the game already limits.
"""

import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

# family name -> (google/fonts path, atlas id)
FONTS = {
    "pressstart2p": "ofl/pressstart2p/PressStart2P-Regular.ttf",
    "silkscreen": "ofl/silkscreen/Silkscreen-Regular.ttf",
    "vt323": "ofl/vt323/VT323-Regular.ttf",
    "audiowide": "ofl/audiowide/Audiowide-Regular.ttf",
    "russoone": "ofl/russoone/RussoOne-Regular.ttf",
    "bungee": "ofl/bungee/Bungee-Regular.ttf",
    "rubikmonoone": "ofl/rubikmonoone/RubikMonoOne-Regular.ttf",
    "creepster": "ofl/creepster/Creepster-Regular.ttf",
}

RAW = "https://raw.githubusercontent.com/google/fonts/main/"
SIZE = 56          # rasterise big, the mod scales down
ATLAS_W = 512

CHARS = [chr(c) for c in range(32, 127)]


def fetch(rel_path: str, dest: str) -> None:
    if os.path.exists(dest):
        return
    url = RAW + rel_path
    res = subprocess.run(["curl", "-sSL", "-o", dest, url])
    if res.returncode != 0 or os.path.getsize(dest) < 1000:
        raise SystemExit(f"failed to download {url}")


def bake_glyphs(ttf: str) -> tuple[list, int, int]:
    font = ImageFont.truetype(ttf, SIZE)
    glyphs = []
    for ch in CHARS:
        img = Image.new("L", (SIZE * 3, SIZE * 3), 0)
        draw = ImageDraw.Draw(img)
        draw.text((SIZE, SIZE), ch, font=font, fill=255)
        box = img.getbbox()
        advance = int(round(font.getlength(ch)))
        if not box:
            glyphs.append((ord(ch), None, 0, 0, 0, advance))
            continue
        crop = img.crop(box)
        glyphs.append((ord(ch), crop, crop.width, crop.height, box[1], advance))

    ascent, descent = font.getmetrics()
    return glyphs, ascent, ascent + descent


def pack(glyphs: list) -> tuple[list, int, int]:
    """Shelf packing: simple, deterministic, good enough at this glyph count."""
    placed = []
    x = y = 0
    shelf_h = 0
    width = ATLAS_W
    for cp, img, w, h, _top, _adv in glyphs:
        if img is None:
            placed.append((cp, 0, 0, 0, 0, _adv))
            continue
        if x + w > width:
            x = 0
            y += shelf_h + 1
            shelf_h = 0
        placed.append((cp, x, y, w, h, _adv))
        x += w + 1
        shelf_h = max(shelf_h, h)
    height = 1
    while height < y + shelf_h:
        height *= 2
    return placed, width, height


def bake(font_id: str, rel_path: str) -> None:
    ttf_dir = os.path.join(ROOT, ".cache", "fonts")
    os.makedirs(ttf_dir, exist_ok=True)
    ttf = os.path.join(ttf_dir, os.path.basename(rel_path))
    fetch(rel_path, ttf)

    glyphs, baseline, line_height = bake_glyphs(ttf)
    placed, width, height = pack(glyphs)

    atlas = Image.new("L", (width, height), 0)
    for cp, img, _w, _h, _top, _adv in glyphs:
        if img is None:
            continue
        for item in placed:
            if item[0] == cp:
                atlas.paste(img, (item[1], item[2]))
                break

    out_dir = os.path.join(ROOT, "resources", "fonts", font_id)
    os.makedirs(out_dir, exist_ok=True)

    with open(os.path.join(out_dir, "atlas.raw"), "wb") as fh:
        fh.write(atlas.tobytes())

    lines = [f"{width} {height} {line_height} {baseline}"]
    for cp, gx, gy, gw, gh, adv in placed:
        lines.append(f"{cp} {gx} {gy} {gw} {gh} {adv}")
    with open(os.path.join(out_dir, "atlas.txt"), "w") as fh:
        fh.write("\n".join(lines) + "\n")

    lic_dir = os.path.join(ROOT, "resources", "licenses", font_id)
    os.makedirs(lic_dir, exist_ok=True)
    family_dir = "/".join(rel_path.split("/")[:2])
    lic = os.path.join(ttf_dir, font_id + "-OFL.txt")
    fetch(family_dir + "/OFL.txt", lic)
    with open(lic, "rb") as src, open(os.path.join(lic_dir, "OFL.txt"), "wb") as dst:
        dst.write(src.read())

    print(f"{font_id}: {width}x{height}, {os.path.getsize(os.path.join(out_dir, 'atlas.raw')) // 1024} KiB")


def main() -> None:
    only = sys.argv[1] if len(sys.argv) > 1 else None
    for font_id, rel_path in FONTS.items():
        if only and only != font_id:
            continue
        bake(font_id, rel_path)


if __name__ == "__main__":
    main()
