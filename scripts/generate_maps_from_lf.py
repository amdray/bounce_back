#!/usr/bin/env python3
"""
Generate level maps directly from lf and if files without intermediate PNG files.
"""

import struct
import os
from PIL import Image, ImageDraw
import io

TILE_SIZE = 16


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


def load_tile_images_from_if(if_path):
    """Load all tile images directly from if file."""
    chunks = read_resource_file(if_path)
    images = []
    for chunk in chunks:
        try:
            img = Image.open(io.BytesIO(chunk))
            # Convert palette mode to RGBA to preserve transparency
            if img.mode == 'P':
                img = img.convert('RGBA')
            images.append(img)
        except:
            images.append(None)
    return images


def parse_tf_metadata(tf_path):
    """Parse tf file to get tile metadata including rotation flags and bg color."""
    chunks = read_resource_file(tf_path)
    if len(chunks) < 2:
        return {}, None

    tile_metadata = {}
    data = chunks[0]
    offset = 0

    # Read header (from g.java lines 147-166)
    b1 = data[offset]; offset += 1  # total images
    b2 = data[offset]; offset += 1  # images from if0
    offset += 1  # wrap_h
    offset += 1  # wrap_v
    offset += 1  # tile_w
    offset += 1  # tile_h
    num_tiles = data[offset]; offset += 1
    split_point = data[offset]; offset += 1
    offset += 1  # tile_mask
    offset += 1  # bg_mask
    bg_color = struct.unpack('>I', data[offset:offset+4])[0]; offset += 4  # bg_color (int)

    # Skip animation data
    U = data[offset]; offset += 1  # number of animations
    if U > 0:
        for i in range(U):
            offset += 1  # unused byte
            offset += 1  # O (frame duration)
            num_frames = data[offset]; offset += 1
            offset += num_frames  # skip frame data

    # Parse tile data
    for tile_id in range(num_tiles):
        if tile_id == split_point:
            # Switch to second chunk
            data = chunks[1]
            offset = 0

        offset += 1  # unused byte
        v_type = data[offset]; offset += 1
        image_idx = data[offset]; offset += 1
        flags = data[offset]; offset += 1
        collision = data[offset]; offset += 1

        # For collision type 1, skip pixel-perfect collision data
        if collision == 1:
            for y in range(16):
                for x in range(16):
                    if offset < len(data):
                        offset += 1

        # Skip color/reference (4 bytes)
        if offset + 4 <= len(data):
            offset += 4

        tile_metadata[tile_id] = {
            'type': v_type,
            'image': image_idx,
            'rotation': flags,
            'collision': collision
        }

    return tile_metadata, bg_color


def parse_tile_map(data):
    """Parse tile map data."""
    if len(data) < 2:
        return None, None, None
    height = data[0]
    width = data[1]
    tiles = []
    idx = 2
    for row in range(height):
        row_tiles = []
        for col in range(width):
            if idx < len(data):
                tile = data[idx]
                row_tiles.append(tile)
                idx += 1
            else:
                row_tiles.append(0)
        tiles.append(row_tiles)
    return width, height, tiles


def parse_level_metadata(data):
    """Parse level metadata to get spawn point (h.java:195-241)."""
    if len(data) < 7:
        return None

    metadata = {
        'b1': data[0],
        'spawn_y_tiles': data[1],  # b2
        'spawn_x_tiles': data[2],  # b3
        'player_type': data[3],    # b4
        'ar': data[4],
        'D': data[5],
        'num_objects': data[6],    # j
    }

    # Calculate pixel coordinates (h.java:292-302)
    offset = 8 if metadata['player_type'] == 0 else 12
    metadata['spawn_x'] = metadata['spawn_x_tiles'] * 16 + offset
    metadata['spawn_y'] = metadata['spawn_y_tiles'] * 16 + offset
    metadata['ball_size'] = 16 if metadata['player_type'] == 0 else 20

    return metadata


def apply_tile_rotation(tile_img, base_rotation, has_0x80_flag):
    """Apply rotation to tile - no transformations needed.

    In g.java:259, the game is created with paramBoolean2=false (this.ab=false).
    This means the rotation code at g.java:238-250 is NEVER executed.
    Therefore, textures in if0/if1 are already pre-rotated.

    The 0x80 flag (from g.java:601-604) means "draw background color", not "flip".
    When 0x80 is set, the game fills a 16x16 rect with this.i (bg color) before drawing tile.
    This is NOT a transformation - just a background fill.

    So no transformations should be applied to the tile image itself.
    """
    # No transformations needed - tiles are already in correct orientation
    return tile_img


def generate_level_map(width, height, tiles, level_num, output_dir, tile_images, tile_overlay_images, tile_metadata, bg_color, spawn_metadata):
    """Generate level map with tiles and spawn point drawn."""
    img_width = width * TILE_SIZE
    img_height = height * TILE_SIZE

    # Sky blue background (RGB, no alpha) - реальный цвет из /res/ib0
    sky_blue = (0, 115, 239)
    img = Image.new('RGB', (img_width, img_height), color=sky_blue)

    # Convert bg_color from ARGB int to RGB tuple
    # bg_color format: 0xAARRGGBB (e.g., 0x7F000CFF)
    bg_r = (bg_color >> 16) & 0xFF
    bg_g = (bg_color >> 8) & 0xFF
    bg_b = bg_color & 0xFF
    water_color = (bg_r, bg_g, bg_b)

    # Draw tiles
    tile_counts = {2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 0, 11: 0, 12: 0, 15: 0, 18: 0, 22: 0, 26: 0, 30: 0, 34: 0, 57: 0, 60: 0, 62: 0, 63: 0, 64: 0, 65: 0, 66: 0, 67: 0, 69: 0, 71: 0, 79: 0, 82: 0, 83: 0, 85: 0, 86: 0, 87: 0, 88: 0, 89: 0, 90: 0, 91: 0, 92: 0, 109: 0, 110: 0, 111: 0, 112: 0, 113: 0, 114: 0, 115: 0, 116: 0}
    if tiles:
        for row in range(height):
            for col in range(width):
                tile_byte = tiles[row][col]
                tile_id = tile_byte & 0x7F  # Base ID without flag
                has_flag = (tile_byte & 0x80) != 0  # Check if 0x80 flag is set
                x = col * TILE_SIZE
                y = row * TILE_SIZE

                # Handle tile 0 (air/empty)
                if tile_id == 0:
                    # If 0x80 flag is set, fill with water/dark background color
                    # (from g.java:601-604)
                    if has_flag:
                        draw = ImageDraw.Draw(img)
                        draw.rectangle([x, y, x + TILE_SIZE - 1, y + TILE_SIZE - 1], fill=water_color)
                    continue

                # If 0x80 flag is set, draw background color first (from g.java:601-604)
                # This applies to ALL tiles, not just tile 0
                if has_flag:
                    draw = ImageDraw.Draw(img)
                    draw.rectangle([x, y, x + TILE_SIZE - 1, y + TILE_SIZE - 1], fill=water_color)

                # Draw background layer
                if tile_id in tile_images and tile_images[tile_id]:
                    tile_img = tile_images[tile_id].copy()

                    # Get base rotation from tf metadata
                    base_rotation = 0x00
                    if tile_id in tile_metadata:
                        base_rotation = tile_metadata[tile_id]['rotation']

                    # Apply base rotation and 0x80 flag
                    tile_img = apply_tile_rotation(tile_img, base_rotation, has_flag)

                    img.paste(tile_img, (x, y), tile_img)
                    tile_counts[tile_id] += 1

                # Draw foreground overlay layer (for special objects like rings)
                if tile_id in tile_overlay_images and tile_overlay_images[tile_id]:
                    overlay_img = tile_overlay_images[tile_id].copy()

                    # Get base rotation from tf metadata
                    base_rotation = 0x00
                    if tile_id in tile_metadata:
                        base_rotation = tile_metadata[tile_id]['rotation']

                    # Apply base rotation and 0x80 flag
                    overlay_img = apply_tile_rotation(overlay_img, base_rotation, has_flag)

                    img.paste(overlay_img, (x, y), overlay_img)

    # Draw spawn point if metadata available
    if spawn_metadata:
        draw = ImageDraw.Draw(img)
        spawn_x = spawn_metadata['spawn_x']
        spawn_y = spawn_metadata['spawn_y']
        ball_size = spawn_metadata['ball_size']
        radius = ball_size // 2

        # Draw player spawn circle (green with red center)
        draw.ellipse([spawn_x - radius, spawn_y - radius, spawn_x + radius, spawn_y + radius],
                     fill=(0, 255, 0), outline=(255, 0, 0), width=2)
        # Draw center cross
        draw.line([spawn_x - 4, spawn_y, spawn_x + 4, spawn_y], fill=(255, 0, 0), width=1)
        draw.line([spawn_x, spawn_y - 4, spawn_x, spawn_y + 4], fill=(255, 0, 0), width=1)

    output_path = os.path.join(output_dir, f'level_{level_num:02d}.png')
    img.save(output_path)

    spawn_info = f" Spawn: ({spawn_metadata['spawn_x']}, {spawn_metadata['spawn_y']})" if spawn_metadata else ""
    print(f'  Level {level_num}: {tile_counts[110]} x 110, {tile_counts[111]} x 111, {tile_counts[112]} x 112{spawn_info}')
    return tile_counts


def main():
    res_path = 'original_code/bounce_back_s60.jar.src/res'
    output_dir = 'level_maps'

    os.makedirs(output_dir, exist_ok=True)

    print('Loading tile metadata from tf...')
    tile_metadata, bg_color = parse_tf_metadata(os.path.join(res_path, 'tf'))
    print(f'  Loaded metadata for {len(tile_metadata)} tiles')
    print(f'  Background color: 0x{bg_color:08X} (RGB: {(bg_color >> 16) & 0xFF}, {(bg_color >> 8) & 0xFF}, {bg_color & 0xFF})')

    print('Loading player sprites from b...')
    player_sprites = load_tile_images_from_if(os.path.join(res_path, 'b'))
    print(f'  Loaded {len(player_sprites)} player sprites')

    print('Loading tile images from if0...')
    if0_images = load_tile_images_from_if(os.path.join(res_path, 'if0'))
    print(f'  Loaded {len(if0_images)} images from if0')

    print('Loading tile images from if1...')
    if1_images = load_tile_images_from_if(os.path.join(res_path, 'if1'))
    print(f'  Loaded {len(if1_images)} images from if1')

    print('Loading special object images from ic...')
    ic_images = load_tile_images_from_if(os.path.join(res_path, 'ic'))
    print(f'  Loaded {len(ic_images)} images from ic')

    # Map tile IDs to images (from tf file analysis)
    # Total images: 111 (0-103 from if0, 104-110 from if1)
    # Tile 110 (img 104) = if1[0]
    # Tile 111 (img 105) = if1[1]
    # Tile 112 (img 106) = if1[2]
    # Tile 113 (img 107) = if1[3]
    # Tile 114 (img 108) = if1[4]
    # Tile 115 (img 109) = if1[5]
    # Tile 116 (img 110) = if1[6]
    tile_images = {
        2: if0_images[0] if len(if0_images) > 0 else None,
        3: if0_images[1] if len(if0_images) > 1 else None,  # Slope tile (cyan triangle)
        4: if0_images[53] if len(if0_images) > 53 else None,  # Slope tile (cyan triangle)
        5: if0_images[54] if len(if0_images) > 54 else None,  # Slope tile (cyan triangle)
        6: if0_images[55] if len(if0_images) > 55 else None,  # Slope tile (cyan triangle)
        7: if0_images[2] if len(if0_images) > 2 else None,  # Collectible item (orange container)
        8: if0_images[56] if len(if0_images) > 56 else None,
        11: if0_images[3] if len(if0_images) > 3 else None,  # Animated pump (type=3), img 3
        12: if0_images[5] if len(if0_images) > 5 else None,  # Health kit
        15: if0_images[6] if len(if0_images) > 6 else None,  # Animated powerup (type=3), img 6
        18: if0_images[8] if len(if0_images) > 8 else None,  # Animated pump (type=3), img 8
        22: if0_images[10] if len(if0_images) > 10 else None,  # Animated powerup (type=3), img 10
        26: if0_images[13] if len(if0_images) > 13 else None,  # Animated powerup (type=3), img 13
        30: if0_images[16] if len(if0_images) > 16 else None,  # Animated tile (type=3), img 16
        34: if0_images[19] if len(if0_images) > 19 else None,
        57: if0_images[71] if len(if0_images) > 71 else None,  # Paired with tile 60
        60: if0_images[73] if len(if0_images) > 73 else None,  # Paired with tile 57
        62: if0_images[36] if len(if0_images) > 36 else None,
        63: if0_images[75] if len(if0_images) > 75 else None,  # Paired with tile 90
        64: if0_images[76] if len(if0_images) > 76 else None,  # Paired with tile 65
        65: if0_images[77] if len(if0_images) > 77 else None,  # Paired with tile 64
        66: if0_images[37] if len(if0_images) > 37 else None,  # Paired with tile 69
        67: if0_images[78] if len(if0_images) > 78 else None,  # Compound object (paired with 71)
        69: if0_images[38] if len(if0_images) > 38 else None,  # Paired with tile 66
        71: if0_images[81] if len(if0_images) > 81 else None,  # Compound object (paired with 67)
        79: if0_images[40] if len(if0_images) > 40 else None,  # Animated tile (type=3), img 40
        82: if0_images[42] if len(if0_images) > 42 else None,  # Animated booster (type=3), img 42
        83: if0_images[86] if len(if0_images) > 86 else None,  # Animated booster (type=3), img 86
        85: if0_images[44] if len(if0_images) > 44 else None,
        86: if0_images[90] if len(if0_images) > 90 else None,
        87: if0_images[91] if len(if0_images) > 91 else None,  # Spikes
        88: if0_images[92] if len(if0_images) > 92 else None,  # Spikes
        89: if0_images[45] if len(if0_images) > 45 else None,
        90: if0_images[93] if len(if0_images) > 93 else None,  # Paired with tile 63
        91: if0_images[94] if len(if0_images) > 94 else None,  # Compound spikes (paired with 92)
        92: if0_images[95] if len(if0_images) > 95 else None,  # Compound spikes (paired with 91)
        # Ring tiles disabled - need all parts together, see ring_tiles_notes.txt
        # 93, 95, 97, 98, 99 - ring parts
        109: if0_images[88] if len(if0_images) > 88 else None,  # Animated tile (type=3), img 88
        110: if1_images[0] if len(if1_images) > 0 else None,
        111: if1_images[1] if len(if1_images) > 1 else None,
        112: if1_images[2] if len(if1_images) > 2 else None,
        113: if1_images[3] if len(if1_images) > 3 else None,
        114: if1_images[4] if len(if1_images) > 4 else None,
        115: if1_images[5] if len(if1_images) > 5 else None,
        116: if1_images[6] if len(if1_images) > 6 else None,
    }

    # Two-layer tiles (background + foreground overlay)
    # Currently none - tile 93/95 are just regular tiles from if0
    tile_overlay_images = {}

    print('\nReading levels from lf...')
    lf_chunks = read_resource_file(os.path.join(res_path, 'lf'))
    num_levels = len(lf_chunks) // 2
    print(f'  Found {num_levels} levels\n')

    total_counts = {110: 0, 111: 0, 112: 0}  # 113, 114, 115, 116 temporarily disabled
    for level_idx in range(num_levels):
        chunk_idx = level_idx * 2
        if chunk_idx + 1 < len(lf_chunks):
            # Parse level metadata for spawn point
            metadata_chunk = lf_chunks[chunk_idx]
            spawn_metadata = parse_level_metadata(metadata_chunk)

            # Parse tilemap
            tilemap_chunk = lf_chunks[chunk_idx + 1]
            width, height, tiles = parse_tile_map(tilemap_chunk)
            if width and height:
                counts = generate_level_map(width, height, tiles, level_idx, output_dir, tile_images, tile_overlay_images, tile_metadata, bg_color, spawn_metadata)
                for tid in total_counts:
                    total_counts[tid] += counts[tid]

    print(f'\nDone! Total tiles drawn:')
    print(f'  Tile 110: {total_counts[110]}')
    print(f'  Tile 111: {total_counts[111]}')
    print(f'  Tile 112: {total_counts[112]}')


if __name__ == '__main__':
    main()
