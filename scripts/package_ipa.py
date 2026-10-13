"""Validate a device bundle and package it, without asserting signing/installability."""
import plistlib
from pathlib import Path
import subprocess
import sys
import zipfile
import json
import hashlib
import os
from bundle_resources import ROOT, validate_game_resources

app, output = map(Path, sys.argv[1:])
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
    resources = validate_game_resources(app)
output.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
    for path in sorted(app.rglob("*")):
        if path.is_file():
            name = "Payload/" + app.name + "/" + path.relative_to(app).as_posix()
            entry = zipfile.ZipInfo.from_file(path, name)
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, path.read_bytes())
print(f"Packaged {output}: ARM64 iPad device bundle; UNSIGNED, installation unverified")
output.with_suffix(".json").write_text(json.dumps({
    "executable": info["CFBundleExecutable"], "bundle_id": info["CFBundleIdentifier"],
    "architecture": architectures, "platform": "iOS device", "support_sha256": resources,
    "source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
    "ipa_sha256": hashlib.sha256(output.read_bytes()).hexdigest(),
    "configuration": os.environ.get("NFS_BUILD_CONFIGURATION", "unspecified"),
    "personal_signing_required": True, "installation_verified": False, "gameplay_verified": False,
}, indent=2)+"\n", encoding="utf-8")
