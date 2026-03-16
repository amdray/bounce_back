#!/usr/bin/env python3
from PIL import Image
for idx in [26,27,28,40,41,42,60,66,69,70,71,78,81]:
    p = f'/mnt/d/OneDrive/VSCodeProject/psp/bounce_back/artifacts/extracted_textures/{idx:03d}.png'
    img = Image.open(p)
    img = img.convert('RGBA')
    px = list(img.getdata())
    nonblack = sum(1 for c in px if c[3] > 0)
    print(f'img {idx:3d}: {img.size}  non-black:{nonblack}')
