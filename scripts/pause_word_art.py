"""Pinned finite metallic pause word images; no runtime font dependencies."""
# SPDX-License-Identifier: MIT
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ART_DIR = ROOT / "assets/hud/pause"
CATALOG_BYTES = 3200
CATALOG_SHA256 = "16805ac53223dde5ec39c39aad04b011fce8fa892c309aa3650646ea7d780bb9"
SOURCE_FILES = (('classic-word-art-0.json', 813240, 'b9af7a21abdbd95ef53bb5f7af94aa17ef99688dd944d136f1a1ec8e284b1865'), ('classic-word-art-1.json', 820163, '4d03a4a3e82d3d9509d8723b1f78e5cee4d50a864bb31807921dcf61e7baf795'))
PART_COUNTS = (3, 3)
SOURCE_HASHES = {name: sha for name, _, sha in SOURCE_FILES}
SCOPE = "Six fixed classic metallic pause word graphics; no font file, alphabet or reusable glyph font."
REGION = (320, 640, 704, 384)
WORDS = (('continue', 'CONTINUE', (322, 642, 305, 79), (10, 9, 315, 88), '737f28d21ee9783b6f240c0e061c24f57e0c01606d98d317c0153c5626883991'), ('restart', 'RESTART', (631, 642, 270, 79), (11, 9, 281, 88), '374c963ccfcdf5b00b4a310da2997357ae132b2aa1ecbaed06949cd8fdf46f49'), ('racing-music', 'RACING MUSIC', (322, 725, 437, 79), (11, 9, 448, 88), 'f8fc7aaac4f0b0704b89bc9f4ef092d0a456363f52a18cec9b62f800c23de826'), ('sound-effects', 'SOUND EFFECTS', (322, 814, 487, 79), (9, 9, 496, 88), '7319c2efc92dd89646d57e9bcbd1d064567c14487d56ed892cef9367731e6246'), ('quit', 'QUIT', (763, 725, 139, 85), (10, 9, 149, 94), 'aa44bc7cc1f8ac93bac53768f4aad1c8dd625c58c29e3a5a1f0ac1f76ce6ac95'), ('quit-to-system', 'QUIT TO SYSTEM', (322, 897, 496, 85), (10, 17, 506, 102), '2792e754547a66d72b673bed000562e482abd697339d170006e89ca81161fa67'))


def _integer(value, low, high):
    return type(value) is int and low <= value <= high


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate pause word-art JSON key")
        result[key] = value
    return result


def _identities():
    return [dict(name=name, text=text, atlas=list(atlas), sourceCrop=list(crop), rgbaSha256=sha)
            for name, text, atlas, crop, sha in WORDS]


def _decode(document):
    if (type(document) is not dict or set(document) != {"schema", "scope", "typeface", "geometryChanged", "words"}
            or type(document["schema"]) is not int or document["schema"] != 1
            or document["scope"] != SCOPE or document["typeface"] != "Haettenschweiler"
            or document["geometryChanged"] is not False
            or type(document["words"]) is not list or len(document["words"]) != 6):
        raise ValueError("Pause word-art schema/scope/inventory changed")
    result = []
    for word, expected in zip(document["words"], _identities()):
        if type(word) is not dict or set(word) != set(expected) | {"colorRuns"}:
            raise ValueError("Pause word-art fields changed")
        if {key:word[key] for key in expected} != expected:
            raise ValueError("Pause word-art identity/order/geometry changed")
        for key in ("atlas", "sourceCrop"):
            if type(word[key]) is not list or any(type(v) is not int for v in word[key]):
                raise ValueError("Pause word-art noninteger geometry")
        count = expected["atlas"][2] * expected["atlas"][3]
        runs = word["colorRuns"]
        if type(runs) is not list or not runs or len(runs) % 5 or len(runs) > count * 5:
            raise ValueError("Pause word-art unbounded/partial runs")
        rgba = bytearray()
        for index in range(0, len(runs), 5):
            length, *color = runs[index:index+5]
            if (not _integer(length, 1, 65535) or length > count - len(rgba)//4
                    or any(not _integer(channel, 0, 255) for channel in color)):
                raise ValueError("Pause word-art run overflow/channel changed")
            rgba.extend(bytes(color) * length)
        if len(rgba) != count * 4 or not any(rgba[3::4]):
            raise ValueError("Pause word-art decoded length/ink changed")
        if hashlib.sha256(rgba).hexdigest() != expected["rgbaSha256"]:
            raise ValueError("Pause word-art decoded digest changed")
        result.append(bytes(rgba))
    return tuple(result)


def _load(source_dir=None):
    directory = Path(ART_DIR if source_dir is None else source_dir)
    raw_parts = []
    # Authenticate every explicitly selected bounded part before decoding anything.
    for name, size, sha in SOURCE_FILES:
        with (directory / name).open("rb") as stream:
            raw = stream.read(size + 1)
        if len(raw) != size or hashlib.sha256(raw).hexdigest() != sha:
            raise ValueError("Pause word-art source hash/length changed")
        raw_parts.append(raw)
    documents = []
    for index, raw in enumerate(raw_parts):
        try:
            document = json.loads(raw, object_pairs_hook=_pairs)
        except (UnicodeError, json.JSONDecodeError) as error:
            raise ValueError("Invalid pause word-art JSON") from error
        if (type(document) is not dict or set(document) != {"schema", "scope", "typeface", "geometryChanged", "words", "part"}
                or type(document["part"]) is not int or document["part"] != index
                or type(document["words"]) is not list or len(document["words"]) != PART_COUNTS[index]):
            raise ValueError("Pause word-art part inventory/order changed")
        documents.append(document)
    merged = {key:value for key, value in documents[0].items() if key != "part"}
    if any(documents[0][key] != part[key] for part in documents[1:]
           for key in ("schema", "scope", "typeface", "geometryChanged")):
        raise ValueError("Pause word-art part scopes differ")
    merged["words"] = [word for part in documents for word in part["words"]]
    return _decode(merged)


def _overlap(a, b):
    x, y, width, height = a
    xx, yy, ww, hh = b
    return x < xx+ww and xx < x+width and y < yy+hh and yy < y+height


def _catalog(catalog, source_dir=None):
    directory = Path(ART_DIR if source_dir is None else source_dir)
    with (directory / "catalog.json").open("rb") as stream:
        raw = stream.read(CATALOG_BYTES + 1)
    if len(raw) != CATALOG_BYTES or hashlib.sha256(raw).hexdigest() != CATALOG_SHA256:
        raise ValueError("Pause original catalog hash/length changed")
    document = json.loads(raw, object_pairs_hook=_pairs)
    try:
        same = (type(catalog) is dict and
                json.dumps(catalog, sort_keys=True, separators=(",", ":"), allow_nan=False) ==
                json.dumps(document, sort_keys=True, separators=(",", ":"), allow_nan=False))
    except (TypeError, ValueError) as error:
        raise ValueError("Invalid pause original catalog") from error
    if not same or catalog["region"] != list(REGION) or len(catalog["entries"]) != 6:
        raise ValueError("Pause original catalog identity/region changed")
    observed = []
    for entry, (name, _, atlas, crop, _) in zip(catalog["entries"], WORDS):
        x, y, width, height = atlas
        if (entry["name"] != name or entry["file"] != name+".svg"
                or catalog["placements"][name]["atlasInkBounds"] != list(atlas)
                or catalog["placements"][name]["originalRasterInkBounds"] != list(crop)
                or not (REGION[0] <= x < x+width <= REGION[0]+REGION[2]
                        and REGION[1] <= y < y+height <= REGION[1]+REGION[3])
                or crop[2]-crop[0] != width or crop[3]-crop[1] != height
                or any(_overlap(atlas, previous) for previous in observed)):
            raise ValueError("Pause original catalog allocation/order/crop changed")
        observed.append(atlas)
    return catalog


def apply_pause_word_art(region, catalog, *, source_dir=None):
    """Replace six fixed colored crops in the existing 704x384 RGBA region."""
    if type(region) is not bytearray or len(region) != REGION[2] * REGION[3] * 4:
        raise ValueError("Pause word-art needs the existing 704x384 RGBA bytearray")
    images = _load(source_dir)
    _catalog(catalog, source_dir)
    changed = 0
    for image, (_, _, (x, y, width, height), _, _) in zip(images, WORDS):
        for row in range(height):
            at = ((y-REGION[1]+row)*REGION[2]+x-REGION[0])*4
            replacement = image[row*width*4:(row+1)*width*4]
            old = region[at:at+width*4]
            changed += sum(old[i:i+4] != replacement[i:i+4] for i in range(0, len(old), 4))
            region[at:at+width*4] = replacement
    return dict(sourceHashes=dict(SOURCE_HASHES), newRgba=_identities(), changedPixels=changed)


def verify_pause_word_art(metadata, catalog, *, source_dir=None):
    """Authenticate selected sources and original geometry without atlas writes."""
    _load(source_dir)
    _catalog(catalog, source_dir)
    if (type(metadata) is not dict or set(metadata) != {"sourceHashes", "newRgba", "changedPixels"}
            or metadata["sourceHashes"] != SOURCE_HASHES or metadata["newRgba"] != _identities()
            or not _integer(metadata["changedPixels"], 0, sum(w[2][2]*w[2][3] for w in WORDS))):
        raise ValueError("Pause word-art metadata changed")
