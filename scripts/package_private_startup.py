"""Prepare separately supplied startup files locally; never upload this package."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import zipfile
from prepare_engine import ROOT, PIN, URL

NAMES = ("install.win", "text.eng", "text.ger", "text.fre", "text.ita", "text.spa", "text.swe")

def prepare(output):
    output = output.resolve()
    if not output.is_relative_to((ROOT / ".build").resolve()):
        raise ValueError("Private startup output must remain in ignored .build storage")
    if output.exists():
        raise ValueError("Existing private startup output will not be overwritten")
    source = ROOT / "candidates/link4"
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip()
    if revision != PIN:
        raise ValueError("Startup source differs from the reviewed pinned revision")
    contents = {}
    for name in NAMES:
        relative = "Need For Speed II SE/" + name
        path = source / relative
        if path.is_symlink() or not path.is_file() or path.stat().st_size > 256 * 1024:
            raise ValueError("Invalid startup input")
        data = path.read_bytes()
        pinned = subprocess.check_output(["git", "show", PIN + ":" + relative], cwd=source)
        if data != pinned:
            raise ValueError("Startup input has unreviewed local changes")
        contents[name] = data
    license_path = source / "LICENSE"
    license_data = subprocess.check_output(["git", "show", PIN + ":LICENSE"], cwd=source)
    if license_path.is_symlink() or license_path.read_bytes() != license_data:
        raise ValueError("Startup license differs from the reviewed source")
    contents["GameSupportLicense.txt"] = license_data
    contents["README.txt"] = (
        "PRIVATE LOCAL STARTUP FILES - DO NOT UPLOAD TO PUBLIC GIT OR RELEASES\n\n"
        "For a future executable-only build with the separately supplied startup\n"
        "importer, copy install.win and text.* into your own selected game folder\n"
        "alongside GAMEDATA and FEDATA. Keep the license notice. The Windows EXE\n"
        "is not needed. No GAMEDATA/FEDATA, application, signing or pairing data\n"
        "is in this package. Leave your original source folder unchanged; prepare\n"
        "a separate transfer copy.\n\n"
        "Build3 updates an existing imported app; this package does not add fresh\n"
        "import support to build3. Do not delete the installed app or its data.\n"
    ).encode()
    metadata = {"scope": "Private local separate startup transfer; not a public artifact", "source": URL,
        "revision": PIN, "files": {name: {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()} for name, data in contents.items()}}
    output.mkdir(parents=True)
    for name, data in contents.items():
        (output / name).write_bytes(data)
    (output / "manifest.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    package = output / "NFSIISE-startup-files-private.zip"
    with zipfile.ZipFile(package, "x", zipfile.ZIP_DEFLATED) as archive:
        for name, data in contents.items():
            archive.writestr(name, data)
        archive.write(output / "manifest.json", "manifest.json")
    print("Private startup package prepared locally; no main game trees or upload")
    return package

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / ".build/private-ipad-startup")
    prepare(parser.parse_args().output)

if __name__ == "__main__":
    main()
