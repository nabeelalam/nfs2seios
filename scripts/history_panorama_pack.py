"""Write a separate, lossless History panorama from opaque raw RGBA8 bytes."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib


def build(rgba, width, height, source):
    if source not in range(0, 49, 7):
        raise ValueError("Expected a History background source index")
    if not 0 < width <= 4096 or not 0 < height <= 4096 or abs(width * 9 - height * 16) > 18:
        raise ValueError("Expected a bounded 16:9 panorama")
    expected = width * height * 4
    if expected > 32 * 1024**2 or len(rgba) != expected or any(a != 255 for a in rgba[3::4]):
        raise ValueError("Expected complete opaque RGBA8 pixels")
    body = zlib.compress(rgba, 9)
    check = zlib.decompressobj()
    decoded = check.decompress(body, expected + 1)
    if decoded != rgba or not check.eof or check.unused_data or check.unconsumed_tail:
        raise ValueError("Lossless round trip failed")
    return struct.pack("<8s6I", b"NFSHWID2", 2, source, width, height, expected, 0) + body


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--rgba", required=True, type=Path)
    ap.add_argument("--width", required=True, type=int)
    ap.add_argument("--height", required=True, type=int)
    ap.add_argument("--source-index", required=True, type=int)
    ap.add_argument("--output", required=True, type=Path)
    args = ap.parse_args()
    if args.rgba.stat().st_size > 32 * 1024**2:
        raise ValueError("RGBA input exceeds the resource limit")
    with args.rgba.open("rb") as stream:
        rgba = stream.read(32 * 1024**2 + 1)
    packed = build(rgba, args.width, args.height, args.source_index)
    with args.output.open("xb") as stream:
        stream.write(packed)
    print(json.dumps({"version": 2, "sourceIndex": args.source_index,
        "width": args.width, "height": args.height, "rgbaBytes": len(rgba),
        "fileBytes": len(packed), "rgbaSHA256": hashlib.sha256(rgba).hexdigest(),
        "fileSHA256": hashlib.sha256(packed).hexdigest(), "losslessVerified": True}))


if __name__ == "__main__":
    main()
