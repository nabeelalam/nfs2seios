"""Reconstruct one trusted optional artwork release from separately owned assets.

The separate owner-art template contains authored payloads and compatibility
metadata, with 24 original resource headers omitted. Original files are read
locally; only a complete, authenticated result may become a new output file.
"""
# SPDX-License-Identifier: MIT
import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import struct
import tempfile

MAGIC = b"OWNART01"
MAX_TEMPLATE = 96 * 1024 * 1024
MAX_ORIGINAL = 8 * 1024 * 1024
HEADER_COUNT = 24
HEADER_BYTES = 16
MAX_DIRECTORY_ENTRIES = 4096
OWNER_FILE = "fedata/pc/art/main.qfs"
META_FIELDS = {"schema", "outputBytes", "outputSha256", "ownerFile",
               "ownerSha256", "decodedSha256", "holes", "payloadSha256"}
HOLE_FIELDS = {"outputOffset", "bytes", "entry", "sourceOffset", "span",
               "entrySha256"}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def valid_digest(value):
    return (type(value) is str and len(value) == 64
            and all(c in "0123456789abcdef" for c in value))


@dataclass(frozen=True)
class TrustedRecipe:
    """Explicit trusted dependency for a release or independent synthetic tests.

    The command line always uses KNOWN_RELEASE. Template-supplied hashes cannot
    replace these trust anchors, and there is no verification-disable option.
    """
    template_sha256: str
    output_sha256: str

    def __post_init__(self):
        require(valid_digest(self.template_sha256)
                and valid_digest(self.output_sha256), "Invalid trusted recipe")


KNOWN_RELEASE = TrustedRecipe(
    "28aa4b2f05b10647059215fddd25e0c07a1dbed9020b52594f1ebd2285fd3d4f",
    "6ad480d154cae6328e7936e6611d58c7d9df4bac687d889aaf0705215b5007bc")


def decode_qfs(data):
    """Bounded RefPack stream used by the supported owner archive."""
    require(type(data) is bytes and 5 <= len(data) <= MAX_ORIGINAL,
            "QFS input bounds")
    require(data[:2] == b"\x10\xfb", "Unsupported QFS header")
    length = int.from_bytes(data[2:5], "big")
    require(16 <= length <= MAX_ORIGINAL, "QFS output bounds")
    output, at, terminal = bytearray(), 5, False
    while at < len(data):
        command = data[at]
        at += 1
        back, count = 0, 0
        if command < 0x80:
            require(at < len(data), "Truncated short QFS command")
            b = data[at]
            at += 1
            literal = command & 3
            count = ((command & 28) >> 2) + 3
            back = ((command & 96) << 3) + b + 1
        elif command < 0xc0:
            require(at + 2 <= len(data), "Truncated medium QFS command")
            b, c = data[at:at + 2]
            at += 2
            literal, count = b >> 6, (command & 63) + 4
            back = ((b & 63) << 8) + c + 1
        elif command < 0xe0:
            require(at + 3 <= len(data), "Truncated long QFS command")
            b, c, e = data[at:at + 3]
            at += 3
            literal, count = command & 3, ((command & 12) << 6) + e + 5
            back = ((command & 16) << 12) + (b << 8) + c + 1
        elif command < 0xfc:
            literal = ((command & 31) << 2) + 4
        else:
            literal, terminal = command & 3, True
        require(at + literal <= len(data)
                and len(output) + literal + count <= length, "QFS copy bounds")
        output.extend(data[at:at + literal])
        at += literal
        if count:
            require(1 <= back <= len(output), "QFS back reference bounds")
            for unused in range(count):
                output.append(output[-back])
        if terminal:
            break
    require(terminal and at == len(data) and len(output) == length,
            "QFS terminal/length mismatch")
    return bytes(output)


def inspect_fsh(data):
    """Directory invariants match the bounded archive metadata inspector."""
    require(type(data) is bytes and 16 <= len(data) <= MAX_ORIGINAL,
            "FSH size bounds")
    magic, size, count, unused = struct.unpack_from("<4sII4s", data)
    require(magic == b"SHPI" and size == len(data)
            and 0 < count <= 4096 and 16 + 8 * count <= len(data),
            "FSH header bounds")
    entries = [struct.unpack_from("<4sI", data, 16 + 8 * i)
               for i in range(count)]
    require(len({name for name, offset in entries}) == count,
            "FSH duplicate names")
    offsets = sorted(offset for name, offset in entries)
    require(len(set(offsets)) == count
            and all(16 + 8 * count <= offset <= size - HEADER_BYTES
                    for offset in offsets), "FSH entry bounds")
    ends = dict(zip(offsets, offsets[1:] + [size]))
    result = {}
    for name, offset in entries:
        require(ends[offset] - offset >= HEADER_BYTES,
                "FSH overlapping headers")
        result[name.decode("ascii")] = (offset, ends[offset] - offset)
    return result


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "Duplicate JSON field")
        result[key] = value
    return result


def parse_template(data, recipe):
    require(isinstance(recipe, TrustedRecipe), "Trusted recipe required")
    require(type(data) is bytes and 16 <= len(data) <= MAX_TEMPLATE,
            "Template bounds")
    require(sha256(data) == recipe.template_sha256,
            "Untrusted or tampered template")
    require(data[:8] == MAGIC, "Expected an owner-art container")
    size = struct.unpack_from("<Q", data, 8)[0]
    require(0 < size <= 65536 and 16 + size < len(data),
            "Template metadata bounds")
    metadata = json.loads(data[16:16 + size], object_pairs_hook=unique_object)
    require(type(metadata) is dict and set(metadata) == META_FIELDS,
            "Template fields")
    require(type(metadata["schema"]) is int and metadata["schema"] == 1,
            "Template schema")
    require(type(metadata["outputBytes"]) is int
            and HEADER_COUNT * HEADER_BYTES < metadata["outputBytes"]
            <= MAX_TEMPLATE, "Template output bounds")
    for key in ("outputSha256", "ownerSha256", "decodedSha256", "payloadSha256"):
        require(valid_digest(metadata[key]), "Template digest type")
    require(metadata["outputSha256"] == recipe.output_sha256,
            "Template output pin")
    require(type(metadata["ownerFile"]) is str
            and metadata["ownerFile"] == OWNER_FILE, "Owner asset path")
    holes = metadata["holes"]
    require(type(holes) is list and len(holes) == HEADER_COUNT,
            "Template header inventory")
    previous, seen = 0, set()
    for hole in holes:
        require(type(hole) is dict and set(hole) == HOLE_FIELDS,
                "Header locator fields")
        require(all(type(hole[k]) is int for k in
                    ("outputOffset", "bytes", "sourceOffset", "span")),
                "Header locator numeric types")
        require(previous <= hole["outputOffset"] and hole["bytes"] == HEADER_BYTES
                and hole["outputOffset"] + HEADER_BYTES <= metadata["outputBytes"],
                "Header overlap/output bounds")
        name = hole["entry"]
        require(type(name) is str and len(name) == 4
                and all(32 <= ord(c) <= 126 and c not in "/\\" for c in name)
                and name not in seen, "Header entry name")
        seen.add(name)
        require(0 <= hole["sourceOffset"] <= MAX_ORIGINAL - HEADER_BYTES
                and HEADER_BYTES <= hole["span"]
                <= MAX_ORIGINAL - hole["sourceOffset"], "Header source bounds")
        require(valid_digest(hole["entrySha256"]), "Header digest type")
        previous = hole["outputOffset"] + HEADER_BYTES
    payload = data[16 + size:]
    require(len(payload) == metadata["outputBytes"] - HEADER_COUNT * HEADER_BYTES
            and sha256(payload) == metadata["payloadSha256"],
            "Template payload bounds/digest")
    return metadata, payload


def reconstruct(template, owner, recipe=KNOWN_RELEASE):
    metadata, payload = parse_template(template, recipe)
    require(type(owner) is bytes and sha256(owner) == metadata["ownerSha256"],
            "Wrong or mutated owner asset")
    decoded = decode_qfs(owner)
    require(sha256(decoded) == metadata["decodedSha256"],
            "Decoded owner asset mismatch")
    entries, headers = inspect_fsh(decoded), []
    for hole in metadata["holes"]:
        require(entries.get(hole["entry"]) == (hole["sourceOffset"], hole["span"]),
                "Owner source locator mismatch")
        body = decoded[hole["sourceOffset"]:hole["sourceOffset"] + hole["span"]]
        require(sha256(body) == hole["entrySha256"], "Owner entry identity mismatch")
        headers.append(body[:HEADER_BYTES])
    output, cursor = bytearray(), 0
    for hole, header in zip(metadata["holes"], headers):
        count = hole["outputOffset"] - len(output)
        require(0 <= count <= len(payload) - cursor, "Template segment bounds")
        output.extend(payload[cursor:cursor + count])
        cursor += count
        output.extend(header)
    output.extend(payload[cursor:])
    require(len(output) == metadata["outputBytes"]
            and sha256(output) == recipe.output_sha256,
            "Reconstructed whole-pack mismatch")
    return bytes(output)


def real_path(path, directory=False):
    """Reject symlinks in the specified path, including parent components."""
    path = Path(path).absolute()
    require(not any(part.is_symlink() for part in (path, *path.parents)),
            "Symbolic links are not accepted")
    resolved = path.resolve(strict=True)
    require(resolved.is_dir() if directory else resolved.is_file(),
            "Expected a regular directory" if directory else "Expected a regular file")
    return resolved


def resolve_owner_asset(owner_root):
    root = real_path(owner_root, directory=True)
    current = root
    for component in PurePosixPath(OWNER_FILE).parts:
        matches = []
        for index, item in enumerate(current.iterdir()):
            require(index < MAX_DIRECTORY_ENTRIES, "Owner directory inventory bounds")
            if item.name.casefold() == component.casefold():
                matches.append(item)
                require(len(matches) <= 1, "Ambiguous owner asset component")
        require(len(matches) == 1, "Missing or ambiguous owner asset component")
        current = matches[0]
        require(not current.is_symlink(), "Owner asset symlink")
        require(current.resolve(strict=True).is_relative_to(root),
                "Owner asset escaped root")
        if component != PurePosixPath(OWNER_FILE).parts[-1]:
            require(current.is_dir(), "Owner asset parent is not a directory")
    return root, real_path(current)


def read_regular(path, limit):
    path = real_path(path)
    require(0 < path.stat().st_size <= limit, "Input file bounds")
    with path.open("rb") as stream:
        data = stream.read(limit + 1)
    require(0 < len(data) <= limit, "Changed input bounds")
    return data


def assemble(template_path, owner_root, output, recipe=KNOWN_RELEASE):
    root, source = resolve_owner_asset(owner_root)
    original = read_regular(source, MAX_ORIGINAL)
    before = sha256(original)
    template = read_regular(template_path, MAX_TEMPLATE)
    result = reconstruct(template, original, recipe)
    target = Path(output).absolute()
    parent = real_path(target.parent, directory=True)
    require(not parent.is_relative_to(root), "Output must be outside owner game folder")
    require(target.name not in ("", ".", "..") and not target.is_symlink()
            and not target.exists(), "Output must be a new regular file")
    require(sha256(read_regular(source, MAX_ORIGINAL)) == before,
            "Owner source changed during assembly")
    temporary = None
    try:
        fd, name = tempfile.mkstemp(prefix=".owner-art-", suffix=".tmp", dir=parent)
        temporary = Path(name)
        with os.fdopen(fd, "wb") as stream:
            stream.write(result)
            stream.flush()
            os.fsync(stream.fileno())
        require(sha256(read_regular(temporary, MAX_TEMPLATE)) == recipe.output_sha256,
                "Staged output changed")
        require(real_path(target.parent, directory=True) == parent,
                "Output parent changed")
        require(sha256(read_regular(source, MAX_ORIGINAL)) == before,
                "Owner source changed before publication")
        # Atomic create only: no replacement fallback, including on filesystems
        # that do not support hard links. A collision leaves the existing file.
        os.link(temporary, parent / target.name)
    finally:
        if temporary is not None:
            temporary.unlink()
    return {"bytes": len(result), "sha256": recipe.output_sha256,
            "ownerUnchanged": sha256(read_regular(source, MAX_ORIGINAL)) == before}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--template", type=Path, required=True)
    parser.add_argument("--owner-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        print(json.dumps(assemble(args.template, args.owner_root, args.output)))
    except (OSError, ValueError, UnicodeError, struct.error) as error:
        parser.exit(1, "Owner artwork assembly refused: " + str(error) + "\n")


if __name__ == "__main__":
    main()
