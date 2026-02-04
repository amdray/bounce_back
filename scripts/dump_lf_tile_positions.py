#!/usr/bin/env python3
"""
Find tileId occurrences in /res/lf tilemaps.

Outputs a readable text report (not JSON), useful for anchoring "this tile exists here"
claims in docs like COLLISION_CONTRACT.md.
"""

from __future__ import annotations

import argparse
import os
import struct
from collections import defaultdict


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
    parser.add_argument("--tile", type=int, action="append", required=True, help="tileId to search (repeatable)")
    parser.add_argument("--max-pos", type=int, default=5, help="Max positions to print per level/tile")
    args = parser.parse_args()

    wanted = set(args.tile)
    chunks = read_container(args.lf)
    num_levels = len(chunks) // 2

    # results[tileId][level] = (w,h,count,[positions...])
    results: dict[int, dict[int, tuple[int, int, int, list[tuple[int, int, int]]]]] = defaultdict(dict)

    for level in range(num_levels):
        tile_chunk = chunks[level * 2 + 1]
        if len(tile_chunk) < 2:
            continue
        height = tile_chunk[0]
        width = tile_chunk[1]
        data = tile_chunk[2 : 2 + height * width]
        if len(data) < height * width:
            data = data + bytes(height * width - len(data))

        counts = {tid: 0 for tid in wanted}
        positions: dict[int, list[tuple[int, int, int]]] = {tid: [] for tid in wanted}  # (x,y,flag80)

        for idx, b in enumerate(data):
            tid = b & 0x7F
            if tid not in wanted:
                continue
            counts[tid] += 1
            if len(positions[tid]) < args.max_pos:
                x = idx % width
                y = idx // width
                positions[tid].append((x, y, 1 if (b & 0x80) else 0))

        for tid in wanted:
            if counts[tid] > 0:
                results[tid][level] = (width, height, counts[tid], positions[tid])

    for tid in sorted(wanted):
        by_level = results.get(tid, {})
        total = sum(v[2] for v in by_level.values())
        print(f"tileId={tid} total={total} levels={len(by_level)}")
        for level in sorted(by_level.keys()):
            w, h, cnt, pos = by_level[level]
            pos_str = ", ".join(f"({x},{y},flag80={f})" for x, y, f in pos)
            print(f"  L{level:02d} map={w}x{h} cnt={cnt} first={pos_str}")
        print()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

