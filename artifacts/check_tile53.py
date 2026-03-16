#!/usr/bin/env python3
"""Verify raw byte offset of tile 53 transform field in res/tf binary."""
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'scripts'))
from dump_tf_tiles import parse_tf, read_container
import struct

tf_path = os.path.join(os.path.dirname(__file__), '..', 'original_code',
                       'bounce_back_s60.jar.src', 'res', 'tf')

tf = parse_tf(tf_path)
print(f"split_index={tf['split_index']}, tile_count={tf['tile_count']}")

# Reconstruct byte offset for each tile in chunk 0
# Chunk 0 tile data starts at c0 offset = header_bytes + anim_bytes
# Header = 15 bytes (see parse_tf)
# Animations = sum of (3 + frame_count) per anim
anim_bytes = sum(3 + len(frames) for frames in tf['anim_frames'])
tile_start_c0 = 15 + anim_bytes
print(f"Tile data starts at c0 offset {tile_start_c0} (file offset {tile_start_c0 + 2 + 2 + 2})")

off = 0
for t in tf['tiles']:
    tile_id = t['tile_id']
    if tile_id == tf['split_index']:
        off = 0  # switch to c1
    size = 265 if t['collision_type'] == 1 else 9
    if tile_id in [52, 53, 54, 55]:
        file_origin = tile_start_c0 + off + 2 + 2 + 2  # account for container header
        print(f"tile {tile_id:3d}: c0_off={off:5d}  file_off={file_origin:5d}=0x{file_origin:04x}"
              f"  render={t['render_type']} img={t['image_index']:3d} tf=0x{t['transform']:02x}"
              f"  col={t['collision_type']} aux={t['aux']}  size={size}")
    off += size

# Now read the raw bytes at those positions to cross-verify
chunks = read_container(tf_path)
c0 = chunks[0]
# Re-scan to find tile 53 offset in c0
off = tile_start_c0
for t in tf['tiles']:
    if t['tile_id'] == tf['split_index']:
        break
    size = 265 if t['collision_type'] == 1 else 9
    if t['tile_id'] == 53:
        raw = c0[off:off+9]
        print(f"\nRaw bytes for tile 53 in c0[{off}:{off+9}]:")
        print(' '.join(f'{b:02x}' for b in raw))
        print(f"  echo={raw[0]} render={raw[1]} img={raw[2]} transform=0x{raw[3]:02x} collision={raw[4]}")
        break
    off += size
