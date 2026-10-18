"""Read FSH directory/entry metadata locally; never modify or export artwork."""
import argparse
import json
import struct
from pathlib import Path

LIMIT = 32 * 1024 * 1024


def inspect(data):
    if len(data) < 16 or len(data) > LIMIT:
        raise ValueError("Archive size outside inspection bounds")
    magic, size, count, directory = struct.unpack_from("<4sII4s", data)
    if magic not in (b"SHPI", b"SHPP") or size != len(data):
        raise ValueError("Expected an uncompressed FSH archive with exact declared size")
    if count > 4096 or 16 + count * 8 > len(data):
        raise ValueError("Invalid directory count")
    entries = [struct.unpack_from("<4sI", data, 16 + 8 * i) for i in range(count)]
    first_allowed = 16 + count * 8
    offsets = sorted({offset for _, offset in entries})
    if any(offset < first_allowed or offset > len(data) - 16 for offset in offsets):
        raise ValueError("Entry header outside archive body")
    ends = dict(zip(offsets, offsets[1:] + [len(data)]))
    records = []
    for name, offset in entries:
        if ends[offset] - offset < 16:
            raise ValueError("Overlapping entry headers")
        code, width, height, *misc = struct.unpack_from("<Ihh4h", data, offset)
        records.append({"name": name.decode("ascii", errors="replace"),
                        "offset": offset, "format_code": code & 127,
                        "entry_compressed": bool(code & 128),
                        "next_attachment_offset": code >> 8,
                        "width_raw": width, "height_raw": height,
                        "misc_raw": misc, "span_bytes": ends[offset] - offset})
    return {"schema": "nfsiise-fsh-inventory-v1", "directory": directory.decode("ascii", errors="replace"),
            "archive_bytes": size, "entry_count": count, "entries": records,
            "scope": "Directory and entry metadata only; image decode, alpha and draw-path identity unverified"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    args = parser.parse_args()
    try:
        with args.archive.open("rb") as stream:
            data = stream.read(LIMIT + 1)
        print(json.dumps(inspect(data), indent=2))
    except (OSError, ValueError, struct.error):
        parser.exit(1, "Unable to inspect: unsupported or malformed FSH archive.\n")


if __name__ == "__main__":
    main()
