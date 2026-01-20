import struct
from collections import Counter
from pathlib import Path


def read_resource_chunks(path: Path):
    with path.open("rb") as f:
        num_chunks = struct.unpack(">H", f.read(2))[0]
        sizes = [struct.unpack(">H", f.read(2))[0] for _ in range(num_chunks)]
        return [f.read(sz) for sz in sizes]


def count_tiles(lf_path: Path):
    chunks = read_resource_chunks(lf_path)
    counts = Counter()
    num_levels = len(chunks) // 2

    for level in range(num_levels):
        tile_chunk = chunks[level * 2 + 1]
        if len(tile_chunk) < 2:
            continue
        height = tile_chunk[0]
        width = tile_chunk[1]
        data = tile_chunk[2:2 + height * width]
        if len(data) < height * width:
            data = data + bytes(height * width - len(data))
        for byte in data:
            counts[byte & 0x7F] += 1

    return counts, num_levels


def main():
    lf_path = Path("original_code/bounce_back_s60.jar.src/res/lf")
    counts, num_levels = count_tiles(lf_path)
    lines = [f"levels: {num_levels}"]
    lines.append("tile_id,count,percent")
    total = sum(counts.values())
    for tile_id, cnt in counts.most_common():
        percent = cnt * 100 / total if total else 0
        lines.append(f"{tile_id:3d},{cnt},{percent:.2f}")

    report_path = Path("tile_counts_report.txt")
    report_path.write_text("\n".join(lines), encoding="utf-8")
    print(f"Saved full report to {report_path}")
    print("Top 15 tiles:")
    for tile_id, cnt in counts.most_common(15):
        percent = cnt * 100 / total if total else 0
        print(f"  {tile_id:3d}: {cnt} ({percent:.2f}%)")


if __name__ == "__main__":
    main()

