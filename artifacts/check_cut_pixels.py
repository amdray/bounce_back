from PIL import Image

img2 = Image.open("artifacts/im_dump/im_02.png").convert("RGBA")
w2, h2 = img2.size
print("im[2]: {}x{}".format(w2, h2))

# Pixels near the cut point (x=83..92) at row y=10
print("\nПиксели по x вокруг разреза (y=10):")
for x in range(80, 96):
    col = img2.getpixel((x, 10))
    print("  x={} y=10: {}".format(x, col))

# Full last row (y=20) - is it all white?
print("\nПоследняя строка y=20 (sample):")
for x in [0, 10, 44, 87, 88, 131, 165, 175]:
    col = img2.getpixel((x, 20))
    print("  x={} y=20: {}".format(x, col))

# What's at inner edge of im[3] at top (y=0)?
img3 = Image.open("artifacts/im_dump/im_03.png").convert("RGBA")
w3, h3 = img3.size
print("\nim[3] top row (y=0) full:")
for x in range(w3):
    col = img3.getpixel((x, 0))
    print("  x={}: {}".format(x, col))
