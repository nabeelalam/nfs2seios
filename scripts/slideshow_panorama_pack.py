"""Package explicitly reviewed private panorama side strips without editing pixels.

Manifest images supply width/height/centerWidth, centerRaw/sideRaw relative files,
archiveSha256/entrySha256/centerRgbaSha256/footerRgbaSha256, and reviewed:true.
sideRaw contains LEFT strip row-major RGBA followed by RIGHT strip row-major
RGBA. Only sides enter the output; the accepted center remains a separate asset.
All inputs/outputs are private. The resource grants no source or GPU authority.
"""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import zlib

MAGIC = b"NFSSPAN\0"
HEADER = struct.Struct("<8sIIIIQ")
TAIL = struct.Struct("<IIIIQ")
RECORD_BYTES = 216
MAX_IMAGE = 64 * 1024**2
MAX_CHUNK = 32 * 1024**2
MAX_PAYLOAD = 256 * 1024**2


def digest(value):
    if not isinstance(value, str) or len(value) != 64:
        raise ValueError("Expected a nonzero SHA256")
    try:
        raw = bytes.fromhex(value)
    except ValueError:
        raise ValueError("Expected a nonzero SHA256") from None
    if len(raw) != 32 or not any(raw):
        raise ValueError("Expected a nonzero SHA256")
    return raw


def read_exact(base, name, size):
    if not isinstance(name, str) or not name or Path(name).is_absolute():
        raise ValueError("Input must be a relative file beside its manifest")
    path = (base / name).resolve()
    if not path.is_relative_to(base) or not path.is_file() or path.stat().st_size != size:
        raise ValueError("Input file escapes manifest directory or has wrong byte count")
    with path.open("rb") as stream:
        data = stream.read(size + 1)
    if len(data) != size:
        raise ValueError("Input byte count changed during reading")
    return data


def opaque(raw):
    return all(value == 255 for value in raw[3::4])


def build(manifest_path, output):
    manifest_path = Path(manifest_path).resolve()
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    rows = manifest["images"]
    if not isinstance(rows, list) or not 1 <= len(rows) <= 98:
        raise ValueError("Expected between1and98 panorama bindings")
    records, chunks, receipt_rows, seen = [], [], [], set()
    total = 0
    for ordinal, row in enumerate(rows):
        width, height, center_width = row["width"], row["height"], row["centerWidth"]
        if (any(type(n) is not int for n in (width, height, center_width)) or
                not 0 < center_width < width <= 4096 or not 0 < height <= 4096 or
                center_width * 3 != height * 4 or (width - center_width) % 2 or
                width * height * 4 > MAX_IMAGE):
            raise ValueError("Invalid symmetric protected-center geometry")
        if row.get("reviewed") is not True:
            raise ValueError("Panorama requires explicit reviewed acceptance")
        key = tuple(digest(row[name]) for name in
                    ("archiveSha256", "entrySha256", "centerRgbaSha256", "footerRgbaSha256"))
        if key in seen:
            raise ValueError("Duplicate panorama routing key")
        seen.add(key)
        center = read_exact(manifest_path.parent, row["centerRaw"], center_width * height * 4)
        sides = read_exact(manifest_path.parent, row["sideRaw"], (width - center_width) * height * 4)
        if hashlib.sha256(center).digest() != key[2] or not opaque(center) or not opaque(sides):
            raise ValueError("Center identity or opaque RGBA validation failed")
        side_sha = hashlib.sha256(sides).digest()
        # Hash composed rows without retaining another full decoded canvas.
        side_row, center_row = (width - center_width) // 2 * 4, center_width * 4
        side_plane = side_row * height
        composed = hashlib.sha256()
        for y in range(height):
            composed.update(sides[y * side_row:(y + 1) * side_row])
            composed.update(center[y * center_row:(y + 1) * center_row])
            composed.update(sides[side_plane + y * side_row:side_plane + (y + 1) * side_row])
        composed_sha = composed.digest()
        for name, actual in (("sideRgbaSha256", side_sha), ("composedRgbaSha256", composed_sha)):
            if name in row and digest(row[name]) != actual:
                raise ValueError("Supplied pixel digest does not match raw bytes")
        chunk = zlib.compress(sides, 9)
        total += len(chunk)
        if not 0 < len(chunk) <= MAX_CHUNK or total > MAX_PAYLOAD:
            raise ValueError("Compressed side payload exceeds resource budget")
        decoder = zlib.decompressobj()
        check = decoder.decompress(chunk, len(sides) + 1)
        if check != sides or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise ValueError("Exact lossless side round trip failed")
        records.append(b"".join(key) + side_sha + composed_sha +
                       TAIL.pack(width, height, center_width, 1, len(chunk)))
        chunks.append(chunk)
        # Stable relative labels only; no filesystem path or private prompt leaks.
        receipt_rows.append({"label": "photo-%03d" % ordinal, "width": width, "height": height,
                             "centerWidth": center_width, "compressedBytes": len(chunk),
                             "centerRgbaSha256": key[2].hex(), "sideRgbaSha256": side_sha.hex(),
                             "composedRgbaSha256": composed_sha.hex(), "reviewed": True})
    data = HEADER.pack(MAGIC, 1, len(records), RECORD_BYTES, 0, total) + b"".join(records) + b"".join(chunks)
    receipt = {"version": 1, "images": len(records), "recordBytes": RECORD_BYTES,
               "fileBytes": len(data), "compressedSideBytes": total,
               "packSha256": hashlib.sha256(data).hexdigest(), "centerPixelsStored": False,
               "losslessVerified": True, "appIntegrated": False, "imagesMetadata": receipt_rows}
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    (output / "slideshow-panorama.ownedz").write_bytes(data)
    (output / "panorama-pack-receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    return receipt


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest")
    parser.add_argument("output")
    args = parser.parse_args()
    print(json.dumps(build(args.manifest, args.output)))
