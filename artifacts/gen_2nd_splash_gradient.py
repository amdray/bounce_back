from PIL import Image

img = Image.open("2nd splash.png").convert("RGBA")
if img.size != (1, 208):
    raise SystemExit(f"unexpected size: {img.size}")

rows = []
for y in range(208):
    r, g, b, a = img.getpixel((0, y))
    rows.append((r, g, b))

with open("artifacts/second_splash_gradient.inc", "w", encoding="ascii") as f:
    f.write("static const uint8_t k_second_splash_grad_208[208][3] = {\n")
    for i, (r, g, b) in enumerate(rows):
        comma = "," if i != 207 else ""
        f.write(f"    {{{r:3d}, {g:3d}, {b:3d}}}{comma}\n")
    f.write("};\n")

print("generated artifacts/second_splash_gradient.inc")
