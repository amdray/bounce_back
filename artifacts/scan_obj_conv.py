import struct

def read_chunks(data):
    idx = 0
    n = struct.unpack_from('>H', data, idx)[0]; idx += 2
    sizes = [struct.unpack_from('>H', data, idx + i*2)[0] for i in range(n)]
    idx += n * 2
    chunks = []
    for s in sizes:
        chunks.append(data[idx:idx+s]); idx += s
    return chunks

data = open('/mnt/d/OneDrive/VSCodeProject/psp/bounce_back/original_code/bounce_back_s60.jar.src/res/lf', 'rb').read()
chunks = read_chunks(data)
num_levels = len(chunks) // 2

for lvl in range(num_levels):
    meta_chunk = chunks[lvl*2]
    tiles_chunk = chunks[lvl*2 + 1]
    
    h = tiles_chunk[0]; w = tiles_chunk[1]
    
    # Look for conveyor tiles in this level
    CONV = {79, 82, 83, 84, 108, 109}
    has_conv = any((tiles_chunk[2 + ty*w + tx] & 0x7F) in CONV 
                   for ty in range(h) for tx in range(w))
    if not has_conv:
        continue
    
    print(f"\n=== Level {lvl} ({w}x{h}) ===")
    
    # Parse metadata
    if len(meta_chunk) >= 7:
        num_objects = meta_chunk[6]
        print(f"  Objects: {num_objects}")
        off = 7
        for i in range(num_objects):
            if off + 9 > len(meta_chunk):
                break
            ao = meta_chunk[off]
            b8 = meta_chunk[off+1]  # top row (tile)
            b9 = meta_chunk[off+2]  # left col (tile)
            b10 = meta_chunk[off+3] # bottom row
            b11 = meta_chunk[off+4] # right col
            s1 = struct.unpack_from('b', meta_chunk, off+7)[0]  # vertical vel
            s0 = struct.unpack_from('b', meta_chunk, off+8)[0]  # horizontal vel
            
            if b8 > b10 or b9 > b11:
                b8,b10 = b10,b8
                b9,b11 = b11,b9
            
            type_name = {0:"bounce_obj",1:"enemy",2:"lift"}
            print(f"  obj[{i}] type={ao}({type_name.get(ao,'?')}) rows={b8}-{b10} cols={b9}-{b11} s=({s0},{s1})")
            off += 9
    
    # Print conveyor rows
    conv_rows = set()
    for ty in range(h):
        for tx in range(w):
            tid = tiles_chunk[2 + ty*w + tx] & 0x7F
            if tid in CONV:
                conv_rows.add(ty)
    print(f"  Conveyor rows: {sorted(conv_rows)}")
