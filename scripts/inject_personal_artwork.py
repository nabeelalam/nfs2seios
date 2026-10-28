"""Add one authenticated personal artwork resource to a verified unsigned iPad IPA.

No signing, credentials, extraction, native-code changes or Mac is involved.
The input IPA and original game files are never modified. Python3 stdlib only.
"""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import plistlib
import re
import shutil
import stat
import struct
import tempfile
import unicodedata
import zipfile

TEMPLATE_BYTES = 8909910
TEMPLATE_SHA256 = "90d429cceb1965894ae650b665290aa549eb1d653d0879bb2f6622747eb6a57d"
OUTPUT_PACK_SHA256 = "8d60efeb04d3cc4e7bf665746f33fbb0bab91d2d573648f1af0ab74b6af3243c"
RESOURCE = "classic-ui.ownedz"
MAX_IPA = 512 * 1024 * 1024
MAX_UNPACKED = 1024 * 1024 * 1024
MAX_MEMBERS = 10000
CHUNK = 1024 * 1024


def require(value, message):
    if not value:
        raise ValueError(message)


def digest_file(path, limit=MAX_IPA):
    h = hashlib.sha256()
    count = 0
    with Path(path).open("rb") as f:
        for b in iter(lambda: f.read(CHUNK), b""):
            count += len(b)
            require(count <= limit, "File grew beyond size limit")
            h.update(b)
    return h.hexdigest()


def safe_name(info, allow_artwork=False):
    name = info.orig_filename
    require(name == info.filename and name and "\x00" not in name,
            "Invalid ZIP member name")
    require("\\" not in name and ":" not in name and not name.startswith("/"),
            "Absolute or non-portable ZIP member path")
    require(unicodedata.normalize("NFC", name) == name,
            "Ambiguous Unicode ZIP member path")
    parts = name.rstrip("/").split("/")
    require(all(p and p not in (".", "..") for p in parts),
            "ZIP path traversal or empty component")
    require(parts[0] == "Payload", "Unexpected content outside Payload")
    require(not info.flag_bits & 1, "Encrypted ZIP member")
    require(info.compress_type in (zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED),
            "Unsupported ZIP compression")
    mode = (info.external_attr >> 16) & 0xFFFF
    kind = stat.S_IFMT(mode)
    require(kind in (0, stat.S_IFREG, stat.S_IFDIR), "ZIP symlink or special file")
    if info.is_dir():
        require(kind in (0, stat.S_IFDIR) and info.file_size == 0,
                "Invalid ZIP directory")
    else:
        require(kind in (0, stat.S_IFREG), "Invalid ZIP regular file")
    folded = [p.casefold() for p in parts]
    require(not any(p in ("_codesignature", "coderesources", "sc_info") for p in folded),
            "Signed or protected bundle; use the unsigned base IPA")
    require(folded[-1] not in ("embedded.mobileprovision", "embedded.provisionprofile"),
            "Provisioned bundle; use the unsigned base IPA")
    require(allow_artwork or folded[-1] != RESOURCE, "Artwork resource already present")
    return name, parts


def macho_check(prefix, stream, name, main):
    # Thin64little-endian MachO; public Apple build is an unsigned ARM64 device app.
    magic = prefix[:4]
    known = {b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe", b"\xfe\xed\xfa\xcf",
             b"\xfe\xed\xfa\xce", b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca",
             b"\xca\xfe\xba\xbf", b"\xbf\xba\xfe\xca"}
    if magic not in known:
        require(not main, "Main executable is not a Mach-O binary")
        return prefix, False
    require(magic == b"\xcf\xfa\xed\xfe" and len(prefix) == 32,
            "Unexpected Mach-O architecture/container")
    _, cpu, subtype, filetype, commands, command_bytes, flags, reserved = struct.unpack("<IiiIIIII", prefix)
    require(cpu == 0x0100000C and commands <= 4096 and command_bytes <= 4 * CHUNK,
            "Expected bounded ARM64 Mach-O")
    if main:
        require(filetype == 2, "Main binary is not a Mach-O executable")
    table = stream.read(command_bytes)
    require(len(table) == command_bytes, "Truncated Mach-O load commands")
    at = 0
    for unused in range(commands):
        require(at + 8 <= len(table), "Mach-O command bounds")
        cmd, size = struct.unpack_from("<II", table, at)
        require(size >= 8 and size % 8 == 0 and size <= len(table) - at,
                "Mach-O command size")
        if cmd == 0x1D:  # LC_CODE_SIGNATURE
            require(size >= 16, "Mach-O signature command size")
            offset, count = struct.unpack_from("<II", table, at + 8)
            require(offset == 0 and count == 0,
                    "Mach-O contains a signature; use the unsigned base IPA")
        if cmd in (0x21, 0x2C):  # LC_ENCRYPTION_INFO / _64
            require(size >= 20 and struct.unpack_from("<I", table, at + 16)[0] == 0,
                    "Encrypted Mach-O cannot be repackaged")
        at += size
    require(at == len(table), "Mach-O load-command length mismatch")
    return prefix + table, True


def raw_storage(path, info):
    with Path(path).open("rb") as f:
        f.seek(info.header_offset)
        header = f.read(30)
        require(len(header) == 30 and header[:4] == b"PK\x03\x04", "ZIP local header")
        filename_bytes, extra_bytes = struct.unpack_from("<HH", header, 26)
        count = 30 + filename_bytes + extra_bytes + info.compress_size
        require(info.header_offset + count <= Path(path).stat().st_size,
                "ZIP member storage bounds")
        f.seek(info.header_offset)
        h = hashlib.sha256()
        remaining = count
        while remaining:
            data = f.read(min(remaining, CHUNK))
            require(data, "Truncated ZIP member storage")
            h.update(data)
            remaining -= len(data)
    return (info.header_offset, count, h.hexdigest())


def inspect(path, allow_resource=False):
    # Complete stream reads also validate ZIP CRCs; no member is extracted to disk.
    result = {}
    storage = {}
    machos = {}
    with zipfile.ZipFile(path, "r") as z:
        infos = z.infolist()
        require(0 < len(infos) <= MAX_MEMBERS, "ZIP member-count limit")
        seen = set()
        app_roots = set()
        total = 0
        members = {}
        for info in infos:
            if allow_resource and info.filename.endswith("/" + RESOURCE):
                # The new entry is checked against its one exact expected path below.
                require(info.filename.count("/") == 2 and info.filename.startswith("Payload/"),
                        "Unexpected artwork entry location")
                name, parts = safe_name(info, allow_artwork=True)
                require(parts[1].endswith(".app") and not info.is_dir()
                        and info.file_size == TEMPLATE_BYTES
                        and info.compress_type == zipfile.ZIP_STORED
                        and not info.flag_bits & 1, "Invalid injected artwork entry")
            else:
                name, parts = safe_name(info)
            key = name.rstrip("/").casefold()
            require(key not in seen, "Duplicate or case-ambiguous ZIP member")
            seen.add(key)
            if len(parts) >= 2:
                require(parts[1].endswith(".app"), "Unexpected Payload member")
                app_roots.add("Payload/" + parts[1])
            total += info.file_size
            require(info.file_size <= MAX_IPA and total <= MAX_UNPACKED,
                    "ZIP uncompressed-size limit")
            require(info.file_size <= 1024 * max(info.compress_size, 1) or info.file_size < CHUNK,
                    "Unexpected ZIP compression ratio")
            members[name] = info
        require(len(app_roots) == 1, "Expected exactly one app in Payload")
        app = next(iter(app_roots))
        plist_name = app + "/Info.plist"
        require(plist_name in members and members[plist_name].file_size <= CHUNK,
                "Missing/bounded app Info.plist")
        plist = plistlib.loads(z.read(plist_name))
        executable = plist.get("CFBundleExecutable")
        require(isinstance(executable, str) and executable
                and PurePosixPath(executable).name == executable
                and "/" not in executable and "\\" not in executable and ":" not in executable,
                "Unsafe app executable name")
        main_name = app + "/" + executable
        require(main_name in members and not members[main_name].is_dir(),
                "Main executable missing")
        for name, info in members.items():
            if info.is_dir():
                result[name] = hashlib.sha256(b"").hexdigest()
            else:
                with z.open(info) as f:
                    prefix = f.read(32)
                    first, macho = macho_check(prefix, f, name, name == main_name)
                    h = hashlib.sha256(first)
                    count = len(first)
                    while True:
                        b = f.read(CHUNK)
                        if not b:
                            break
                        count += len(b)
                        require(count <= info.file_size, "ZIP member grew")
                        h.update(b)
                    require(count == info.file_size, "ZIP member length mismatch")
                    result[name] = h.hexdigest()
                    if macho:
                        machos[name] = h.hexdigest()
            storage[name] = raw_storage(path, info)
        require(main_name in machos, "Main executable was not validated")
        ranges = sorted((offset, offset + count) for offset, count, unused in storage.values())
        require(all(ranges[i-1][1] <= ranges[i][0] for i in range(1, len(ranges))),
                "Overlapping ZIP member storage")
        return dict(app=app, members=result, storage=storage, machos=machos,
                    unpackedBytes=total, comment=z.comment)


def inject(base_ipa, base_sha256, template, output):
    require(re.fullmatch("[0-9a-f]{64}", base_sha256 or ""), "Provide the trusted base IPA SHA-256")
    base = Path(base_ipa).resolve(strict=True)
    source = Path(template).resolve(strict=True)
    target_raw = Path(output).absolute()
    parent = target_raw.parent.resolve(strict=True)
    target = parent / target_raw.name
    require(base.is_file() and source.is_file() and parent.is_dir(), "Expected regular input files/output folder")
    require(not target.exists() and not target.is_symlink() and target not in (base, source),
            "Output must be a new file")
    require(0 < base.stat().st_size <= MAX_IPA - TEMPLATE_BYTES - 65536, "Base IPA size limit")
    require(source.stat().st_size == TEMPLATE_BYTES, "Wrong personal artwork size")
    with source.open("rb") as f:
        artwork = f.read(TEMPLATE_BYTES + 1)
    require(len(artwork) == TEMPLATE_BYTES and hashlib.sha256(artwork).hexdigest() == TEMPLATE_SHA256,
            "Wrong or mutated personal artwork template")
    require(digest_file(base) == base_sha256, "Base IPA identity mismatch")
    with base.open("rb") as f:
        require(f.read(4) == b"PK\x03\x04", "Expected a plain IPA ZIP archive")
    before = inspect(base)
    resource = before["app"] + "/" + RESOURCE
    temporary = None
    try:
        fd, name = tempfile.mkstemp(prefix=".personal-classic-ui-", suffix=".ipa.tmp", dir=parent)
        temporary = Path(name)
        with os.fdopen(fd, "wb") as out, base.open("rb") as inp:
            h = hashlib.sha256()
            copied = 0
            while True:
                b = inp.read(CHUNK)
                if not b:
                    break
                copied += len(b)
                require(copied <= MAX_IPA - TEMPLATE_BYTES - 65536, "Base grew while copying")
                h.update(b)
                out.write(b)
            out.flush()
            os.fsync(out.fileno())
        require(h.hexdigest() == base_sha256, "Base IPA changed while copying")
        # Append preserves every original local header and compressed member byte.
        with zipfile.ZipFile(temporary, "a", allowZip64=True) as z:
            member = zipfile.ZipInfo(resource, (1980, 1, 1, 0, 0, 0))
            member.create_system = 3
            member.external_attr = (stat.S_IFREG | 0o644) << 16
            member.compress_type = zipfile.ZIP_STORED
            z.writestr(member, artwork)
        require(temporary.stat().st_size <= MAX_IPA, "Finished IPA size limit")
        after = inspect(temporary, allow_resource=True)
        require(after["app"] == before["app"] and after["comment"] == before["comment"],
                "App identity or ZIP comment changed")
        require(set(after["members"]) == set(before["members"]) | {resource},
                "Unexpected added/removed IPA member")
        require(after["members"][resource] == TEMPLATE_SHA256,
                "Injected artwork identity mismatch")
        require(all(after["members"][k] == v for k, v in before["members"].items()),
                "Existing executable/resource bytes changed")
        require(all(after["storage"][k] == v for k, v in before["storage"].items()),
                "Existing ZIP member storage changed")
        require(after["machos"] == before["machos"], "Mach-O bytes changed")
        require(digest_file(base) == base_sha256 and digest_file(source) == TEMPLATE_SHA256,
                "An input changed")
        result = dict(schema=1, packageKind="private-personal-classic-ui",
                      personalArtworkPresent=True, publicReleaseReady=False,
                      personalSigningRequired=True, installationVerified=False,
                      gameplayVerified=False, baseIpaSha256=base_sha256,
                      outputIpaSha256=digest_file(temporary),
                      outputBytes=temporary.stat().st_size,
                      templateSha256=TEMPLATE_SHA256, templateBytes=TEMPLATE_BYTES,
                      reconstructedPackSha256=OUTPUT_PACK_SHA256,
                      resourceEntry=resource, addedMembers=1,
                      existingMembersPreserved=len(before["members"]),
                      allExistingMemberBytesIdentical=True,
                      allExistingCompressedStorageIdentical=True,
                      machOHashes=after["machos"], signed=False,
                      originalInputsUnchanged=True)
        # Atomic create-only publication: no overwrite fallback on unsupported FS.
        os.link(temporary, target)
        return result
    finally:
        if temporary is not None:
            temporary.unlink()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--base-ipa", type=Path, required=True)
    p.add_argument("--base-sha256", required=True,
                   help="Trusted SHA-256 from the public unsigned build artifact")
    p.add_argument("--template", type=Path, required=True)
    p.add_argument("--output", type=Path, required=True)
    args = p.parse_args()
    print(json.dumps(inject(args.base_ipa, args.base_sha256, args.template, args.output), indent=2))


if __name__ == "__main__":
    main()
