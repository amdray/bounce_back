#!/usr/bin/env python3
import sys
sys.path.insert(0, 'scripts')
from dump_tf_tiles import parse_tf
tf = parse_tf('original_code/bounce_back_s60.jar.src/res/tf')
vals = sorted(set(t['transform'] for t in tf['tiles']))
print('Distinct b-values:', vals)
for b in vals:
    tiles = [t['tile_id'] for t in tf['tiles'] if t['transform'] == b]
    print(f'  b={b}: tiles={tiles}')
