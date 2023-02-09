#!/usr/bin/env python

import glob
from PIL import Image
from math import floor
import os.path
import io

transparency = 0


def palettize_file(filename):
    output_filename = filename.replace(os.path.splitext(filename)[1], ".img"). \
        replace("src/", "assets/")
    print(str(filename) + " became " + str(output_filename))
    img_src = Image.open(filename).convert('RGBA')
    img_data = img_src.load()
    img_dst = io.open(output_filename, "wb")
    img_dst.write(bytearray([(img_src.width & 0xFF00) >> 8,
                             img_src.width & 0xFF,
                             (img_src.height & 0xFF00) >> 8,
                             (img_src.height & 0xFF)]))

    last = img_data[0, 0]
    repetitions = 0

    for y in range(0, img_src.height):
        for x in range(0, img_src.width):
            pixel = img_data[x, y]
            adjusted = pixel
            if (adjusted[0] != last[0] or adjusted[1] != last[1] or adjusted[2] != last[2] or adjusted[3] != last[
                3] or repetitions == 255):
                img_dst.write(bytearray([last[0]]))
                img_dst.write(bytearray([last[1]]))
                img_dst.write(bytearray([last[2]]))
                img_dst.write(bytearray([last[3]]))
                img_dst.write(bytearray([repetitions]))
                last = adjusted
                repetitions = 1
            else:
                repetitions = repetitions + 1

    img_dst.write(bytearray([last[0]]))
    img_dst.write(bytearray([last[1]]))
    img_dst.write(bytearray([last[2]]))
    img_dst.write(bytearray([last[3]]))
    img_dst.write(bytearray([repetitions]))


if __name__ == "__main__":

    transparency = 0

    for filename in glob.glob('src/*.png'):
        palettize_file(filename)
