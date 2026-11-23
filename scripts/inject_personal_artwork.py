"""Add authenticated personal artwork resources to a verified unsigned iPad IPA.

No signing, credentials, extraction, native-code changes or Mac is involved.
The input IPA and original game files are never modified. Python3 stdlib only.
"""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import importlib.util
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
_metadata_spec = importlib.util.spec_from_file_location("slideshow_metadata", Path(__file__).with_name("slideshow_metadata.py"))
_metadata_module = importlib.util.module_from_spec(_metadata_spec)
_metadata_spec.loader.exec_module(_metadata_module)
compiled_catalogs = _metadata_module.compiled_catalogs

TEMPLATE_BYTES = 16227314
TEMPLATE_SHA256 = "09745bbf4577925f0426d11cbfab932dd9d6e1c096e55e16beadf4a3b5fa4ae7"
OUTPUT_PACK_SHA256 = "8e83caefa2c88691b9d020d7193148f4789681e7918ab74df7cef67f5a8609b0"
RESOURCE = "classic-ui.ownedz"
CREDITS_RESOURCE = "credits-ui.ownedz"
CREDITS_BYTES = 3189214
CREDITS_SHA256 = "a82ec61a8f1d26ecd2dc384f408220a262c970648c320ddb52d14b4343bab1d0"
INTRO_RESOURCE = "intro-ui.ownedz"
INTRO_BYTES = 6187645
INTRO_SHA256 = "bff0d7b7d7f97191583d86833d003d9907c5163f93fb1dc7c0a1f6fccd8ae8f2"
TRACK_INFO_RESOURCE = "track-info-ui.ownedz"
TRACK_INFO_BYTES = 36501092
TRACK_INFO_SHA256 = "ebb70bf576f21ad46bb926c4d282c5ac26e002ea6f53c6e1a292f342fbad711d"
CAR_INFO_RESOURCE = "car-info-ui.ownedz"
CAR_INFO_BYTES = 85094133
CAR_INFO_SHA256 = "cfadc3458f55bb015aa925430217c74ea103acbaa6ddc9139ce7f2784d5c6f2d"
PLAYER_PHOTO_RESOURCE = "player-photo-ui.ownedz"
PLAYER_PHOTO_BYTES = 27187927
PLAYER_PHOTO_SHA256 = "27b01190715f4369ebc3152f34164cdd4daed76a6b16a3241514b8e70587cad8"
LOCATION_PHOTO_RESOURCE = "location-photo-ui.ownedz"
LOCATION_PHOTO_BYTES = 9468676
LOCATION_PHOTO_SHA256 = "e78edb5444ecdfbfc1604b3cf2c554e6d080d7079411696c7e437c69bc75d340"
TRACK_BADGE_RESOURCE = "track-badge-ui.ownedz"
TRACK_BADGE_BYTES = 11691617
TRACK_BADGE_SHA256 = "9444929865bca655ca8796d95596233c5337116dae86feb32590a6df7ffdda3d"

WIDE_MENU_RESOURCE = "wide-menu-ui.ownedz"
WIDE_MENU_BYTES = 4822825
WIDE_MENU_SHA256 = "c8bba49686d875377d929b0bd5f3d2836c8544d2382ee82f72f031f8ffb1d5e4"

RESULTS_BACKDROP_RESOURCE = "results-backdrop-ui.ownedz"
RESULTS_BACKDROP_BYTES = 8286561
RESULTS_BACKDROP_SHA256 = "1c41d5d60cf951cc5bc134fcfc2f8ee9f483ff5e5cdfe0cfaac0325103096969"


def _load_slideshow_release():
    path = Path(__file__).resolve().parents[1] / "src/hd/ClassicScreens/SlideshowArtworkRelease.json"
    if path.stat().st_size > 4096:
        raise ValueError("Slideshow release catalog exceeds limit")
    value = json.loads(path.read_bytes())
    if (set(value) != {"schema", "filename", "fileBytes", "sha256", "formatVersion", "imageCount"}
            or value["schema"] != 1 or value["filename"] != "slideshow-ui.ownedz"
            or value["formatVersion"] != 4 or value["imageCount"] != 98
            or type(value["fileBytes"]) is not int or not 16 < value["fileBytes"] <= 512 * 1024 * 1024
            or not isinstance(value["sha256"], str)
            or not re.fullmatch(r"[0-9a-f]{64}", value["sha256"]) or value["sha256"] == "0" * 64):
        raise ValueError("Invalid Slideshow release catalog")
    return value


_SLIDESHOW_RELEASE = _load_slideshow_release()
SLIDESHOW_RESOURCE = _SLIDESHOW_RELEASE["filename"]
SLIDESHOW_BYTES = _SLIDESHOW_RELEASE["fileBytes"]
SLIDESHOW_SHA256 = _SLIDESHOW_RELEASE["sha256"]
SLIDESHOW_FOOTER_RESOURCE = "slideshow-footer.ownedz"
SLIDESHOW_FOOTER_BYTES = 5321403
SLIDESHOW_FOOTER_SHA256 = "bbd3016b6a3b21d36f84ae859a267cc70d653b5a7191585e9cdaea4428675c91"

# This optional catalog contains fingerprints only, never artwork payloads.
_panorama_spec = importlib.util.spec_from_file_location(
    "slideshow_panorama_release", Path(__file__).with_name("slideshow_panorama_release.py"))
_panorama_module = importlib.util.module_from_spec(_panorama_spec)
_panorama_spec.loader.exec_module(_panorama_module)
_SLIDESHOW_PANORAMA_RELEASE = _panorama_module.load(
    Path(__file__).resolve().parents[1] / _panorama_module.CATALOG)
SLIDESHOW_PANORAMA_RESOURCE = "slideshow-panorama.ownedz"


def _load_history_release():
    path = Path(__file__).resolve().parents[1] / "src/hd/ClassicScreens/HistoryArtworkRelease.json"
    if path.stat().st_size > 65536:
        raise ValueError("History release catalog exceeds limit")
    catalog = json.loads(path.read_bytes())
    if set(catalog) != {"schema", "history", "panoramas"} or catalog["schema"] != 1:
        raise ValueError("Invalid History release catalog")
    history = catalog["history"]
    if (set(history) != {"filename", "bytes", "payloadBytes", "sha256"}
            or history["filename"] != "history-ui.ownedz"
            or type(history["bytes"]) is not int or not 16 < history["bytes"] <= 64 * 1024**2
            or type(history["payloadBytes"]) is not int or not 8 <= history["payloadBytes"] <= 128 * 1024**2
            or not isinstance(history["sha256"], str)
            or not re.fullmatch("[0-9a-f]{64}", history["sha256"])):
        raise ValueError("Invalid History background release")
    panoramas = catalog["panoramas"]
    if not isinstance(panoramas, list) or len(panoramas) != 7:
        raise ValueError("Expected seven History panorama pins")
    for index, row in enumerate(panoramas):
        if (set(row) != {"sourceIndex", "filename", "bytes", "sha256"}
                or type(row["sourceIndex"]) is not int or row["sourceIndex"] != index * 7
                or row["filename"] != "history-panorama-" + str(index * 7) + ".ownedz"
                or type(row["bytes"]) is not int or not 32 < row["bytes"] <= 34 * 1024**2
                or not isinstance(row["sha256"], str)
                or not re.fullmatch("[0-9a-f]{64}", row["sha256"])
                or row["sha256"] == "0" * 64):
            raise ValueError("Invalid History panorama release")
    return catalog


_HISTORY_RELEASE = _load_history_release()
HISTORY_RESOURCE = _HISTORY_RELEASE["history"]["filename"]
HISTORY_BYTES = _HISTORY_RELEASE["history"]["bytes"]
HISTORY_SHA256 = _HISTORY_RELEASE["history"]["sha256"]
HISTORY_PANORAMA_PINS = {row["filename"]: (row["bytes"], row["sha256"])
                         for row in _HISTORY_RELEASE["panoramas"]}


CLASS_PICTURE_RESOURCE = "class-picture-ui.ownedz"
CLASS_PICTURE_BYTES = 5207398
CLASS_PICTURE_SHA256 = "c2c9cea9e69bd307cf4c76c5bbd87bc28e7fe6cac08162911b531dd44cea4747"

def _load_victory_release():
    path = Path(__file__).resolve().parents[1] / "src/hd/ClassicScreens/VictoryArtworkRelease.json"
    if path.stat().st_size > 4096:
        raise ValueError("Victory release catalog exceeds limit")
    value = json.loads(path.read_bytes())
    if (set(value) != {"schema", "filename", "fileBytes", "metadataSha256", "sha256", "imageCount"}
            or value["schema"] != 1 or value["filename"] != "victory-ui.ownedz"
            or value["imageCount"] != 32 or not 0 < value["fileBytes"] <= 512 * 1024 * 1024
            or any(not re.fullmatch(r"[0-9a-f]{64}", value[key]) or value[key] == "0" * 64
                   for key in ("metadataSha256", "sha256"))):
        raise ValueError("Invalid Victory release catalog")
    return value


_VICTORY_RELEASE = _load_victory_release()
VICTORY_RESOURCE = _VICTORY_RELEASE["filename"]
VICTORY_BYTES = _VICTORY_RELEASE["fileBytes"]
VICTORY_SHA256 = _VICTORY_RELEASE["sha256"]


def resource_pins():
    return {RESOURCE: (TEMPLATE_BYTES, TEMPLATE_SHA256),
            CLASS_PICTURE_RESOURCE: (CLASS_PICTURE_BYTES, CLASS_PICTURE_SHA256),
            HISTORY_RESOURCE: (HISTORY_BYTES, HISTORY_SHA256),
            VICTORY_RESOURCE: (VICTORY_BYTES, VICTORY_SHA256),
            **HISTORY_PANORAMA_PINS,
            INTRO_RESOURCE: (INTRO_BYTES, INTRO_SHA256),
            CREDITS_RESOURCE: (CREDITS_BYTES, CREDITS_SHA256),
            TRACK_INFO_RESOURCE: (TRACK_INFO_BYTES, TRACK_INFO_SHA256),
            CAR_INFO_RESOURCE: (CAR_INFO_BYTES, CAR_INFO_SHA256),
            PLAYER_PHOTO_RESOURCE: (PLAYER_PHOTO_BYTES, PLAYER_PHOTO_SHA256),
            LOCATION_PHOTO_RESOURCE: (LOCATION_PHOTO_BYTES, LOCATION_PHOTO_SHA256),
            TRACK_BADGE_RESOURCE: (TRACK_BADGE_BYTES, TRACK_BADGE_SHA256),
            WIDE_MENU_RESOURCE: (WIDE_MENU_BYTES, WIDE_MENU_SHA256),
            RESULTS_BACKDROP_RESOURCE: (RESULTS_BACKDROP_BYTES, RESULTS_BACKDROP_SHA256),
            SLIDESHOW_RESOURCE: (SLIDESHOW_BYTES, SLIDESHOW_SHA256),
            SLIDESHOW_FOOTER_RESOURCE: (SLIDESHOW_FOOTER_BYTES, SLIDESHOW_FOOTER_SHA256),
            **({SLIDESHOW_PANORAMA_RESOURCE: (_SLIDESHOW_PANORAMA_RELEASE['fileBytes'],
                                             _SLIDESHOW_PANORAMA_RELEASE['sha256'])}
               if _SLIDESHOW_PANORAMA_RELEASE else {})}
# These are local bounded-input policies, not Apple or SideStore limits.
# The complete authenticated artwork catalog alone is larger than 512 MiB.
MAX_BASE_IPA = 512 * 1024 * 1024
MAX_MEMBER_BYTES = 512 * 1024 * 1024
MAX_IPA = 768 * 1024 * 1024
MAX_UNPACKED = 1024 * 1024 * 1024
MAX_MEMBERS = 10000
CHUNK = 1024 * 1024


def require(value, message):
    if not value:
        raise ValueError(message)


class ReleasePinMatcher:
    """Match raw or hexadecimal loader pins, including chunk-boundary matches."""
    def __init__(self, pins, sizes=None, catalogs=None):
        self.patterns = {}
        for name, pin in pins.items():
            require(isinstance(pin, str) and re.fullmatch("[0-9a-fA-F]{64}", pin),
                    "Invalid required artwork release pin")
            self.patterns[name] = (bytes.fromhex(pin), pin.lower().encode("ascii"),
                                   pin.upper().encode("ascii"))
        for name, size in (sizes or {}).items():
            require(type(size) is int and 0 < size <= _panorama_module.MAX_BYTES,
                    "Invalid required artwork release size")
            pin = pins.get(name)
            require(pin is not None, "Required artwork size lacks its digest")
            self.patterns[name + ':bytes'] = (
                ('NFS-SLIDESHOW-PANORAMA:' + pin.lower() + ':' + str(size)).encode('ascii'),)
        for name, pattern in (catalogs or {}).items():
            require(isinstance(pattern, bytes) and 0 < len(pattern) <= 131072,
                    "Invalid required compiled catalog")
            require(name not in self.patterns, "Duplicate compiled catalog name")
            self.patterns[name] = (pattern,)
        self.overlap = max((len(p) for ps in self.patterns.values() for p in ps), default=1) - 1
        self.missing = set(self.patterns)
        self.tail = b""

    def feed(self, block):
        if not self.missing:
            return
        window = self.tail + block
        for name in tuple(self.missing):
            if any(pattern in window for pattern in self.patterns[name]):
                self.missing.remove(name)
        self.tail = window[-self.overlap:] if self.missing and self.overlap else b""


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
    require(allow_artwork or (folded[-1] not in resource_pins()
                             and folded[-1] != SLIDESHOW_PANORAMA_RESOURCE), "Artwork resource already present")
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
    device_platform = False
    for unused in range(commands):
        require(at + 8 <= len(table), "Mach-O command bounds")
        cmd, size = struct.unpack_from("<II", table, at)
        require(size >= 8 and size % 8 == 0 and size <= len(table) - at,
                "Mach-O command size")
        if main and cmd == 0x32:  # LC_BUILD_VERSION
            require(size >= 24 and not device_platform, "Invalid Mach-O device platform command")
            platform, _, _, tools = struct.unpack_from("<IIII", table, at + 8)
            require(platform == 2 and tools <= (size - 24) // 8, "Expected iOS device platform")
            device_platform = True
        if cmd == 0x19:  # LC_SEGMENT_64
            require(size >= 72, "Truncated Mach-O segment command")
            initial_protection = struct.unpack_from("<I", table, at + 60)[0]
            require(initial_protection & 0x06 != 0x06,
                    "Mach-O segment starts writable and executable")
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
    require(not main or device_platform, "Missing iOS device platform command")
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


def inspect(path, allow_resource=False, required_artwork_sha256=None, required_release_pins=None, required_release_sizes=None, required_catalogs=None):
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
        artwork_pins = resource_pins()
        artwork_digests = {}
        for info in infos:
            pin = artwork_pins.get(info.filename.rsplit("/", 1)[-1]) if allow_resource else None
            if pin is not None:
                # Each new entry must be a pinned resource at the app root.
                require(info.filename.count("/") == 2 and info.filename.startswith("Payload/"),
                        "Unexpected artwork entry location")
                name, parts = safe_name(info, allow_artwork=True)
                require(parts[1].endswith(".app") and not info.is_dir()
                        and info.file_size == pin[0]
                        and info.compress_type == zipfile.ZIP_STORED
                        and not info.flag_bits & 1, "Invalid injected artwork entry")
                artwork_digests[name] = pin[1]
            else:
                name, parts = safe_name(info)
            key = name.rstrip("/").casefold()
            require(key not in seen, "Duplicate or case-ambiguous ZIP member")
            seen.add(key)
            if len(parts) >= 2:
                require(parts[1].endswith(".app"), "Unexpected Payload member")
                app_roots.add("Payload/" + parts[1])
            total += info.file_size
            require(info.file_size <= MAX_MEMBER_BYTES and total <= MAX_UNPACKED,
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
                    release = dict(required_release_pins or {}) if name == main_name else {}
                    if required_artwork_sha256 and name == main_name:
                        release[RESOURCE] = required_artwork_sha256
                    matcher = ReleasePinMatcher(release, required_release_sizes if name == main_name else None,
                                                required_catalogs if name == main_name else None)
                    matcher.feed(first)
                    while True:
                        b = f.read(CHUNK)
                        if not b:
                            break
                        count += len(b)
                        require(count <= info.file_size, "ZIP member grew")
                        h.update(b)
                        matcher.feed(b)
                    require(count == info.file_size, "ZIP member length mismatch")
                    require(not matcher.missing, "Base executable lacks the required artwork release pin")
                    digest = h.hexdigest()
                    require(name not in artwork_digests or digest == artwork_digests[name],
                            "Injected artwork digest does not match its pin")
                    result[name] = digest
                    if macho:
                        machos[name] = h.hexdigest()
            storage[name] = raw_storage(path, info)
        require(main_name in machos, "Main executable was not validated")
        ranges = sorted((offset, offset + count) for offset, count, unused in storage.values())
        require(all(ranges[i-1][1] <= ranges[i][0] for i in range(1, len(ranges))),
                "Overlapping ZIP member storage")
        return dict(app=app, members=result, storage=storage, machos=machos,
                    unpackedBytes=total, comment=z.comment)


def inject(base_ipa, base_sha256, template, output, intro=None, track_info=None, car_info=None, player_photo=None, track_badge=None, wide_menu=None, results_backdrop=None, slideshow=None, slideshow_footer=None, history=None, class_picture=None, location_photo=None, history_panoramas_dir=None, credits=None, victory=None, slideshow_panorama=None):
    require(slideshow_panorama is None or (_SLIDESHOW_PANORAMA_RELEASE is not None and slideshow is not None and slideshow_footer is not None),
            "Slideshow panorama requires its approved pin and matching photo/footer pair")
    require((history is None) == (history_panoramas_dir is None),
            "History backgrounds and all seven panoramas must be supplied together")
    require(re.fullmatch("[0-9a-f]{64}", base_sha256 or ""), "Provide the trusted base IPA SHA-256")
    base = Path(base_ipa).resolve(strict=True)
    source = Path(template).resolve(strict=True)
    target_raw = Path(output).absolute()
    parent = target_raw.parent.resolve(strict=True)
    target = parent / target_raw.name
    require(base.is_file() and source.is_file() and parent.is_dir(), "Expected regular input files/output folder")
    require(not target.exists() and not target.is_symlink() and target not in (base, source),
            "Output must be a new file")
    require(0 < base.stat().st_size <= min(MAX_BASE_IPA, MAX_IPA - TEMPLATE_BYTES - 65536), "Base IPA size limit")
    require(source.stat().st_size == TEMPLATE_BYTES, "Wrong personal artwork size")
    with source.open("rb") as f:
        artwork = f.read(TEMPLATE_BYTES + 1)
    require(len(artwork) == TEMPLATE_BYTES and hashlib.sha256(artwork).hexdigest() == TEMPLATE_SHA256,
            "Wrong or mutated personal artwork template")
    require((slideshow is None) == (slideshow_footer is None),
            "Slideshow photos and overlay resources must be supplied together")
    extra_inputs = {}
    artwork_inputs = {RESOURCE: artwork}
    if intro is not None:
        intro_source = Path(intro).resolve(strict=True)
        require(intro_source.is_file() and intro_source != target, "Expected regular intro artwork input")
        require(intro_source.stat().st_size == INTRO_BYTES, "Wrong intro artwork size")
        with intro_source.open("rb") as f:
            intro_artwork = f.read(INTRO_BYTES + 1)
        require(len(intro_artwork) == INTRO_BYTES
                and hashlib.sha256(intro_artwork).hexdigest() == INTRO_SHA256,
                "Wrong or mutated intro artwork")
        extra_inputs[intro_source] = INTRO_SHA256
        artwork_inputs[INTRO_RESOURCE] = intro_artwork
    if credits is not None:
        credits_source = Path(credits).resolve(strict=True)
        require(credits_source.is_file() and credits_source != target, "Expected regular credits artwork input")
        require(credits_source.stat().st_size == CREDITS_BYTES, "Wrong credits artwork size")
        with credits_source.open("rb") as f:
            credits_artwork = f.read(CREDITS_BYTES + 1)
        require(len(credits_artwork) == CREDITS_BYTES
                and hashlib.sha256(credits_artwork).hexdigest() == CREDITS_SHA256,
                "Wrong or mutated credits artwork")
        extra_inputs[credits_source] = CREDITS_SHA256
        artwork_inputs[CREDITS_RESOURCE] = credits_artwork
    if track_info is not None:
        track_source = Path(track_info).resolve(strict=True)
        require(track_source.is_file() and track_source != target, "Expected regular track-info artwork input")
        require(track_source.stat().st_size == TRACK_INFO_BYTES, "Wrong track-info artwork size")
        with track_source.open("rb") as f:
            track_artwork = f.read(TRACK_INFO_BYTES + 1)
        require(len(track_artwork) == TRACK_INFO_BYTES
                and hashlib.sha256(track_artwork).hexdigest() == TRACK_INFO_SHA256,
                "Wrong or mutated track-info artwork")
        extra_inputs[track_source] = TRACK_INFO_SHA256
        artwork_inputs[TRACK_INFO_RESOURCE] = track_artwork
    if car_info is not None:
        car_source = Path(car_info).resolve(strict=True)
        require(car_source.is_file() and car_source != target, "Expected regular car-info artwork input")
        require(car_source.stat().st_size == CAR_INFO_BYTES, "Wrong car-info artwork size")
        with car_source.open("rb") as f:
            car_artwork = f.read(CAR_INFO_BYTES + 1)
        require(len(car_artwork) == CAR_INFO_BYTES
                and hashlib.sha256(car_artwork).hexdigest() == CAR_INFO_SHA256,
                "Wrong or mutated car-info artwork")
        extra_inputs[car_source] = CAR_INFO_SHA256
        artwork_inputs[CAR_INFO_RESOURCE] = car_artwork
    if player_photo is not None:
        player_source = Path(player_photo).resolve(strict=True)
        require(player_source.is_file() and player_source != target, "Expected regular player-photo artwork input")
        require(player_source.stat().st_size == PLAYER_PHOTO_BYTES, "Wrong player-photo artwork size")
        with player_source.open("rb") as f:
            player_artwork = f.read(PLAYER_PHOTO_BYTES + 1)
        require(len(player_artwork) == PLAYER_PHOTO_BYTES
                and hashlib.sha256(player_artwork).hexdigest() == PLAYER_PHOTO_SHA256,
                "Wrong or mutated player-photo artwork")
        extra_inputs[player_source] = PLAYER_PHOTO_SHA256
        artwork_inputs[PLAYER_PHOTO_RESOURCE] = player_artwork
    if location_photo is not None:
        location_source = Path(location_photo).resolve(strict=True)
        require(location_source.is_file() and location_source != target, "Expected regular location-photo artwork input")
        require(location_source.stat().st_size == LOCATION_PHOTO_BYTES, "Wrong location-photo artwork size")
        with location_source.open("rb") as f:
            location_artwork = f.read(LOCATION_PHOTO_BYTES + 1)
        require(len(location_artwork) == LOCATION_PHOTO_BYTES
                and hashlib.sha256(location_artwork).hexdigest() == LOCATION_PHOTO_SHA256,
                "Wrong or mutated location-photo artwork")
        extra_inputs[location_source] = LOCATION_PHOTO_SHA256
        artwork_inputs[LOCATION_PHOTO_RESOURCE] = location_artwork
    if track_badge is not None:
        badge_source = Path(track_badge).resolve(strict=True)
        require(badge_source.is_file() and badge_source != target, "Expected regular track-badge artwork input")
        require(badge_source.stat().st_size == TRACK_BADGE_BYTES, "Wrong track-badge artwork size")
        with badge_source.open("rb") as f:
            badge_artwork = f.read(TRACK_BADGE_BYTES + 1)
        require(len(badge_artwork) == TRACK_BADGE_BYTES
                and hashlib.sha256(badge_artwork).hexdigest() == TRACK_BADGE_SHA256,
                "Wrong or mutated track-badge artwork")
        extra_inputs[badge_source] = TRACK_BADGE_SHA256
        artwork_inputs[TRACK_BADGE_RESOURCE] = badge_artwork
    for value, resource in ((wide_menu, WIDE_MENU_RESOURCE),
                            (results_backdrop, RESULTS_BACKDROP_RESOURCE),
                            (slideshow, SLIDESHOW_RESOURCE),
                            (slideshow_footer, SLIDESHOW_FOOTER_RESOURCE),
                            (slideshow_panorama, SLIDESHOW_PANORAMA_RESOURCE),
                            (history, HISTORY_RESOURCE),
                            (victory, VICTORY_RESOURCE),
                            (class_picture, CLASS_PICTURE_RESOURCE)):
        if value is None:
            continue
        source_path = Path(value).resolve(strict=True)
        size, expected = resource_pins()[resource]
        require(source_path.is_file() and source_path != target, f"Expected regular input for {resource}")
        require(source_path.stat().st_size == size, f"Wrong size for {resource}")
        with source_path.open("rb") as f:
            artwork_bytes = f.read(size + 1)
        require(len(artwork_bytes) == size and hashlib.sha256(artwork_bytes).hexdigest() == expected,
                f"Wrong or mutated {resource}")
        extra_inputs[source_path] = expected
        artwork_inputs[resource] = artwork_bytes
    if history_panoramas_dir is not None:
        panorama_dir = Path(history_panoramas_dir).resolve(strict=True)
        require(panorama_dir.is_dir(), "Expected History panorama folder")
        for name, (size, expected) in HISTORY_PANORAMA_PINS.items():
            path = (panorama_dir / name).resolve(strict=True)
            require(path.is_file() and path != target, "Expected regular History panorama")
            require(path.stat().st_size == size, "Wrong History panorama size")
            with path.open("rb") as stream:
                data = stream.read(size + 1)
            require(len(data) == size and hashlib.sha256(data).hexdigest() == expected,
                    "Wrong or mutated History panorama")
            extra_inputs[path] = expected
            artwork_inputs[name] = data
    added_bytes = sum(len(value) for value in artwork_inputs.values())
    require(base.stat().st_size <= MAX_IPA - added_bytes - 65536, "Combined IPA size limit")
    require(digest_file(base) == base_sha256, "Base IPA identity mismatch")
    with base.open("rb") as f:
        require(f.read(4) == b"PK\x03\x04", "Expected a plain IPA ZIP archive")
    release_pins = ({HISTORY_RESOURCE: HISTORY_SHA256,
                     **{name: pin[1] for name, pin in HISTORY_PANORAMA_PINS.items()}}
                    if history is not None else {})
    if victory is not None:
        release_pins[VICTORY_RESOURCE] = _VICTORY_RELEASE["metadataSha256"]
    release_sizes = {}
    catalogs = {}
    if slideshow_panorama is not None:
        for name in (SLIDESHOW_PANORAMA_RESOURCE,):
            release_pins[name] = resource_pins()[name][1]
        catalogs = compiled_catalogs(Path(__file__).resolve().parents[1])
        release_sizes[SLIDESHOW_PANORAMA_RESOURCE] = _SLIDESHOW_PANORAMA_RELEASE['fileBytes']
    before = inspect(base, required_artwork_sha256=OUTPUT_PACK_SHA256,
                     required_release_pins=release_pins, required_release_sizes=release_sizes,
                     required_catalogs=catalogs)
    require(before["unpackedBytes"] + added_bytes <= MAX_UNPACKED,
            "Combined unpacked size limit")
    resource = before["app"] + "/" + RESOURCE
    additions = {before["app"] + "/" + name: data for name, data in artwork_inputs.items()}
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
                require(copied <= MAX_IPA - added_bytes - 65536, "Base grew while copying")
                h.update(b)
                out.write(b)
            out.flush()
            os.fsync(out.fileno())
        require(h.hexdigest() == base_sha256, "Base IPA changed while copying")
        # Append preserves every original local header and compressed member byte.
        with zipfile.ZipFile(temporary, "a", allowZip64=True) as z:
            for entry, data in additions.items():
                member = zipfile.ZipInfo(entry, (1980, 1, 1, 0, 0, 0))
                member.create_system = 3
                member.external_attr = (stat.S_IFREG | 0o644) << 16
                member.compress_type = zipfile.ZIP_STORED
                z.writestr(member, data)
        require(temporary.stat().st_size <= MAX_IPA, "Finished IPA size limit")
        after = inspect(temporary, allow_resource=True, required_artwork_sha256=OUTPUT_PACK_SHA256,
                        required_release_pins=release_pins,
                        required_release_sizes=release_sizes, required_catalogs=catalogs)
        require(after["app"] == before["app"] and after["comment"] == before["comment"],
                "App identity or ZIP comment changed")
        require(set(after["members"]) == set(before["members"]) | set(additions),
                "Unexpected added/removed IPA member")
        require(after["members"][resource] == TEMPLATE_SHA256,
                "Injected artwork identity mismatch")
        require(all(after["members"][entry] == hashlib.sha256(data).hexdigest()
                    for entry, data in additions.items()), "Injected resource identity mismatch")
        require(all(digest_file(path) == expected for path, expected in extra_inputs.items()),
                "An optional artwork input changed")
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
                      resourceEntry=resource, addedMembers=len(additions),
                      existingMembersPreserved=len(before["members"]),
                      allExistingMemberBytesIdentical=True,
                      allExistingCompressedStorageIdentical=True,
                      machOHashes=after["machos"], signed=False,
                      originalInputsUnchanged=True,
                      historyReleasePinsVerified=history is not None)
        result['slideshowPanoramaReleasePinsVerified'] = slideshow_panorama is not None
        result['slideshowCompiledCatalogsVerified'] = slideshow_panorama is not None
        if slideshow_panorama is not None:
            result['compiledCatalogs'] = {
                name: dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
                for name, data in catalogs.items()}
        if credits is not None or intro is not None or track_info is not None or car_info is not None or player_photo is not None or track_badge is not None or wide_menu is not None or results_backdrop is not None or slideshow is not None or history is not None or class_picture is not None or location_photo is not None or victory is not None:
            result["additionalResources"] = {
                name: dict(sha256=hashlib.sha256(data).hexdigest(), bytes=len(data))
                for name, data in artwork_inputs.items() if name != RESOURCE}
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
    p.add_argument("--credits", type=Path, help="Optional pinned eleven-layer Credits artwork")
    p.add_argument("--intro", type=Path, help="Optional separately pinned splash artwork")
    p.add_argument("--track-info", type=Path, help="Optional separately pinned track-info artwork")
    p.add_argument("--car-info", type=Path, help="Optional separately pinned car-info artwork")
    p.add_argument("--player-photo", type=Path, help="Optional separately pinned Player-menu photos")
    p.add_argument("--track-badge", type=Path, help="Optional separately pinned animated track badges")
    p.add_argument("--wide-menu", type=Path, help="Optional separately pinned widescreen map and panel")
    p.add_argument("--results-backdrop", type=Path, help="Optional separately pinned Results backdrops")
    p.add_argument("--slideshow", type=Path, help="Pinned lossless slideshow photos; supply with --slideshow-footer")
    p.add_argument("--slideshow-footer", type=Path, help="Pinned matching slideshow overlay resource")
    p.add_argument("--slideshow-panorama", type=Path, help="Optional pinned side strips; requires matching slideshow photo/footer pair and compiled pin")
    p.add_argument("--history", type=Path, help="Pinned manufacturer-history photographs and localized headings")
    p.add_argument("--class-picture", type=Path, help="Optional separately pinned Opponents class A/B/C collages")
    p.add_argument("--location-photo", type=Path, help="Separately pinned eight Location photos; use a base with required Location support")
    p.add_argument("--history-panoramas-dir", type=Path,
                   help="Folder with all seven pinned panoramas; required with --history")
    p.add_argument("--victory", type=Path, help="Pinned adaptive victory artwork")
    args = p.parse_args()
    print(json.dumps(inject(args.base_ipa, args.base_sha256, args.template, args.output, args.intro, args.track_info, args.car_info, args.player_photo, args.track_badge, args.wide_menu, args.results_backdrop, args.slideshow, args.slideshow_footer, args.history, args.class_picture, args.location_photo, args.history_panoramas_dir, args.credits, args.victory, args.slideshow_panorama), indent=2))


if __name__ == "__main__":
    main()
