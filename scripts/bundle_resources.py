"""Verify required game support files; this does not verify a compiled binary."""
import hashlib

from publication_audit import PATTERNS
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HUD_NOTICE_SOURCES = {
    "BebasNeue-OFL.txt": ROOT / "assets/hud/fonts/BebasNeue-OFL.txt",
    "Michroma-OFL.txt": ROOT / "assets/hud/fonts/Michroma-OFL.txt",
    "Oxanium-OFL.txt": ROOT / "assets/hud/fonts/Oxanium-OFL.txt",
    "HUDArtworkNotice.md": ROOT / "assets/hud/HUDArtworkNotice.md",
}

CLASSIC_NOTICE_SOURCES = {
    "Fonts/LeagueGothic-OFL.txt": ROOT / "assets/licenses/classic-ui/LeagueGothic-OFL.txt",
    "Fonts/Michroma-OFL.txt": ROOT / "assets/licenses/classic-ui/Michroma-OFL.txt",
    "Fonts/NOTICE.txt": ROOT / "assets/licenses/classic-ui/NOTICE.txt",
}

def validate_game_resources(app: Path, classic_ui: bool = False) -> dict[str, str]:
    support = ROOT / "candidates/link4/Need For Speed II SE"
    sources = {name: support / name for name in (
        "install.win", "text.eng", "text.ger", "text.fre", "text.ita", "text.spa", "text.swe"
    )}
    sources["GameSupportLicense.txt"] = ROOT / "candidates/link4/LICENSE"
    sources["SDLLicense.txt"] = ROOT / ".build/vendor/SDL/LICENSE.txt"
    sources.update(HUD_NOTICE_SOURCES)
    if classic_ui: sources.update(CLASSIC_NOTICE_SOURCES)
    hashes = {}
    for name, source in sources.items():
        target = app / name
        if target.is_symlink() or not target.is_file():
            raise RuntimeError(f"Required game bundle resource missing: {name}")
        expected = hashlib.sha256(source.read_bytes()).hexdigest()
        actual = hashlib.sha256(target.read_bytes()).hexdigest()
        if actual != expected:
            raise RuntimeError(f"Game bundle resource differs from pinned support: {name}")
        hashes[name] = actual
    for path in app.rglob("*"):
        if path.is_symlink():
            raise RuntimeError("Unexpected symbolic link in static game bundle")
        if path.name.lower() in {"gamedata", "fedata"} or path.suffix.lower() in {
            ".exe", ".viv", ".fsh", ".qfs", ".dct"
        }:
            raise RuntimeError("Original game data must be supplied separately")
        if path.suffix.lower() == ".ownedz":
            raise RuntimeError("Personal artwork packs must be supplied separately")
    return hashes

def validate_bundle_payloads(app: Path) -> None:
    """Fail before packaging if a member embeds private paths or credentials."""
    for path in sorted(app.rglob("*")):
        if path.is_symlink():
            raise RuntimeError("Unexpected symbolic link in static app bundle")
        if not path.is_file():
            continue
        data = path.read_bytes()
        for category, pattern in PATTERNS.items():
            if pattern.search(data):
                raise RuntimeError(f"App bundle contains a prohibited {category} pattern")
