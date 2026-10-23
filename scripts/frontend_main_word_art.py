"""Pinned complete-word Main graphics; no fonts or original assets are loaded."""
# SPDX-License-Identifier: MIT
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ART_PATH = ROOT / "assets/hud/frontend/main-word-art.json"
SOURCE_SHA = "91cd862d1be9e0e8b4193f0b6b4c0f52f977f8575ae54836e850562f7b80bd56"
SOURCE_BYTES = 405540
ATLAS_WIDTH, ATLAS_HEIGHT = 1024, 2048
WORDS = (
    ("game-setup.svg", "game setup", 380, 119, "74e68993aed31e26135941baeaf019a5f07c51783c8a72a0d4f756bf9a8be0ff"),
    ("location.svg", "location", 264, 99, "2fdcc0756147a41e6633da746053cec35170ae202768357d5908ab7928487230"),
    ("player-one.svg", "player one", 340, 119, "cbb2cf481baa8587780a4b5574a53ea85686f768ecd5428af666f6e2d6b451d6"),
    ("player-two.svg", "player two", 344, 119, "11a8c6eb61d4eb378d4bcdedbbac75ea6901c12d20277826a866037a309cb649"),
    ("opponents.svg", "opponents", 348, 119, "b4a31a0336d06fa02b49956af8b9415b127dc0baa0993e45819b9c902fb7b02b"),
    ("race.svg", "RACE", 300, 177, "19a8dfc5e6164b3e17213d4b07976f4a40cc95ecce538d86b25ab02b9e90ee13"),
    ("options.svg", "options", 248, 119, "59915bdcb4ba74d611701174034b6abe51e9013b5166873fbfd0ec4543fae3eb"),
    ("exit.svg", "exit", 128, 98, "0e27ee065143fe5fe3864f390203f09cad5a3c68190746d5c58067bfcc195a84"),
)

def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate Main word-art JSON key")
        result[key] = value
    return result

def _integer(value, low, high):
    return type(value) is int and low <= value <= high

def _identities():
    return [dict(file=file, text=text, width=width, height=height, rgbaSha256=sha)
            for file, text, width, height, sha in WORDS]

def _decode(document):
    keys = {"schema", "scope", "typeface", "revision", "licenseReference", "words"}
    if type(document) is not dict or set(document) != keys or type(document["schema"]) is not int or document["schema"] != 1:
        raise ValueError("Main word-art schema changed")
    if document["scope"] != "Eight complete-word graphics; no font file, alphabet or reusable glyph font." or document["typeface"] != "Haettenschweiler":
        raise ValueError("Main word-art scope/typeface changed")
    revision = document["revision"]
    if revision != {"widthIncreaseOriginalUiPixels": 2, "strokeInsetOriginalUiPixels": 0.25, "rightEdgeAndBaselinePreserved": True} or type(revision.get("rightEdgeAndBaselinePreserved")) is not bool:
        raise ValueError("Main word-art revision changed")
    if document["licenseReference"] != "https://learn.microsoft.com/en-us/typography/fonts/font-faq#redistribution-and-extended-rights":
        raise ValueError("Main word-art reference changed")
    words = document["words"]
    if type(words) is not list or len(words) != len(WORDS):
        raise ValueError("Main word-art inventory changed")
    decoded = []
    for word, expected in zip(words, WORDS):
        if type(word) is not dict or set(word) != {"file", "text", "width", "height", "rgbaSha256", "alphaRuns"}:
            raise ValueError("Main word-art fields changed")
        file, text, width, height, sha = expected
        if (word["file"], word["text"], word["width"], word["height"], word["rgbaSha256"]) != expected or type(word["width"]) is not int or type(word["height"]) is not int:
            raise ValueError("Main word-art identity/order/geometry changed")
        count = width * height
        runs = word["alphaRuns"]
        if type(runs) is not list or not runs or len(runs) % 2 or len(runs) > 2 * count:
            raise ValueError("Main word-art run count changed")
        alpha = bytearray()
        for at in range(0, len(runs), 2):
            length, value = runs[at:at + 2]
            if not _integer(length, 1, 65535) or not _integer(value, 0, 255) or length > count - len(alpha):
                raise ValueError("Main word-art run overflow/value")
            alpha.extend(bytes((value,)) * length)
        if len(alpha) != count or not any(alpha):
            raise ValueError("Main word-art decoded length/ink changed")
        rgba = bytearray(b"\xff" * (count * 4))
        rgba[3::4] = alpha
        if hashlib.sha256(rgba).hexdigest() != sha:
            raise ValueError("Main word-art RGBA digest changed")
        decoded.append(bytes(rgba))
    return tuple(decoded)

def _load(source_path=None):
    # Bound the file before JSON parsing; a changed file is never silently used.
    with Path(ART_PATH if source_path is None else source_path).open("rb") as stream:
        raw = stream.read(SOURCE_BYTES + 1)
    if len(raw) != SOURCE_BYTES or hashlib.sha256(raw).hexdigest() != SOURCE_SHA:
        raise ValueError("Main word-art source hash/length changed")
    try:
        document = json.loads(raw, object_pairs_hook=_pairs)
    except (UnicodeError, json.JSONDecodeError) as error:
        raise ValueError("Invalid Main word-art JSON") from error
    return _decode(document)

def _placements(placements):
    if type(placements) is not list or len(placements) != len(WORDS):
        raise ValueError("Main word-art placement inventory changed")
    result = []
    for placement, (file, _, width, height, _) in zip(placements, WORDS):
        if type(placement) is not dict or placement.get("file") != file:
            raise ValueError("Main word-art placement identity/order changed")
        atlas, crop = placement.get("atlas"), placement.get("sourceCrop")
        if type(atlas) is not list or len(atlas) != 4 or not all(_integer(v, 0, 2048) for v in atlas):
            raise ValueError("Main word-art atlas geometry changed")
        x, y, w, h = atlas
        if (w, h) != (width, height) or x + w > ATLAS_WIDTH or y + h > ATLAS_HEIGHT:
            raise ValueError("Main word-art rectangle out of bounds")
        if type(crop) is not list or len(crop) != 4 or any(type(v) is not int for v in crop) or crop != [0, 0, width, height]:
            raise ValueError("Main word-art source crop changed")
        for prior in result:
            px, py, pw, ph = prior["atlas"]
            if x < px + pw and px < x + w and y < py + ph and py < y + h:
                raise ValueError("Main word-art rectangles overlap")
        result.append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop)))
    return result

def apply_main_word_art(pixels, placements, *, source_path=None):
    """Replace only the eight validated rectangles in the existing RGBA atlas."""
    if type(pixels) is not bytearray or len(pixels) != ATLAS_WIDTH * ATLAS_HEIGHT * 4:
        raise ValueError("Main word-art needs the existing 1024x2048 RGBA bytearray")
    rgba = _load(source_path)
    checked = _placements(placements)
    # Every validation completes before the first write; invalid input is inert.
    changed = 0
    for image, placement in zip(rgba, checked):
        x, y, width, height = placement["atlas"]
        for row in range(height):
            at = ((y + row) * ATLAS_WIDTH + x) * 4
            replacement = image[row * width * 4:(row + 1) * width * 4]
            old = pixels[at:at + width * 4]
            changed += sum(old[k:k + 4] != replacement[k:k + 4] for k in range(0, len(old), 4))
            pixels[at:at + width * 4] = replacement
    return dict(sourceSha=SOURCE_SHA, newRgba=_identities(), placements=checked, changedPixels=changed)

def verify_main_word_art(metadata, *, source_path=None):
    """Validate frozen art and placement metadata for layout-only regeneration."""
    _load(source_path)
    if type(metadata) is not dict or set(metadata) != {"sourceSha", "newRgba", "placements", "changedPixels"} or metadata["sourceSha"] != SOURCE_SHA or metadata["newRgba"] != _identities():
        raise ValueError("Main word-art metadata identity changed")
    if _placements(metadata["placements"]) != metadata["placements"]:
        raise ValueError("Main word-art metadata placements are not canonical")
    if not _integer(metadata["changedPixels"], 0, sum(w * h for _, _, w, h, _ in WORDS)):
        raise ValueError("Main word-art changed pixel count out of bounds")
