#!/usr/bin/env python3
"""
Dump /res/tf tile metadata as parsed by g.java constructor.

Outputs a readable text table (default), suitable for referencing in docs:
- renderType (v)
- imageIndex (T)
- transform (b)
- collisionType (l)
- aux (af) meaning:
    - if renderType==3: animationGroupId
    - if collisionType==3: maskBaseTileId (alias: s[tile]=s[aux])

Default input points at the decompiled jar resources in this repo:
  bounce_back/original_code/bounce_back_s60.jar.src/res/tf
"""

from __future__ import annotations

import argparse
import os
import struct


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


class Cursor:
    def __init__(self, data: bytes):
        self.data = data
        self.off = 0

    def take_u8(self) -> int:
        if self.off >= len(self.data):
            raise ValueError("EOF")
        v = self.data[self.off]
        self.off += 1
        return v

    def take_i8(self) -> int:
        return struct.unpack(">b", bytes([self.take_u8()]))[0]

    def take_bool(self) -> bool:
        # DataInputStream.readBoolean(): 1 byte, nonzero => true
        return self.take_u8() != 0

    def take_i32(self) -> int:
        if self.off + 4 > len(self.data):
            raise ValueError("EOF")
        (v,) = struct.unpack(">i", self.data[self.off : self.off + 4])
        self.off += 4
        return v


def parse_tf(tf_path: str) -> dict:
    chunks = read_container(tf_path)
    if len(chunks) < 2:
        raise ValueError("Expected /res/tf to have 2 chunks")

    c0 = Cursor(chunks[0])
    images_total = c0.take_i8()
    images_base = c0.take_i8()
    clamp_x = c0.take_bool()
    clamp_y = c0.take_bool()
    tile_w = c0.take_i8()
    tile_h = c0.take_i8()
    if tile_w == 12:
        tile_w = 16
        tile_h = 16
    tile_count = c0.take_i8()
    split_index = c0.take_i8()
    tile_id_mask = c0.take_u8()
    tile_flag_mask = c0.take_u8()
    bg_color_i32 = c0.take_i32()

    anim_count = c0.take_i8()
    anim_periods: list[int] = []
    anim_frames: list[list[int]] = []
    for _ in range(max(0, anim_count)):
        _unused = c0.take_i8()
        period = c0.take_i8()
        frame_count = c0.take_i8()
        frames = [c0.take_u8() for _ in range(max(0, frame_count))]
        anim_periods.append(period)
        anim_frames.append(frames)

    tiles = []
    cursor = c0
    c1 = Cursor(chunks[1])
    for tile_id in range(max(0, tile_count)):
        if tile_id == split_index:
            cursor = c1
        echo = cursor.take_i8()
        render_type = cursor.take_i8()
        image_index = cursor.take_u8()
        transform = cursor.take_u8()
        collision_type = cursor.take_i8()

        has_mask = collision_type == 1
        mask_file = None
        if has_mask:
            # Stored in the same order the constructor reads booleans: y then x.
            # Note: g.java stores to s[tile][x][y], while collision reads s[tile][y][x].
            mask_file = [[False] * tile_w for _ in range(tile_h)]
            for y in range(tile_h):
                for x in range(tile_w):
                    mask_file[y][x] = cursor.take_bool()

        aux = cursor.take_i32()

        tiles.append(
            {
                "tile_id": tile_id,
                "echo": echo,
                "render_type": render_type,
                "image_index": image_index,
                "transform": transform,
                "collision_type": collision_type,
                "has_mask": has_mask,
                "aux": aux,
                "mask_file": mask_file,
            }
        )

    return {
        "images_total": images_total,
        "images_base": images_base,
        "clamp_x": clamp_x,
        "clamp_y": clamp_y,
        "tile_w": tile_w,
        "tile_h": tile_h,
        "tile_count": tile_count,
        "split_index": split_index,
        "tile_id_mask": tile_id_mask,
        "tile_flag_mask": tile_flag_mask,
        "bg_color_i32": bg_color_i32,
        "anim_count": anim_count,
        "anim_periods": anim_periods,
        "anim_frames": anim_frames,
        "tiles": tiles,
    }


def main() -> int:
    default_tf = os.path.join(
        os.path.dirname(__file__),
        "original_code",
        "bounce_back_s60.jar.src",
        "res",
        "tf",
    )
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tf", default=default_tf, help="Path to /res/tf container")
    parser.add_argument(
        "--only-collision3",
        action="store_true",
        help="Print only tiles with collisionType==3",
    )
    parser.add_argument(
        "--dump-mask",
        type=int,
        action="append",
        default=[],
        help="Dump inline mask (collisionType==1) for given tileId (repeatable)",
    )
    args = parser.parse_args()

    tf = parse_tf(args.tf)
    print(
        "tf:"
        f" images={tf['images_total']} (base={tf['images_base']})"
        f" tile={tf['tile_w']}x{tf['tile_h']}"
        f" tileCount={tf['tile_count']} splitIndex={tf['split_index']}"
        f" clampX={int(tf['clamp_x'])} clampY={int(tf['clamp_y'])}"
        f" tileIdMask=0x{tf['tile_id_mask']:02X} flagMask=0x{tf['tile_flag_mask']:02X}"
        f" bgColor(i32)=0x{(tf['bg_color_i32'] & 0xFFFFFFFF):08X}"
        f" animCount={tf['anim_count']}"
    )
    print()
    print("tileId  v(render)  T(img)  b(transform)  l(coll)  aux(i32)        note")
    print("-----  ---------  ------  -----------  -------  --------  ---------------------------")
    for t in tf["tiles"]:
        if args.only_collision3 and t["collision_type"] != 3:
            continue
        note = ""
        if t["collision_type"] == 1:
            note = "mask inline (16x16 booleans)"
        elif t["collision_type"] == 2:
            note = "solid full-tile"
        elif t["collision_type"] == 3:
            note = f"mask alias from tileId={t['aux']}"
        if t["render_type"] == 3:
            if note:
                note += "; "
            note += f"animGroup={t['aux']}"
        print(
            f"{t['tile_id']:5d}  {t['render_type']:9d}  {t['image_index']:6d}  "
            f"0x{t['transform']:02X}       {t['collision_type']:7d}  "
            f"{t['aux']:8d}  {note}"
        )

    if args.dump_mask:
        tile_w = tf["tile_w"]
        tile_h = tf["tile_h"]
        wanted = set(args.dump_mask)
        print()
        print("inline masks (runtime orientation):")
        print("format: 16x16, '#'=solid, '.'=empty; x left->right, y top->bottom")
        for t in tf["tiles"]:
            tid = t["tile_id"]
            if tid not in wanted:
                continue
            if t["collision_type"] != 1 or not t["mask_file"]:
                print()
                print(f"tileId {tid}: no inline mask (collisionType={t['collision_type']})")
                continue
            mf = t["mask_file"]  # [fileY][fileX]
            # Runtime reads s[y][x], while load stored s[x][y] from file order.
            # Therefore runtime mask is transpose: runtime[y][x] = mf[x][y].
            print()
            print(f"tileId {tid}:")
            for y in range(tile_h):
                row = []
                for x in range(tile_w):
                    row.append("#" if mf[x][y] else ".")
                print("".join(row))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
