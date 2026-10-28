"""Validate a device bundle and package it, without asserting signing/installability."""
import plistlib
from pathlib import Path
import subprocess
import sys
import zipfile
import json
import hashlib
import os
import re
from bundle_resources import ROOT, validate_game_resources

app, output = map(Path, sys.argv[1:])
classic_ui = os.environ.get("NFS_CLASSIC_UI", "OFF") == "ON"
classic_fingerprint = os.environ.get("NFS_CLASSIC_ASSET_MANIFEST_SHA256", "")
if classic_ui and os.environ.get("NFS_PUBLISH_RELEASE_REQUESTED", "false") == "true":
    raise RuntimeError("Classic base package requires personal artwork; public release is prohibited")
if classic_ui and re.fullmatch(r"[0-9a-f]{64}", classic_fingerprint) is None:
    raise RuntimeError("Classic package requires its approved artwork manifest identity")
with (app / "Info.plist").open("rb") as stream:
    info = plistlib.load(stream)
binary = app / info["CFBundleExecutable"]
architectures = subprocess.check_output(["lipo", "-archs", str(binary)], text=True).strip()
if architectures != "arm64":
    raise RuntimeError(f"Expected device ARM64, found {architectures}")
load = subprocess.check_output(["otool", "-l", str(binary)], text=True)
if "platform 2" not in load and "platform IOS" not in load:
    raise RuntimeError("Mach-O does not declare the iOS device platform")
if info.get("UIDeviceFamily") != [2]:
    raise RuntimeError("Expected an iPad-only bundle")
resources = {}
if info["CFBundleExecutable"] == "NFSIISEGame":
    if info.get("CFBundleIdentifier") != "io.github.nabeelalam.nfsiise.game":
        raise RuntimeError("Unexpected original-engine game bundle identity")
    if not info.get("UIFileSharingEnabled") or not info.get("LSSupportsOpeningDocumentsInPlace"):
        raise RuntimeError("Game bundle must expose import storage and diagnostic log in Files")
    resources = validate_game_resources(app, classic_ui=classic_ui)
output.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
    for path in sorted(app.rglob("*")):
        if path.is_file():
            name = "Payload/" + app.name + "/" + path.relative_to(app).as_posix()
            entry = zipfile.ZipInfo.from_file(path, name)
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, path.read_bytes())
print(f"Packaged {output}: ARM64 iPad device bundle; UNSIGNED, installation unverified")
if classic_ui: print("CLASSIC PUBLIC BASE: personal artwork must be imported; this is not a finished public release")
output.with_suffix(".json").write_text(json.dumps({
    "executable": info["CFBundleExecutable"], "bundle_id": info["CFBundleIdentifier"],
    "architecture": architectures, "platform": "iOS device", "support_sha256": resources,
    "source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
    "ipa_sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
    "configuration": os.environ.get("NFS_BUILD_CONFIGURATION", "unspecified"),
    **({"classic_ui": True, "classic_asset_manifest_sha256": classic_fingerprint, "personal_artwork_required": True, "public_release_ready": False, "package_kind": "classic-public-base-needs-personal-artwork"} if classic_ui else {}),
    "personal_signing_required": True, "installation_verified": False, "gameplay_verified": False,
}, indent=2)+"\n", encoding="utf-8")
