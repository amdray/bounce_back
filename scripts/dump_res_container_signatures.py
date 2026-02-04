#!/usr/bin/env python3
"""
Inspect /res/* containers (class c format) and report what each chunk looks like.

Use this to *prove* image/sound chunk formats (e.g., PNG magic) before bring-up on PSP.
"""

from __future__ import annotations

import argparse
import os
import struct
from dataclasses import dataclass
from collections import Counter


MAGICS = [
    (b"\x89PNG\r\n\x1a\n", "PNG"),
    (b"\xff\xd8\xff", "JPEG"),
    (b"GIF87a", "GIF87a"),
    (b"GIF89a", "GIF89a"),
    (b"OggS", "OGG"),
    (b"RIFF", "RIFF"),
    (b"ID3", "MP3(ID3)"),
]


@dataclass(frozen=True)
class ChunkInfo:
    index: int
    size: int
    magic: str
    head_hex: str


def detect_magic(data: bytes) -> str:
    for sig, name in MAGICS:
        if data.startswith(sig):
            return name
    return "unknown"


def read_container(path: str) -> list[bytes]:
    with open(path, "rb") as f:
        blob = f.read()

    # Validate "class c" container header:
    # u16 count, then count*u16 sizes, then chunk data.
    if len(blob) < 2:
        raise ValueError("File too small")
    (count,) = struct.unpack(">H", blob[:2])
    sizes_bytes = 2 + count * 2
    if count == 0 or sizes_bytes > len(blob):
        raise ValueError("Not a container (invalid chunkCount/size table)")

    sizes = list(struct.unpack(f">{count}H", blob[2:sizes_bytes]))
    off = sizes_bytes
    chunks: list[bytes] = []
    for size in sizes:
        if off + size > len(blob):
            raise ValueError("Not a container (truncated chunk data)")
        chunks.append(blob[off : off + size])
        off += size
    return chunks


def summarize(path: str, max_chunks: int) -> tuple[int, list[ChunkInfo]]:
    chunks = read_container(path)
    magic_counts = Counter(detect_magic(ch) for ch in chunks)
    infos: list[ChunkInfo] = []
    for i, ch in enumerate(chunks[:max_chunks]):
        infos.append(
            ChunkInfo(
                index=i,
                size=len(ch),
                magic=detect_magic(ch),
                head_hex=ch[:16].hex(),
            )
        )
    return len(chunks), infos, magic_counts


def main() -> int:
    default_res_dir = os.path.join(
        os.path.dirname(__file__),
        "original_code",
        "bounce_back_s60.jar.src",
        "res",
    )
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--res-dir", default=default_res_dir, help="Path to extracted /res directory")
    parser.add_argument(
        "--file",
        action="append",
        default=[],
        help="Specific res filename to inspect (repeatable). Default: common image containers.",
    )
    parser.add_argument("--max-chunks", type=int, default=8, help="Max chunks to print per file")
    args = parser.parse_args()

    if args.file:
        files = args.file
    else:
        files = ["if0", "if1", "if2", "ib0", "ic", "im", "b", "bg", "s", "r", "tf", "lf"]

    for name in files:
        path = os.path.join(args.res_dir, name)
        if not os.path.exists(path):
            print(f"{name}: (missing)")
            print()
            continue
        try:
            total, infos, magic_counts = summarize(path, args.max_chunks)
            print(f"{name}: chunks={total}")
            counts_str = " ".join(f"{k}={v}" for k, v in sorted(magic_counts.items()))
            print(f"  magic_counts: {counts_str}")
            for info in infos:
                print(
                    f"  #{info.index:03d} size={info.size:5d} magic={info.magic:8s} head16={info.head_hex}"
                )
            if total > args.max_chunks:
                print(f"  ... ({total - args.max_chunks} more)")
            print()
            continue
        except Exception:
            # Treat as a raw blob (e.g., /res/r demo script, /res/i icon).
            with open(path, "rb") as f:
                blob = f.read()
            magic = detect_magic(blob)
            print(f"{name}: raw size={len(blob)} magic={magic} head16={blob[:16].hex()}")
            print()
            continue

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
