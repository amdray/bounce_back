#!/usr/bin/env python3
"""
Dump /res/lf enemy records with the same parsing/normalization logic as h.b(level).

Why:
- Enemy records are 9 bytes each and feed directly into runtime arrays (ao/f/k/ag/s).
- Without a concrete dump, porting the enemy subsystem becomes guesswork.

Default input points at the decompiled jar resources in this repo:
  bounce_back/original_code/bounce_back_s60.jar.src/res/lf
"""

from __future__ import annotations

import argparse
import json
import os
import struct
from dataclasses import dataclass, asdict


def i8(value: int) -> int:
    return struct.unpack(">b", bytes([value & 0xFF]))[0]


def u8(value: int) -> int:
    return value & 0xFF


def read_container(path: str) -> list[bytes]:
    with open(path, "rb") as f:
        header = f.read(2)
        if len(header) != 2:
            raise ValueError("File too small for container header")
        (count,) = struct.unpack(">H", header)
        sizes_raw = f.read(count * 2)
        if len(sizes_raw) != count * 2:
            raise ValueError("File too small for chunk sizes")
        sizes = list(struct.unpack(f">{count}H", sizes_raw))
        chunks: list[bytes] = []
        for size in sizes:
            data = f.read(size)
            if len(data) != size:
                raise ValueError("Unexpected EOF while reading chunk data")
            chunks.append(data)
        return chunks


@dataclass(frozen=True)
class EnemyRecord:
    index: int
    enemy_type: int
    tile_x0: int
    tile_y0: int
    tile_x1: int
    tile_y1: int
    init_off_a_px: int
    init_off_b_px: int
    speed_a: int
    speed_b: int
    normalized: bool
    norm_tile_x0: int
    norm_tile_y0: int
    norm_tile_x1: int
    norm_tile_y1: int
    ag0_px: int
    ag1_px: int
    s0: int
    s1: int
    extent_x_px: int
    extent_y_px: int
    raw_hex: str


@dataclass(frozen=True)
class LevelSummary:
    level_index: int
    theme_id: int
    spawn_y_tiles: int
    spawn_x_tiles: int
    ball_type: int
    ar: int
    d_field: int
    enemy_count: int
    spawn_x_px: int
    spawn_y_px: int
    tilemap_h: int | None
    tilemap_w: int | None
    enemies: list[EnemyRecord]


def parse_level(chunks: list[bytes], level_index: int) -> LevelSummary:
    meta_idx = level_index * 2
    map_idx = meta_idx + 1
    if meta_idx >= len(chunks):
        raise IndexError(f"Missing metadata chunk for level {level_index} (idx={meta_idx})")
    if map_idx >= len(chunks):
        raise IndexError(f"Missing tilemap chunk for level {level_index} (idx={map_idx})")

    meta = chunks[meta_idx]
    if len(meta) < 7:
        raise ValueError(f"Level {level_index}: metadata chunk too small: {len(meta)} bytes")

    theme_id = i8(meta[0])
    spawn_y_tiles = i8(meta[1])
    spawn_x_tiles = i8(meta[2])
    ball_type = i8(meta[3])
    ar = i8(meta[4])
    d_field = i8(meta[5])
    enemy_count = i8(meta[6])

    offset_px = 8 if ball_type == 0 else 12
    spawn_x_px = spawn_x_tiles * 16 + offset_px
    spawn_y_px = spawn_y_tiles * 16 + offset_px

    enemies: list[EnemyRecord] = []
    off = 7
    for enemy_index in range(max(0, enemy_count)):
        if off + 9 > len(meta):
            raise ValueError(
                f"Level {level_index}: enemy[{enemy_index}] out of bounds "
                f"(need 9 bytes at offset {off}, have {len(meta) - off})"
            )
        raw = meta[off : off + 9]
        off += 9

        enemy_type = i8(raw[0])
        tile_x0 = i8(raw[1])
        tile_y0 = i8(raw[2])
        tile_x1 = i8(raw[3])
        tile_y1 = i8(raw[4])
        init_off_a_px = i8(raw[5]) * 16
        init_off_b_px = i8(raw[6]) * 16
        speed_a = i8(raw[7])
        speed_b = i8(raw[8])

        # Mirror h.b(level) normalization (h.java:218-240)
        norm_x0, norm_y0, norm_x1, norm_y1 = tile_x0, tile_y0, tile_x1, tile_y1
        m = init_off_a_px
        n = init_off_b_px
        b12 = speed_a
        b5 = speed_b
        normalized = False

        if norm_x0 > norm_x1 or norm_y0 > norm_y1:
            normalized = True
            norm_x0, norm_x1 = norm_x1, norm_x0
            norm_y0, norm_y1 = norm_y1, norm_y0
            m = (norm_x1 - norm_x0) * 16
            n = (norm_y1 - norm_y0) * 16
            if b5 > 0 or b12 > 0:
                b5 = -b5
                b12 = -b12

        # Runtime layout in h:
        #   ag[i][1] = m; ag[i][0] = n; s[i][1] = b12; s[i][0] = b5;
        ag0_px = n
        ag1_px = m
        s0 = b5
        s1 = b12

        extent_x_px = (norm_x1 - norm_x0) * 16
        extent_y_px = (norm_y1 - norm_y0) * 16

        enemies.append(
            EnemyRecord(
                index=enemy_index,
                enemy_type=enemy_type,
                tile_x0=tile_x0,
                tile_y0=tile_y0,
                tile_x1=tile_x1,
                tile_y1=tile_y1,
                init_off_a_px=init_off_a_px,
                init_off_b_px=init_off_b_px,
                speed_a=speed_a,
                speed_b=speed_b,
                normalized=normalized,
                norm_tile_x0=norm_x0,
                norm_tile_y0=norm_y0,
                norm_tile_x1=norm_x1,
                norm_tile_y1=norm_y1,
                ag0_px=ag0_px,
                ag1_px=ag1_px,
                s0=s0,
                s1=s1,
                extent_x_px=extent_x_px,
                extent_y_px=extent_y_px,
                raw_hex=raw.hex(),
            )
        )

    tilemap = chunks[map_idx]
    tilemap_h = u8(tilemap[0]) if len(tilemap) >= 1 else None
    tilemap_w = u8(tilemap[1]) if len(tilemap) >= 2 else None

    return LevelSummary(
        level_index=level_index,
        theme_id=theme_id,
        spawn_y_tiles=spawn_y_tiles,
        spawn_x_tiles=spawn_x_tiles,
        ball_type=ball_type,
        ar=ar,
        d_field=d_field,
        enemy_count=enemy_count,
        spawn_x_px=spawn_x_px,
        spawn_y_px=spawn_y_px,
        tilemap_h=tilemap_h,
        tilemap_w=tilemap_w,
        enemies=enemies,
    )


def main() -> int:
    default_lf = os.path.join(
        os.path.dirname(__file__),
        "original_code",
        "bounce_back_s60.jar.src",
        "res",
        "lf",
    )

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lf", default=default_lf, help="Path to /res/lf container")
    parser.add_argument("--level", type=int, default=None, help="Dump only one level index (0..21)")
    parser.add_argument("--json", action="store_true", help="Output as JSON")
    parser.add_argument("--out", default="-", help="Output file path (default: stdout)")
    args = parser.parse_args()

    chunks = read_container(args.lf)
    level_count = len(chunks) // 2

    levels: list[LevelSummary] = []
    if args.level is not None:
        levels.append(parse_level(chunks, args.level))
    else:
        for level_index in range(level_count):
            levels.append(parse_level(chunks, level_index))

    if args.out == "-":
        out = None
    else:
        os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
        out = open(args.out, "w", encoding="utf-8")
    try:
        if args.json:
            (out or __import__("sys").stdout).write(
                json.dumps([asdict(l) for l in levels], indent=2, ensure_ascii=False) + "\n"
            )
            return 0

        w = out.write if out is not None else __import__("sys").stdout.write
        for level in levels:
            w(
                f"level={level.level_index:02d} theme={level.theme_id} "
                f"spawnTiles=({level.spawn_x_tiles},{level.spawn_y_tiles}) "
                f"spawnPx=({level.spawn_x_px},{level.spawn_y_px}) "
                f"ballType={level.ball_type} ar={level.ar} D={level.d_field} "
                f"tilemap={level.tilemap_w}x{level.tilemap_h} enemies={level.enemy_count}\n"
            )
            for e in level.enemies:
                w(
                    "  "
                    f"#{e.index:02d} type={e.enemy_type} "
                    f"tiles=({e.tile_x0},{e.tile_y0})..({e.tile_x1},{e.tile_y1}) "
                    f"initOffPx=({e.init_off_a_px},{e.init_off_b_px}) "
                    f"speed=({e.speed_a},{e.speed_b}) "
                    f"norm={e.normalized} "
                    f"normTiles=({e.norm_tile_x0},{e.norm_tile_y0})..({e.norm_tile_x1},{e.norm_tile_y1}) "
                    f"ag=({e.ag0_px},{e.ag1_px}) s=({e.s0},{e.s1}) "
                    f"extentPx=({e.extent_x_px},{e.extent_y_px}) "
                    f"raw={e.raw_hex}\n"
                )
            w("\n")
        return 0
    finally:
        if out is not None:
            out.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
