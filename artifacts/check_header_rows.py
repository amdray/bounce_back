from PIL import Image
img = Image.open("artifacts/im_dump/im_02.png").convert("RGBA")
w,h = img.size
print("im[2]: {}x{}".format(w,h))
for y in range(h):
    col = img.getpixel((88, y))
    print("  x=88 y={}: {}".format(y, col))
