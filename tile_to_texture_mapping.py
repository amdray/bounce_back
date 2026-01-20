#!/usr/bin/env python3
"""
Generate complete tile ID to image index mapping from tf file.
"""

from parse_tf import parse_tf_file

if __name__ == '__main__':
    tf_path = r'd:\OneDrive\bounce_back\original_code\bounce_back_s60.jar.src\res\tf'
    tile_data = parse_tf_file(tf_path)

    print('\n' + '='*60)
    print('Complete Tile to Image Mapping')
    print('='*60)
    print(f'{"Tile ID":<10} {"Image Index":<15} {"Source File":<15} {"Type":<8} {"Flags":<8}')
    print('-'*60)

    # Images 0-103 from if0, 104-110 from if1
    for tile_id in sorted(tile_data.keys()):
        t = tile_data[tile_id]
        img_idx = t['image_idx']
        tile_type = t['type']
        flags = t['flags']

        # Determine source file
        if img_idx <= 103:
            source = f'if0[{img_idx}]'
        else:
            source = f'if1[{img_idx - 104}]'

        print(f'{tile_id:<10} {img_idx:<15} {source:<15} {tile_type:<8} 0x{flags:02x}')

    print('\n' + '='*60)
    print('Summary:')
    print(f'Total tiles: {len(tile_data)}')
    print('='*60)
