import struct, os
with open("release/res/im","rb") as f:
    data = f.read()
results = []
i = 0
while i < len(data)-8:
    if data[i:i+8] == b"\x89PNG\r\n\x1a\n":
        s = i + 8 + 4
        if data[s:s+4] == b"IHDR":
            w,h = struct.unpack(">II", data[s+4:s+12])
            results.append((len(results), i, w, h))
        i += 1
    else:
        i += 1
for j,off,w,h in results:
    print(f"im[{j}] offset={off} size={w}x{h}")
print(f"total: {len(results)} images")

# also extract them
os.makedirs("artifacts/im_dump", exist_ok=True)
for idx in range(len(results)):
    start = results[idx][1]
    end = results[idx+1][1] if idx+1 < len(results) else len(data)
    with open(f"artifacts/im_dump/im_{idx:02d}.png","wb") as f:
        f.write(data[start:end])
print("Extracted to artifacts/im_dump/")
