from PIL import Image

img2 = Image.open("artifacts/im_dump/im_02.png").convert("RGBA")
w2, h2 = img2.size
print("im[2] header: {}x{}".format(w2, h2))
for x in [0, 43, 87, 88, 132, 175]:
    col = img2.getpixel((x, h2 // 2))
    print("  x={} mid: rgba={}".format(x, col))

img3 = Image.open("artifacts/im_dump/im_03.png").convert("RGBA")
w3, h3 = img3.size
print("im[3] column: {}x{}".format(w3, h3))
for y in [0, h3 // 2, h3 - 1]:
    col_l = img3.getpixel((0, y))
    col_r = img3.getpixel((w3-1, y))
    print("  y={} left={} right={}".format(y, col_l, col_r))
