#!/usr/bin/env python3
"""
Convert 1-bit BMP to C bitmap array for Adafruit SSD1306
"""
import struct

def read_bmp_header(f):
    # Read BMP header
    bmp_header = f.read(14)
    if bmp_header[:2] != b'BM':
        raise ValueError("Not a valid BMP file")

    pixel_offset = struct.unpack('<I', bmp_header[10:14])[0]

    # Read DIB header
    dib_header_size = struct.unpack('<I', f.read(4))[0]
    f.seek(14)  # Back to start of DIB header
    dib_header = f.read(dib_header_size)

    width = struct.unpack('<i', dib_header[4:8])[0]
    height = struct.unpack('<i', dib_header[8:12])[0]
    bits_per_pixel = struct.unpack('<H', dib_header[14:16])[0]

    return width, abs(height), bits_per_pixel, pixel_offset

def bmp_to_oled_bitmap(bmp_path):
    with open(bmp_path, 'rb') as f:
        width, height, bpp, pixel_offset = read_bmp_header(f)

        print(f"Image: {width}x{height}, {bpp} bits per pixel")

        # Read pixel data
        f.seek(pixel_offset)

        # For monochrome BMP, calculate row size (padded to 4 bytes)
        row_size = ((width * bpp + 31) // 32) * 4

        # Read all pixel data
        pixel_data = []
        for y in range(height):
            row = f.read(row_size)
            pixel_data.append(row)

        # BMP is stored bottom-to-top, so reverse
        pixel_data.reverse()

        # Convert to OLED format (vertical bytes)
        bitmap = []
        for page in range(0, height, 8):  # 8 rows per page
            for x in range(width):
                byte = 0
                for bit in range(8):
                    y = page + bit
                    if y < height:
                        # Get pixel from BMP data
                        byte_index = x // 8
                        bit_index = 7 - (x % 8)

                        if byte_index < len(pixel_data[y]):
                            pixel_byte = pixel_data[y][byte_index]
                            # BMP uses 1 for white, 0 for black
                            # OLED uses 1 for white, 0 for black
                            if pixel_byte & (1 << bit_index):
                                byte |= (1 << bit)
                bitmap.append(byte)

        # Output C array
        print("\nconst unsigned char meme_bitmap[] PROGMEM = {")
        for i in range(0, len(bitmap), 16):
            line = ', '.join(f'0x{b:02x}' for b in bitmap[i:i+16])
            if i + 16 < len(bitmap):
                print(f"  {line},")
            else:
                print(f"  {line}")
        print("};")
        print(f"\n#define MEME_WIDTH {width}")
        print(f"#define MEME_HEIGHT {height}")

if __name__ == "__main__":
    bmp_to_oled_bitmap("lib/meme.bmp")
