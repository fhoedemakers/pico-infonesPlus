#!/usr/bin/env python3
"""Convert binary PPM (P6) files to PNG using only the stdlib (zlib)."""
import struct, sys, zlib, glob, os

def read_ppm(path):
    with open(path, 'rb') as f:
        data = f.read()
    if not data.startswith(b'P6'):
        raise ValueError('not P6')
    # header: P6\n<w> <h>\n255\n
    parts = data.split(b'\n', 3)
    w, h = map(int, parts[1].split())
    return w, h, parts[3][:w * h * 3]

def write_png(path, w, h, rgb):
    def chunk(tag, payload):
        return (struct.pack('>I', len(payload)) + tag + payload +
                struct.pack('>I', zlib.crc32(tag + payload) & 0xFFFFFFFF))
    raw = b''.join(b'\x00' + rgb[y*w*3:(y+1)*w*3] for y in range(h))
    png = (b'\x89PNG\r\n\x1a\n' +
           chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)) +
           chunk(b'IDAT', zlib.compress(raw, 6)) +
           chunk(b'IEND', b''))
    with open(path, 'wb') as f:
        f.write(png)

for src in sys.argv[1:]:
    for p in glob.glob(src):
        w, h, rgb = read_ppm(p)
        out = os.path.splitext(p)[0] + '.png'
        write_png(out, w, h, rgb)
        print(out)
