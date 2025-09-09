
from PIL import Image

# Configuration
input_file = "... .png"         # Replace with your image path
output_file = "output.h"          # Output Arduino array file
width, height = 320, 240        # MCU screen size

# Open image and resize
img = Image.open(input_file).convert("RGB")
img = img.resize((width, height))

# Convert to RGB565
pixels = []
for y in range(height):
    for x in range(width):
        r, g, b = img.getpixel((x, y))
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        pixels.append(rgb565)

# Write to Arduino array file
with open(output_file, "w") as f:
    f.write("const uint16_t logo[] PROGMEM = {\n")
    for i, val in enumerate(pixels):
        if i == len(pixels) - 1:
            f.write(f"0x{val:04X}")  # No comma for the last element
        else:
            f.write(f"0x{val:04X}, ")
        if (i + 1) % 12 == 0:
            f.write("\n")
    f.write("\n};\n")

print(f"Task completed, generated {output_file}. Size: {width}x{height}, total pixels: {len(pixels)}.")
