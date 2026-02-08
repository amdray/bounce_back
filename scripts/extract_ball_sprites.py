#!/usr/bin/env python3
"""
Extract 25 ball sprite PNGs from res/b file.
Based on c.java loader logic.
"""

import struct
import os

def extract_ball_sprites(res_b_path, output_dir):
    """Extract all ball sprites from res/b."""
    with open(res_b_path, 'rb') as f:
        # Read header: number of chunks (2 bytes, big-endian)
        num_chunks = struct.unpack('>H', f.read(2))[0]
        print(f'Number of sprites: {num_chunks}')
        
        # Read all chunk sizes
        chunk_sizes = []
        for i in range(num_chunks):
            size = struct.unpack('>H', f.read(2))[0]
            chunk_sizes.append(size)
            print(f'Sprite {i:02d}: {size} bytes')
        
        # Read all chunks (PNG files)
        os.makedirs(output_dir, exist_ok=True)
        for i, size in enumerate(chunk_sizes):
            data = f.read(size)
            output_path = os.path.join(output_dir, f'ball_{i:02d}.png')
            with open(output_path, 'wb') as out:
                out.write(data)
            print(f'Extracted: {output_path}')
        
        return num_chunks

if __name__ == '__main__':
    base_path = r'D:\OneDrive\VS Code Project\psp\bounce_back'
    res_b_path = os.path.join(base_path, 'release', 'res', 'b')
    output_dir = os.path.join(base_path, 'artifacts', 'ball_sprites')
    
    if not os.path.exists(res_b_path):
        print(f'ERROR: {res_b_path} not found')
        exit(1)
    
    print('Extracting ball sprites from res/b...\n')
    count = extract_ball_sprites(res_b_path, output_dir)
    print(f'\nExtracted {count} ball sprites to {output_dir}')
