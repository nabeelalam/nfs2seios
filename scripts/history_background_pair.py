#!/usr/bin/env python3
"""Derive matched History backgrounds and lossless wide side resources.

The manifest supplies sourceIndex, width, height and an opaque RGBA path per
panorama. Paths are relative to the private manifest. Existing packs are never
overwritten. Native text/title images and background logical bounds stay intact.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import zlib

from history_panorama_pack import build as build_panorama

MAX_PAYLOAD = 128 * 1024**2
MAX_FILE = 64 * 1024**2
MAX_RGBA = 32 * 1024**2


def sha(data):
    return hashlib.sha256(data).hexdigest()


def unpack_history(packed, expected_sha):
    if not isinstance(expected_sha, str) or len(expected_sha) != 64 or sha(packed) != expected_sha.lower():
        raise ValueError("Base History pack digest differs from pin")
    if not 16 < len(packed) <= MAX_FILE or packed[:8] != b"NFSHIST1":
        raise ValueError("Invalid History pack header")
    version, length = struct.unpack_from("<II", packed, 8)
    if version != 1 or not 8 <= length <= MAX_PAYLOAD:
        raise ValueError("Invalid History payload bounds")
    decoder = zlib.decompressobj()
    payload = decoder.decompress(packed[16:], length + 1)
    if len(payload) != length or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise ValueError("History stream is incomplete or has trailing data")
    count, reserved = struct.unpack_from("<II", payload)
    if count != 49 or reserved:
        raise ValueError("Expected complete History source catalog")
    rows = []
    at = 8
    for index in range(count):
        if at + 48 > len(payload):
            raise ValueError("Truncated History metadata")
        metadata = payload[at:at + 48]
        source, width, height, flags = struct.unpack_from("<4I", metadata)
        bounds = struct.unpack_from("<8f", metadata, 16)
        if source != index or flags or not (0 < width <= 8192 and 0 < height <= 8192):
            raise ValueError("Invalid History image metadata")
        if not all(math.isfinite(v) for v in bounds):
            raise ValueError("Invalid History image bounds")
        size = width * height * 4
        at += 48
        if size > MAX_PAYLOAD or at + size > len(payload):
            raise ValueError("Truncated History pixels")
        rows.append((metadata, payload[at:at + size]))
        at += size
    if at != len(payload):
        raise ValueError("History payload has trailing data")
    return rows


def derive(rows, panoramas):
    if not 1 <= len(panoramas) <= 7:
        raise ValueError("Expected one to seven backgrounds")
    changed = set()
    result = list(rows)
    side_files = {}
    replacements = []
    for source, width, height, rgba in panoramas:
        if source not in range(0, 49, 7) or source in changed:
            raise ValueError("Expected unique background source indices")
        if not (0 < width <= 4096 and 0 < height <= 4096) or width * 9 != height * 16 or width % 8:
            raise ValueError("Expected exact 16:9 dimensions with integral central crop")
        if len(rgba) != width * height * 4 or len(rgba) > MAX_RGBA or any(a != 255 for a in rgba[3::4]):
            raise ValueError("Expected bounded opaque RGBA panorama")
        crop_width = width * 3 // 4
        crop_left = width // 8
        if crop_width * 3 != height * 4:
            raise ValueError("Central crop is not exact 4:3")
        stride = width * 4
        crop = b"".join(rgba[y * stride + crop_left * 4:y * stride + (crop_left + crop_width) * 4]
                        for y in range(height))
        metadata = bytearray(result[source][0])
        struct.pack_into("<II", metadata, 4, crop_width, height)
        result[source] = (bytes(metadata), crop)
        side = build_panorama(rgba, width, height, source)
        side_files["history-panorama-" + str(source) + ".ownedz"] = side
        replacements.append({"sourceIndex": source, "panoramaSize": [width, height],
                             "cropRectangle": [crop_left, 0, crop_width, height],
                             "centerRGBA_SHA256": sha(crop), "panoramaRGBA_SHA256": sha(rgba),
                             "panoramaFileSHA256": sha(side), "panoramaFileBytes": len(side)})
        changed.add(source)
    payload = struct.pack("<II", 49, 0) + b"".join(meta + pixels for meta, pixels in result)
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("History payload exceeds production limit")
    packed = b"NFSHIST1" + struct.pack("<II", 1, len(payload)) + zlib.compress(payload, 9)
    if len(packed) > MAX_FILE:
        raise ValueError("History pack exceeds production limit")
    checked = unpack_history(packed, sha(packed))
    if checked != result:
        raise ValueError("History lossless round trip failed")
    for index in range(49):
        if index not in changed and checked[index] != rows[index]:
            raise ValueError("Unrelated History source changed")
    receipt = {"schema": 1, "scope": "Matched private History background and panorama resources",
               "baseHistorySHA256": None, "historySHA256": sha(packed),
               "historyFileBytes": len(packed), "historyPayloadBytes": len(payload),
               "imageCount": 49, "unchangedSourceCount": 49 - len(changed),
               "losslessVerified": True, "replacements": replacements,
               "artworkApproved": False, "runtimeVerified": False, "physicalIpadVerified": False}
    return packed, side_files, receipt


def bounded_read(path, maximum):
    if path.stat().st_size > maximum:
        raise ValueError("Input exceeds resource limit")
    with path.open("rb") as stream:
        data = stream.read(maximum + 1)
    if len(data) > maximum:
        raise ValueError("Input exceeds resource limit")
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", required=True, type=Path)
    parser.add_argument("--base-sha256", required=True)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    args = parser.parse_args()
    base = bounded_read(args.base, MAX_FILE)
    rows = unpack_history(base, args.base_sha256)
    manifest = json.loads(bounded_read(args.manifest, 64 * 1024))
    if not isinstance(manifest, list) or not 1 <= len(manifest) <= 7:
        raise ValueError("Expected one to seven manifest entries")
    panoramas = []
    for entry in manifest:
        if not isinstance(entry, dict) or set(entry) != {"sourceIndex", "width", "height", "rgba"}:
            raise ValueError("Unexpected manifest fields")
        values = [entry[key] for key in ("sourceIndex", "width", "height")]
        if any(type(value) is not int for value in values) or not isinstance(entry["rgba"], str):
            raise ValueError("Invalid manifest values")
        path = args.manifest.parent / entry["rgba"]
        panoramas.append((*values, bounded_read(path, MAX_RGBA)))
    packed, sides, receipt = derive(rows, panoramas)
    receipt["baseHistorySHA256"] = sha(base)
    args.output_dir.mkdir(parents=False, exist_ok=False)
    outputs = {"history-ui.ownedz": packed, **sides,
               "PAIR-RECEIPT.json": (json.dumps(receipt, indent=2) + "\n").encode()}
    for name, content in outputs.items():
        with (args.output_dir / name).open("xb") as stream:
            stream.write(content)
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
