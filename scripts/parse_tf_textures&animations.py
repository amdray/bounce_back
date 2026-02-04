#!/usr/bin/env python3
"""
Parse tf file to understand tile metadata and image indices.
Based on g.java lines 212-234.
"""

import struct

def read_resource_file(filepath):
    """Read a resource file in the custom format used by class c."""
    with open(filepath, 'rb') as f:
        num_chunks = struct.unpack('>H', f.read(2))[0]
        chunk_sizes = []
        for i in range(num_chunks):
            size = struct.unpack('>H', f.read(2))[0]
            chunk_sizes.append(size)
        chunks = []
        for size in chunk_sizes:
            data = f.read(size)
            chunks.append(data)
        return chunks

def parse_tf_file(filepath):
    """Parse the tf (tile format) file."""
    chunks = read_resource_file(filepath)

    # tf file has 2 chunks (from h.java:254-256)
    # arrayOfByte3 = c2.a() - first chunk (paramArrayOfbyte1)
    # arrayOfByte4 = c2.a() - second chunk (paramArrayOfbyte2)

    print(f'Number of chunks in tf: {len(chunks)}')

    data = chunks[0]  # First chunk
    offset = 0

    # From g.java constructor lines 147-166:
    # b1 = readByte() - total number of images
    # b2 = readByte() - number of images from first file
    # d = readByte() - wrap horizontal flag
    # X = readByte() - wrap vertical flag
    # f = readByte() - tile width
    # A = readByte() - tile height
    # h = readByte() - number of tiles
    # b3 = readByte() - split point (where to switch from paramArrayOfbyte1 to paramArrayOfbyte2)
    # z = readByte() - tile ID mask (0xFF & 0x7F = 127)
    # q = readByte() - background flag mask (128)
    # i = readInt() - background color

    b1 = data[offset]; offset += 1  # total images
    b2 = data[offset]; offset += 1  # images from if0
    wrap_h = data[offset]; offset += 1
    wrap_v = data[offset]; offset += 1
    tile_w = data[offset]; offset += 1
    tile_h = data[offset]; offset += 1
    num_tiles = data[offset]; offset += 1
    split_point = data[offset]; offset += 1
    tile_mask = data[offset]; offset += 1
    bg_mask = data[offset]; offset += 1
    bg_color = struct.unpack('>I', data[offset:offset+4])[0]; offset += 4

    print(f'Total images (b1): {b1}')
    print(f'Images from if0 (b2): {b2}')
    print(f'Images from if1: {b1 - b2}')
    print(f'Tile size: {tile_w}x{tile_h}')
    print(f'Number of tiles (h): {num_tiles}')
    print(f'Split point: {split_point}')
    print(f'Tile ID mask: 0x{tile_mask:02x}')
    print(f'Background mask: 0x{bg_mask:02x}')
    print(f'Background color: 0x{bg_color:08x}')
    print()

    # Skip animation data (lines 186-201)
    # U = readByte() - number of animations
    U = data[offset]; offset += 1
    print(f'Number of animations (U): {U}')

    if U > 0:
        for i in range(U):
            offset += 1  # unused byte
            O = data[offset]; offset += 1  # frame duration
            num_frames = data[offset]; offset += 1
            print(f'  Animation {i}: {num_frames} frames, duration {O}')
            offset += num_frames  # skip frame data
    print()

    # Now parse tile data (lines 212-234)
    print(f'Tile metadata (starting at offset {offset}):')
    print(f'{"ID":<4} {"Type":<6} {"Image":<6} {"Flags":<6} {"Coll":<6}')
    print('-' * 40)

    tile_data = {}
    for tile_id in range(num_tiles):
        if tile_id == split_point:
            print(f'  [Split point at tile {tile_id} - switching to chunk 2]')
            # Switch to second chunk (paramArrayOfbyte2)
            data = chunks[1]
            offset = 0

        unused = data[offset]; offset += 1
        v_type = data[offset]; offset += 1  # tile type (v[b4])
        image_idx = data[offset]; offset += 1  # image index (T[b4])
        flags = data[offset]; offset += 1  # rotation/flip flags (b[b4])
        collision = data[offset]; offset += 1  # collision type (l[b4])

        # For collision type 1, there's additional data
        if collision == 1:
            # Read boolean array for pixel-perfect collision (16x16 for 12x12 upscaled)
            for y in range(16):  # Always 16x16 after upscaling from 12x12
                for x in range(16):
                    if offset < len(data):
                        offset += 1  # skip boolean

        # Read color/reference (af[b4])
        if offset + 4 <= len(data):
            color_ref = struct.unpack('>I', data[offset:offset+4])[0]
            offset += 4
        else:
            color_ref = 0

        # For collision type 3, s[b4] = s[af[b4]] (reference to another tile's collision)

        tile_data[tile_id] = {
            'type': v_type,
            'image_idx': image_idx,
            'flags': flags,
            'collision': collision,
            'color_ref': color_ref
        }

        # Print tiles 0-10 and 110-120 for analysis
        if (tile_id >= 0 and tile_id <= 10) or (tile_id >= 110 and tile_id <= 120):
            print(f'{tile_id:<4} {v_type:<6} {image_idx:<6} 0x{flags:02x}   {collision:<6}')

    return tile_data

if __name__ == '__main__':
    tf_path = 'original_code/bounce_back_s60.jar.src/res/tf'
    tile_data = parse_tf_file(tf_path)

    print('\n' + '='*40)
    print('Tile to Image mapping for tiles 110-120:')
    print('='*40)
    for tid in range(110, 121):
        if tid in tile_data:
            img_idx = tile_data[tid]['image_idx']
            tile_type = tile_data[tid]['type']
            print(f'Tile {tid}: image_idx={img_idx}, type={tile_type}')
