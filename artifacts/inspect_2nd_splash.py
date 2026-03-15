from PIL import Image
img = Image.open("2nd splash.png").convert("RGBA")
print("size", img.size)
print("top", img.getpixel((0, 0)))
print("mid", img.getpixel((0, img.size[1] // 2)))
print("bot", img.getpixel((0, img.size[1] - 1)))
