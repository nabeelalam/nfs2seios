"""Recompress a local Car Info or Track Info artwork pack without changing its RGBA pixels."""

import argparse
import hashlib
from pathlib import Path
import struct
import zlib

IMAGE_LIMITS = {b"NFSCARIF": 48, b"NFSTRKIF": 52}
MAX_FILE = 128 * 1024 * 1024
MAX_PAYLOAD = 384 * 1024 * 1024


def recompress(data):
    magic = data[:8]
    if not 16 < len(data) <= MAX_FILE or magic not in IMAGE_LIMITS:
        raise ValueError("Invalid information artwork pack")
    version, size = struct.unpack_from("<II", data, 8)
    if version != 1 or not 8 <= size <= MAX_PAYLOAD:
        raise ValueError("Expected a bounded version 1 information artwork pack")
    inflater = zlib.decompressobj()
    payload = bytearray(inflater.decompress(memoryview(data)[16:], size + 1))
    if len(payload) != size or not inflater.eof or inflater.unused_data:
        raise ValueError("Invalid compressed artwork payload")
    count, reserved = struct.unpack_from("<II", payload)
    if not 1 <= count <= IMAGE_LIMITS[magic] or reserved:
        raise ValueError("Invalid artwork image count")

    at = 8
    for _ in range(count):
        if len(payload) - at < 48:
            raise ValueError("Truncated artwork metadata")
        _, width, height, flags = struct.unpack_from("<IIII", payload, at)
        at += 48
        stride = width * 4
        end = at + stride * height
        if flags or not (1 <= width <= 8192 and 1 <= height <= 8192) or end > len(payload):
            raise ValueError("Invalid artwork image bounds")
        # Work bottom-up so each predictor still contains the original bytes.
        for row in range(end - stride, at, -stride):
            payload[row:row + stride] = bytes(
                (value - above) & 255
                for value, above in zip(payload[row:row + stride], payload[row - stride:row])
            )
        at = end

    if at != size:
        raise ValueError("Trailing artwork payload")
    encoded = magic + struct.pack("<II", 2, size) + zlib.compress(payload, 9)
    if len(encoded) >= len(data):
        raise ValueError("Row delta encoding did not shrink this pack")
    return encoded


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    try:
        with args.source.open("rb") as stream:
            encoded = recompress(stream.read(MAX_FILE + 1))
        with args.output.open("xb") as stream:
            stream.write(encoded)
    except (OSError, ValueError, zlib.error) as exc:
        parser.exit(1, f"Unable to recompress artwork: {exc}\n")
    print(f"{len(encoded)} bytes, SHA-256 {hashlib.sha256(encoded).hexdigest()}")


if __name__ == "__main__":
    main()
