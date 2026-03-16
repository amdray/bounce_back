import struct

CONVEYOR_TILES = {79, 82, 83, 84, 108, 109}

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

print(f"Total levels: {num_levels}")
for lvl in range(num_levels):
    tiles_chunk = chunks[lvl*2 + 1]
    h = tiles_chunk[0]; w = tiles_chunk[1]
    found = []
    for ty in range(h):
        for tx in range(w):
            tid = tiles_chunk[2 + ty*w + tx] & 0x7F
            if tid in CONVEYOR_TILES:
                found.append((tid, tx, ty))
    if found:
        print(f"\nLevel {lvl} ({w}x{h}):")
        for tid, tx, ty in found:
            print(f"  tile {tid} at col={tx}, row={ty}")
