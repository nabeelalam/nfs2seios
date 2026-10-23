"""Pinned finite uppercase Player heading/button graphics; no font dependencies."""
# SPDX-License-Identifier: MIT
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ART_DIR = ROOT / "assets/hud/frontend"
SOURCE_FILE = "player-heading-art.json"
SOURCE_BYTES = 177782
SOURCE_SHA256 = "d61620832e1e27f8257f09d102c39d1fc60da8c4c8e7666c9a2d34b2e511cd55"
SOURCE_HASHES = {SOURCE_FILE: SOURCE_SHA256}
SOURCE_FILES = ((SOURCE_FILE, SOURCE_BYTES, SOURCE_SHA256),)
ATLAS_WIDTH, ATLAS_HEIGHT = 1024, 2048
FAMILIES = ("main", "location", "setup", "player", "opponents", "options")
SCOPE = "Four fixed uppercase Player/shared heading and button graphics; no font file, alphabet or reusable glyph font."
WORDS = (('done.svg', 'DONE', (538, 1882, 128, 76), (0, 11, 128, 76), '217a110df4a5b2bb6116e7575b9568d2a892b248d6e2d57c6f023f89c9fb3933', ('location/done.svg', 'setup/done.svg', 'player/done.svg', 'opponents/done.svg', 'options/done.svg')), ('cancel.svg', 'CANCEL', (0, 1835, 184, 76), (0, 11, 184, 76), '80c4aa81ca550ffcb667b6c556cce454beeff2d640522ceb9a57939fff3c314f', ('location/cancel.svg', 'setup/cancel.svg', 'player/cancel.svg', 'opponents/cancel.svg')), ('player-one-heading.svg', 'PLAYER ONE', (592, 961, 284, 76), (0, 11, 284, 76), '6aa0f1bcb37601c25e67dd0e822185b21a54c24aca5f4216063d6e36bb7ea632', ('player/player-one-heading.svg',)), ('player-two-heading.svg', 'PLAYER TWO', (652, 800, 292, 76), (0, 11, 292, 76), '82bb3db4a9dee018996f36f7e1f99a81dbd7a5ceebb687b668f21e5cf438f586', ('player/player-two-heading.svg',)))


def _integer(value, low, high):
    return type(value) is int and low <= value <= high


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate Player heading-art JSON key")
        result[key] = value
    return result


def _identities():
    return [dict(file=file, text=text, atlas=list(atlas), sourceCrop=list(crop),
                 rgbaSha256=sha, sharedUses=list(aliases))
            for file, text, atlas, crop, sha, aliases in WORDS]


def _decode(document):
    if (type(document) is not dict or set(document) != {"schema", "scope", "typeface", "geometryChanged", "words"}
            or type(document["schema"]) is not int or document["schema"] != 1
            or document["scope"] != SCOPE
            or document["typeface"] != "Haettenschweiler" or document["geometryChanged"] is not False):
        raise ValueError("Player heading-art schema/scope changed")
    if type(document["words"]) is not list or len(document["words"]) != len(WORDS):
        raise ValueError("Player heading-art inventory changed")
    result = []
    for word, expected in zip(document["words"], _identities()):
        if type(word) is not dict or set(word) != set(expected) | {"alphaRuns"}:
            raise ValueError("Player heading-art fields changed")
        if {key:word[key] for key in expected} != expected:
            raise ValueError("Player heading-art identity/order/geometry/aliases changed")
        for key in ("atlas", "sourceCrop"):
            if type(word[key]) is not list or any(type(v) is not int for v in word[key]):
                raise ValueError("Player heading-art noninteger geometry")
        _, _, width, height = expected["atlas"]
        count, runs = width * height, word["alphaRuns"]
        if type(runs) is not list or not runs or len(runs) % 2 or len(runs) > count * 2:
            raise ValueError("Player heading-art unbounded/odd runs")
        alpha = bytearray()
        for i in range(0, len(runs), 2):
            length, value = runs[i:i+2]
            if not _integer(length, 1, 65535) or not _integer(value, 0, 255) or length > count - len(alpha):
                raise ValueError("Player heading-art run overflow/value")
            alpha.extend(bytes((value,)) * length)
        if len(alpha) != count or not any(alpha):
            raise ValueError("Player heading-art decoded length/ink changed")
        rgba = bytearray(b"\xff" * (count * 4))
        rgba[3::4] = alpha
        if hashlib.sha256(rgba).hexdigest() != expected["rgbaSha256"]:
            raise ValueError("Player heading-art decoded digest changed")
        result.append(bytes(rgba))
    return tuple(result)


def _load(source_dir=None):
    directory = Path(ART_DIR if source_dir is None else source_dir)
    # Authenticate the explicitly selected bounded source before any atlas write.
    with (directory / SOURCE_FILE).open("rb") as stream:
        raw = stream.read(SOURCE_BYTES + 1)
    if len(raw) != SOURCE_BYTES or hashlib.sha256(raw).hexdigest() != SOURCE_SHA256:
        raise ValueError("Player heading-art source hash/length changed")
    try:
        document = json.loads(raw, object_pairs_hook=_pairs)
    except (UnicodeError, json.JSONDecodeError) as error:
        raise ValueError("Invalid Player heading-art JSON") from error
    return _decode(document)


def _overlap(a, b):
    x, y, w, h = a
    xx, yy, ww, hh = b
    return x < xx + ww and xx < x + w and y < yy + hh and yy < y + h


def _placements(families):
    if type(families) is not dict or set(families) != set(FAMILIES):
        raise ValueError("Player heading-art requires all six final placement families")
    observed = []
    for file, text, atlas, crop, sha, aliases in WORDS:
        if not (0 <= atlas[0] < atlas[0]+atlas[2] <= ATLAS_WIDTH and 0 <= atlas[1] < atlas[1]+atlas[3] <= ATLAS_HEIGHT):
            raise ValueError("Player heading-art fixed rectangle out of bounds")
        if any(_overlap(atlas, prior["atlas"]) for prior in observed):
            raise ValueError("Player heading-art fixed rectangles overlap")
        observed.append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop), sharedUses=[]))
    seen = set()
    for family in FAMILIES:
        entries = families[family]
        if type(entries) is not list or not 1 <= len(entries) <= 128:
            raise ValueError("Player heading-art unbounded placement family")
        for entry in entries:
            if type(entry) is not dict or type(entry.get("file")) is not str:
                raise ValueError("Invalid Player heading-art placement")
            name = family + "/" + entry["file"]
            if name in seen:
                raise ValueError("Duplicate Player heading-art family placement")
            seen.add(name)
            atlas, crop = entry.get("atlas"), entry.get("sourceCrop")
            if (type(atlas) is not list or len(atlas) != 4 or not all(_integer(v, 0, 2048) for v in atlas)
                    or not atlas[2] or not atlas[3] or atlas[0]+atlas[2] > ATLAS_WIDTH or atlas[1]+atlas[3] > ATLAS_HEIGHT
                    or type(crop) is not list or len(crop) != 4 or not all(_integer(v, 0, 2048) for v in crop)
                    or crop[0] != 0 or crop[2:] != atlas[2:]):
                raise ValueError("Player heading-art placement geometry invalid")
            for record, (_, _, fixed, fixed_crop, _, aliases) in zip(observed, WORDS):
                if name in aliases:
                    if atlas != list(fixed) or crop != list(fixed_crop):
                        raise ValueError("Player heading-art alias geometry changed")
                    record["sharedUses"].append(name)
                elif _overlap(atlas, fixed):
                    raise ValueError("Player heading-art write overlaps unapproved placement")
    for record, (_, _, _, _, _, aliases) in zip(observed, WORDS):
        if record["sharedUses"] != list(aliases):
            raise ValueError("Player heading-art missing/reordered/unapproved shared uses")
    return observed


def apply_player_heading_art(pixels, family_placements, *, source_dir=None):
    """Replace four fixed allocations after packing and the earlier fixed-word overrides."""
    if type(pixels) is not bytearray or len(pixels) != ATLAS_WIDTH * ATLAS_HEIGHT * 4:
        raise ValueError("Player heading-art needs the existing 1024x2048 RGBA bytearray")
    images, checked = _load(source_dir), _placements(family_placements)
    changed = 0
    for image, placement in zip(images, checked):
        x, y, width, height = placement["atlas"]
        for row in range(height):
            at = ((y + row) * ATLAS_WIDTH + x) * 4
            replacement = image[row*width*4:(row+1)*width*4]
            old = pixels[at:at+width*4]
            changed += sum(old[i:i+4] != replacement[i:i+4] for i in range(0, len(old), 4))
            pixels[at:at+width*4] = replacement
    return dict(sourceHashes=dict(SOURCE_HASHES), newRgba=_identities(), placements=checked, changedPixels=changed)


def verify_player_heading_art(metadata, family_placements, *, source_dir=None):
    """Verify selected-project provenance and exact aliases during layout-only work."""
    _load(source_dir)
    checked = _placements(family_placements)
    if (type(metadata) is not dict or set(metadata) != {"sourceHashes", "newRgba", "placements", "changedPixels"}
            or metadata["sourceHashes"] != SOURCE_HASHES or metadata["newRgba"] != _identities()
            or metadata["placements"] != checked
            or not _integer(metadata["changedPixels"], 0, sum(w[2][2]*w[2][3] for w in WORDS))):
        raise ValueError("Player heading-art metadata changed")
