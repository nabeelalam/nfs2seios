"""Allowlist and inspect an unsigned game IPA before creating a public download."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import plistlib
import re
import stat
import struct
import zipfile
from publication_audit import PATTERNS

PREFIX = "Payload/NFSIISEGame.app/"
LICENSES = {"GameSupportLicense.txt", "SDLLicense.txt"}
SUPPORT = {"install.win", "text.eng", "text.ger", "text.fre", "text.ita", "text.spa", "text.swe"} | LICENSES
FILES = SUPPORT | {"Info.plist", "PkgInfo", "NFSIISEGame"}
MANIFEST_KEYS = {"executable", "bundle_id", "architecture", "platform", "support_sha256", "source_commit", "ipa_sha256", "configuration", "personal_signing_required", "installation_verified", "gameplay_verified"}
LIMIT = 64 * 1024 * 1024

def inspect(ipa, manifest, source, assets_separate=False):
    if ipa.is_symlink() or manifest.is_symlink():
        raise ValueError("Package symlinks refused")
    if not re.fullmatch(r"[0-9a-f]{40}", source):
        raise ValueError("Invalid source identity")
    if ipa.stat().st_size > LIMIT or manifest.stat().st_size > 16384:
        raise ValueError("Package exceeds delivery budget")
    metadata = json.loads(manifest.read_text(encoding="utf-8"))
    if set(metadata) != MANIFEST_KEYS:
        raise ValueError("Unexpected manifest fields")
    expected = {"executable": "NFSIISEGame", "bundle_id": "io.github.nabeelalam.nfsiise.game", "architecture": "arm64", "platform": "iOS device", "source_commit": source, "configuration": "Release", "personal_signing_required": True, "installation_verified": False, "gameplay_verified": False}
    if any(metadata[key] != value for key, value in expected.items()):
        raise ValueError("Package identity or acceptance claims differ")
    if metadata["ipa_sha256"] != hashlib.sha256(ipa.read_bytes()).hexdigest():
        raise ValueError("IPA digest differs")
    required_support = LICENSES if assets_separate else SUPPORT
    if set(metadata["support_sha256"]) != required_support:
        raise ValueError("Unexpected support manifest")
    with zipfile.ZipFile(ipa) as archive:
        entries = archive.infolist()
        required_files = required_support | {"Info.plist", "PkgInfo", "NFSIISEGame"}
        if len(entries) != len(required_files) or {entry.filename for entry in entries} != {PREFIX + name for name in required_files}:
            raise ValueError("Unexpected IPA contents")
        if sum(entry.file_size for entry in entries) > LIMIT:
            raise ValueError("Unpacked IPA exceeds delivery budget")
        for entry in entries:
            if stat.S_ISLNK(entry.external_attr >> 16) or entry.flag_bits & 1:
                raise ValueError("Symlink or encrypted entry refused")
            data = archive.read(entry)
            for category, pattern in PATTERNS.items():
                if pattern.search(data):
                    raise ValueError("IPA privacy check failed: " + category)
            name = entry.filename[len(PREFIX):]
            if name in SUPPORT and hashlib.sha256(data).hexdigest() != metadata["support_sha256"][name]:
                raise ValueError("Support digest differs")
        info = plistlib.loads(archive.read(PREFIX + "Info.plist"))
        if info.get("CFBundleIdentifier") != expected["bundle_id"] or info.get("CFBundleExecutable") != expected["executable"] or info.get("UIDeviceFamily") != [2]:
            raise ValueError("Unexpected bundle identity")
        binary = archive.read(PREFIX + "NFSIISEGame")
        if len(binary) < 32:
            raise ValueError("Missing Mach-O header")
        magic, cpu, _, _, count, size, _, _ = struct.unpack_from("<8I", binary)
        if magic != 0xfeedfacf or cpu != 0x100000c or size > len(binary) - 32:
            raise ValueError("Expected thin ARM64 Mach-O")
        offset = 32
        device = False
        for _ in range(count):
            if offset + 8 > 32 + size:
                raise ValueError("Truncated Mach-O commands")
            command, length = struct.unpack_from("<2I", binary, offset)
            if length < 8 or offset + length > 32 + size:
                raise ValueError("Invalid Mach-O command")
            if command == 0x32 and length >= 24:
                device = struct.unpack_from("<I", binary, offset + 8)[0] == 2
            offset += length
        if not device or offset != 32 + size:
            raise ValueError("Expected iOS device platform")
    if any(pattern.search(manifest.read_bytes()) for pattern in PATTERNS.values()):
        raise ValueError("Manifest privacy check failed")
    return metadata

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ipa", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--source", required=True)
    args = parser.parse_args()
    metadata = inspect(args.ipa, args.manifest, args.source)
    if args.output.exists():
        raise ValueError("Existing release download will not be overwritten")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    # Public delivery contains the executable and licenses only. Configuration
    # and language files stay in the user's separately imported game storage.
    payload = io.BytesIO()
    with zipfile.ZipFile(args.ipa) as original, zipfile.ZipFile(payload, "w", zipfile.ZIP_DEFLATED) as stripped:
        for entry in original.infolist():
            if entry.filename[len(PREFIX):] not in SUPPORT - LICENSES:
                stripped.writestr(entry, original.read(entry))
    metadata["support_sha256"] = {name: metadata["support_sha256"][name] for name in LICENSES}
    metadata["ipa_sha256"] = hashlib.sha256(payload.getvalue()).hexdigest()
    import tempfile
    with tempfile.TemporaryDirectory(prefix="nfs-release-", dir=args.output.parent) as folder:
        ipa = Path(folder) / "game.ipa"
        manifest = Path(folder) / "game.json"
        ipa.write_bytes(payload.getvalue())
        manifest.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
        inspect(ipa, manifest, args.source, assets_separate=True)
        with zipfile.ZipFile(args.output, "x", zipfile.ZIP_DEFLATED) as archive:
            archive.write(ipa, "NFSIISEGame-development-unsigned.ipa")
            archive.write(manifest, "NFSIISEGame-development-unsigned.json")
    print("Release download inspected: executable and licenses only; configuration, language and original assets excluded")

if __name__ == "__main__":
    main()
