#!/usr/bin/env python3
"""
Extract all PNG files from res/if0 and res/if1 with index numbers.
"""

import struct
import os

def read_resource_file(filepath):
    """Read a resource file in the custom format."""
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

def extract_pngs(if_path, output_dir, start_index=0):
    """Extract all PNGs from a resource file."""
    chunks = read_resource_file(if_path)
    
    os.makedirs(output_dir, exist_ok=True)
    
    for i, chunk in enumerate(chunks):
        output_path = os.path.join(output_dir, f'{start_index + i:03d}.png')
        with open(output_path, 'wb') as f:
            f.write(chunk)
        print(f'Extracted: {output_path} ({len(chunk)} bytes)')
    
    return len(chunks)

if __name__ == '__main__':
    base_path = r'D:\OneDrive\VS Code Project\psp\bounce_back'
    if0_path = os.path.join(base_path, 'release', 'res', 'if0')
    if1_path = os.path.join(base_path, 'release', 'res', 'if1')
    output_dir = os.path.join(base_path, 'artifacts', 'extracted_textures')
    
    print('Extracting from if0...')
    count0 = extract_pngs(if0_path, output_dir, start_index=0)
    
    print(f'\nExtracted {count0} PNGs from if0')
    
    if os.path.exists(if1_path):
        print('\nExtracting from if1...')
        count1 = extract_pngs(if1_path, output_dir, start_index=count0)
        print(f'\nExtracted {count1} PNGs from if1')
        print(f'\nTotal: {count0 + count1} PNGs')
    else:
        print('\nif1 not found, skipping')
        print(f'\nTotal: {count0} PNGs')
    
    print(f'\nAll PNGs saved to: {output_dir}')
