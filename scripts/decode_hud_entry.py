"""Decode one indexed HUD entry for private visual inspection, without editing originals."""
import argparse
import binascii
import struct
import zlib
from inspect_hud_archive import inspect, LIMIT


def decode(data, index):
    report = inspect(data)
    if type(index) is not int or not 0 <= index < report["entry_count"]:
        raise ValueError("Invalid entry index")
    entry = report["entries"][index]
    width, height = entry["width_raw"], entry["height_raw"]
    if entry["format_code"] != 0x7b or entry["entry_compressed"] or not 0 < width <= 4096 or not 0 < height <= 4096:
        raise ValueError("Expected a bounded uncompressed indexed8 bitmap")
    pixels = width * height
    if 16 + pixels > entry["span_bytes"]:
        raise ValueError("Truncated indexed pixels")
    if entry["next_attachment_offset"] and entry["next_attachment_offset"] < 16 + pixels:
        raise ValueError("Attachment overlaps image pixels")
    palettes = [r for r in report["entries"] if r["format_code"] in (0x24, 0x2a) and not r["entry_compressed"]]
    palette = None
    offset = entry["offset"]
    end = offset + entry["span_bytes"]
    # Local palettes take precedence. Unknown attachments are metadata, not pixels.
    for _ in range(32):
        step = struct.unpack_from("<I", data, offset)[0] >> 8
        if not step:
            break
        if step < 16 or offset + step + 16 > end:
            raise ValueError("Invalid attachment span")
        offset += step
        code, colors, rows = struct.unpack_from("<Ihh", data, offset)
        if code & 127 in (0x22, 0x23, 0x24, 0x29, 0x2a, 0x2d):
            if code & 255 not in (0x24, 0x2a):
                raise ValueError("Unsupported local palette")
            palette = dict(offset=offset, format_code=code & 127, width_raw=colors,
                           height_raw=rows, span_bytes=end-offset)
    else:
        raise ValueError("Too many attachments")
    # An embedded palette is sufficient even when the archive has no global
    # palette (or several unrelated ones). Resolve the fallback only if needed.
    if palette is None:
        if len(palettes) != 1:
            raise ValueError("Ambiguous or missing global palette")
        palette = palettes[0]
    count = palette["width_raw"]
    stride = 4 if palette["format_code"] == 0x2a else 3
    if not 0 < count <= 256 or palette["height_raw"] != 1 or 16 + count * stride > palette["span_bytes"]:
        raise ValueError("Invalid palette span")
    start = palette["offset"] + 16
    table = []
    for i in range(count):
        raw = data[start+i*stride:start+(i+1)*stride]
        if stride == 4:
            table.append(bytes((raw[2], raw[1], raw[0], raw[3])))
        else:
            # The original RGB-palette conversion reserves entry 255 as transparent.
            table.append(raw+bytes((0 if i == 255 else 255,)))
    indices = data[entry["offset"]+16:entry["offset"]+16+pixels]
    if any(value >= count for value in indices):
        raise ValueError("Palette index outside table")
    return width, height, b"".join(table[value] for value in indices)


def png(width, height, rgba):
    if len(rgba) != width*height*4 or not 0 < width <= 4096 or not 0 < height <= 4096:
        raise ValueError("Invalid RGBA dimensions")
    def chunk(kind, body):
        return struct.pack(">I", len(body))+kind+body+struct.pack(">I", binascii.crc32(kind+body)&0xffffffff)
    rows = b"".join(b"\0"+rgba[i*width*4:(i+1)*width*4] for i in range(height))
    return b"\x89PNG\r\n\x1a\n"+chunk(b"IHDR", struct.pack(">2I5B", width, height, 8, 6, 0, 0, 0))+chunk(b"IDAT", zlib.compress(rows))+chunk(b"IEND", b"")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive")
    parser.add_argument("entry_index", type=int)
    parser.add_argument("output_png")
    args = parser.parse_args()
    try:
        with open(args.archive, "rb") as source:
            data = source.read(LIMIT+1)
        image = decode(data, args.entry_index)
        with open(args.output_png, "xb") as output:
            output.write(png(*image))
    except (OSError, ValueError, struct.error):
        parser.exit(1, "Unable to decode: unsupported entry, malformed data or output already exists.\n")


if __name__ == "__main__":
    main()
