import struct
with open("release/res/ic","rb") as f:
    data = f.read()
results = []
i = 0
while i < len(data)-8:
    if data[i:i+8] == b"\x89PNG\r\n\x1a\n":
        s = i + 8 + 4
        if data[s:s+4] == b"IHDR":
            w,h = struct.unpack(">II", data[s+4:s+12])
            results.append((len(results), w, h))
        i += 1
    else:
        i += 1
for j,w,h in results:
    print(f"ic[{j}]: {w}x{h}")
