"""Pinned finite Player/shared whole-word graphics; no font dependencies."""
# SPDX-License-Identifier: MIT
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ART_DIR = ROOT / "assets/hud/frontend"
SOURCE_FILES = (('player-word-art-0.json', 652605, '03338501fd1da1caf81da6d16f8775ef8accd1ffaf35a0965040d7732ffbaa29'), ('player-word-art-1.json', 767535, '6b204dfc6f0b8b184a7e56c72e92e542b997c124f4cb914563f5a68b32e81a34'))
SOURCE_HASHES = {name: sha for name, _, sha in SOURCE_FILES}
ATLAS_WIDTH, ATLAS_HEIGHT = 1024, 2048
FAMILIES = ("main", "location", "setup", "player", "opponents", "options")
WORDS = (('car.svg', 'car', (920, 0, 84, 55), (0, 27, 84, 55), '6832ec3fa00396215a4e10c9a272154aa06986213088d404c90735eb6df4129b', ('player/car.svg', 'opponents/car.svg')), ('transmission.svg', 'transmission', (692, 614, 320, 74), (0, 8, 320, 74), 'f5298a5bcf94fffe7bc3a0a37d1f9b185862de89ab8bd2a81c9ecf1737cb4ef1', ('player/transmission.svg',)), ('color.svg', 'color', (882, 878, 132, 77), (0, 6, 132, 77), 'b235db83482b60b8ec1df6bd8ac109742fec0c623f275a318772bdca3620ecf8', ('player/color.svg',)), ('graph.svg', 'graph', (178, 1916, 148, 96), (0, 6, 148, 96), '477c63d52cad9d46c14c8a1abc0102cc4a8808594564e2fa256f1fe0bdf38a4c', ('player/graph.svg',)), ('colon.svg', ':', (964, 1529, 24, 54), (0, 28, 24, 54), '8a6a9aaeb74025aa02b44cfe9c28a9fcbe3adb2c08754130f40d7b5b48b566be', ('location/colon.svg', 'setup/colon.svg', 'player/colon.svg', 'opponents/colon.svg', 'options/colon.svg')), ('manual.svg', 'manual', (214, 1758, 188, 77), (0, 6, 188, 77), 'fc525465f76710c15d800fab7b1b4bb768d28f7bb4989915f68667fd93b2c697', ('player/manual.svg',)), ('automatic.svg', 'automatic', (528, 1375, 256, 74), (0, 8, 256, 74), '116824336e8019ab2ad1f1e1db2195051a8aae7eebb0087fa5c171325c8e9c19', ('player/automatic.svg',)), ('off.svg', 'off', (668, 1882, 80, 76), (0, 6, 80, 76), '8ce237c1b2d828ade2a8b03716bc241d63bf6358adda03724b269046205a7404', ('setup/off.svg', 'player/off.svg', 'opponents/off.svg', 'options/off.svg')), ('on.svg', 'on', (946, 800, 72, 55), (0, 27, 72, 55), '7fd1b8f6715efdda5eb4457c51eb6a55fdffd2aec02c49960cb9d1a0f78ea8ce', ('setup/on.svg', 'player/on.svg', 'opponents/on.svg', 'options/on.svg')), ('ford-indigo.svg', 'Ford Indigo', (0, 1060, 280, 96), (0, 6, 280, 96), 'ea2313eb8b3bec984a67681bd7dfdca060b83fe88d99ff0ef3b4ba083717b46c', ('player/ford-indigo.svg', 'opponents/ford-indigo.svg')), ('mustang-mach-iii.svg', 'Mustang Mach III', (498, 0, 420, 96), (0, 6, 420, 96), 'a95998e2c3c8aa991831350429686b1eda1a49f79d5834f20c7df3054edab367', ('player/mustang-mach-iii.svg', 'opponents/mustang-mach-iii.svg')), ('jaguar-xj220.svg', 'Jaguar XJ220', (346, 516, 344, 91), (0, 11, 344, 91), '0c3323db001e79f2377f1466cda533d7ed45d6d3d75398c4df2b7f071627b847', ('player/jaguar-xj220.svg', 'opponents/jaguar-xj220.svg')), ('lotus-gt1.svg', 'Lotus GT1', (512, 1451, 252, 71), (0, 11, 252, 71), 'bd90147c7d344e162468cfb9110c7e3a8c5dee6f61e9721e7280361649757adc', ('player/lotus-gt1.svg', 'opponents/lotus-gt1.svg')), ('lotus-esprit-v8.svg', 'Lotus Esprit V8', (0, 196, 372, 94), (0, 8, 372, 94), '07eeb24201d0e730599c47d4cba430ad4a38ed45817beaad085102e852c944f3', ('player/lotus-esprit-v8.svg', 'opponents/lotus-esprit-v8.svg')), ('nazca-c2.svg', 'Nazca C2', (786, 1375, 232, 71), (0, 11, 232, 71), 'ca113bcc486ae3292196d1f726ccf9a50b701324df5e60db73abfc80fae02b09', ('player/nazca-c2.svg', 'opponents/nazca-c2.svg')), ('italdesign-cala.svg', 'Italdesign Cal\xe0', (0, 292, 364, 98), (0, 4, 364, 98), 'aa936151bdb90add357cd87da09a28da9c0e759a5b772e93cea7aae6573a608e', ('player/italdesign-cala.svg', 'opponents/italdesign-cala.svg')), ('isdera-112i.svg', 'Isdera 112i', (744, 219, 272, 76), (0, 6, 272, 76), 'ef9f3181e808d8460d7463938f46fa50fa60345e2334b15fcb437ba6a7d49b99', ('player/isdera-112i.svg', 'opponents/isdera-112i.svg')), ('mclaren-f1.svg', 'McLaren F1', (0, 987, 288, 71), (0, 11, 288, 71), 'ee16f01be7d7ba2014703203b33d0f642cd5d93cb62dccd63f562146fda66833', ('player/mclaren-f1.svg', 'opponents/mclaren-f1.svg')), ('ferrari-f50.svg', 'Ferrari F50', (0, 1158, 272, 74), (0, 8, 272, 74), '3b42108377f72e6fe0d7a47f3fd92e5ee42f1d01b9fdce77552b39bbfc184843', ('player/ferrari-f50.svg', 'opponents/ferrari-f50.svg')), ('ferrari-355-f1.svg', 'Ferrari 355 F1', (0, 611, 344, 74), (0, 8, 344, 74), '0a869f503f3087f539e4d3b6b5d059ed2ab6a90a040ffbaa3755fbbe16f97086', ('player/ferrari-355-f1.svg', 'opponents/ferrari-355-f1.svg')), ('ford-gt90.svg', 'Ford GT90', (266, 1304, 260, 76), (0, 6, 260, 76), 'e4e44a1e4844b7e3869574d5a5ba978e522f92f43029d373c4ea8a7edd200a55', ('player/ford-gt90.svg', 'opponents/ford-gt90.svg')), ('class-a.svg', 'class A', (186, 1837, 176, 77), (0, 6, 176, 77), 'ea0c0bd71eec52ff80c132ccaac927646261b50f72e01aa73f739a190f1d4198', ('player/class-a.svg', 'opponents/class-a.svg')), ('class-b.svg', 'class B', (590, 1803, 176, 77), (0, 6, 176, 77), '80ee0b61134ba6857f61fe54b75f7ab8f6255343691d54d257c605e9b18080bd', ('player/class-b.svg', 'opponents/class-b.svg')), ('class-c.svg', 'class C', (0, 1913, 176, 77), (0, 6, 176, 77), '02579b934d99bf448d091139dfd8906321af114888e62c92df9a8acd0af81d47', ('player/class-c.svg', 'opponents/class-c.svg')), ('acceleration.svg', 'acceleration', (342, 780, 308, 77), (0, 6, 308, 77), '9d67857dea3f33172e6f9efc75b081494e2d003a55dcaccb0f24bd3d34c1babc', ('player/acceleration.svg',)), ('top-speed.svg', 'top speed', (258, 1382, 252, 96), (0, 6, 252, 96), 'c6cf93eff7ba65fc255e4272f9d23adbc3b5ddb9b558712ced620b7f01f6f4c9', ('player/top-speed.svg',)), ('braking.svg', 'braking', (698, 1705, 192, 96), (0, 6, 192, 96), '3101fdd3007a829063af01563a69ee5e5e88977a1b9a86335cca9526a1e91fa4', ('player/braking.svg',)), ('handling.svg', 'handling', (742, 1607, 216, 96), (0, 6, 216, 96), '2bb422508c09f4eb95144a0f31963489f9b119fed71439b72352e56f0cd9b2a9', ('player/handling.svg',)), ('settings.svg', 'settings...', (0, 1504, 248, 94), (0, 8, 248, 94), 'f2aecbcac90812fe08c945d0c3db25f87edb7e35606601365b8a9516d2aa9c0a', ('player/settings.svg',)), ('showcase.svg', 'showcase...', (302, 957, 288, 76), (0, 6, 288, 76), 'f8a312b5b672d9221714be9236554edfe9d722070fa60767c98b7a0180a46793', ('player/showcase.svg',)))


def _integer(value, low, high):
    return type(value) is int and low <= value <= high


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate Player text-art JSON key")
        result[key] = value
    return result


def _identities():
    return [dict(file=file, text=text, atlas=list(atlas), sourceCrop=list(crop),
                 rgbaSha256=sha, sharedUses=list(aliases))
            for file, text, atlas, crop, sha, aliases in WORDS]


def _decode(document):
    if (type(document) is not dict or set(document) != {"schema", "scope", "typeface", "geometryChanged", "words"}
            or type(document["schema"]) is not int or document["schema"] != 1
            or document["scope"] != "Thirty fixed Player/shared text graphics including one fixed colon allocation; no font file, alphabet or reusable glyph font."
            or document["typeface"] != "Haettenschweiler" or document["geometryChanged"] is not False):
        raise ValueError("Player text-art schema/scope changed")
    if type(document["words"]) is not list or len(document["words"]) != 30:
        raise ValueError("Player text-art inventory changed")
    result = []
    for word, expected in zip(document["words"], _identities()):
        if type(word) is not dict or set(word) != set(expected) | {"alphaRuns"}:
            raise ValueError("Player text-art fields changed")
        if {key:word[key] for key in expected} != expected:
            raise ValueError("Player text-art identity/order/geometry/aliases changed")
        for key in ("atlas", "sourceCrop"):
            if type(word[key]) is not list or any(type(v) is not int for v in word[key]):
                raise ValueError("Player text-art noninteger geometry")
        _, _, width, height = expected["atlas"]
        count, runs = width * height, word["alphaRuns"]
        if type(runs) is not list or not runs or len(runs) % 2 or len(runs) > count * 2:
            raise ValueError("Player text-art unbounded/odd runs")
        alpha = bytearray()
        for i in range(0, len(runs), 2):
            length, value = runs[i:i+2]
            if not _integer(length, 1, 65535) or not _integer(value, 0, 255) or length > count - len(alpha):
                raise ValueError("Player text-art run overflow/value")
            alpha.extend(bytes((value,)) * length)
        if len(alpha) != count or not any(alpha):
            raise ValueError("Player text-art decoded length/ink changed")
        rgba = bytearray(b"\xff" * (count * 4))
        rgba[3::4] = alpha
        if hashlib.sha256(rgba).hexdigest() != expected["rgbaSha256"]:
            raise ValueError("Player text-art decoded digest changed")
        result.append(bytes(rgba))
    return tuple(result)


def _load(source_dir=None):
    directory = Path(ART_DIR if source_dir is None else source_dir)
    raw_parts = []
    # Authenticate both bounded files before decoding any image or writing pixels.
    for name, size, sha in SOURCE_FILES:
        with (directory / name).open("rb") as stream:
            raw = stream.read(size + 1)
        if len(raw) != size or hashlib.sha256(raw).hexdigest() != sha:
            raise ValueError("Player text-art source hash/length changed")
        raw_parts.append(raw)
    documents = []
    for index, raw in enumerate(raw_parts):
        try:
            document = json.loads(raw, object_pairs_hook=_pairs)
        except (UnicodeError, json.JSONDecodeError) as error:
            raise ValueError("Invalid Player text-art JSON") from error
        if (type(document) is not dict or set(document) != {"schema", "scope", "typeface", "geometryChanged", "words", "part"}
                or type(document["part"]) is not int or document["part"] != index
                or type(document["words"]) is not list or len(document["words"]) != 15):
            raise ValueError("Player text-art part inventory/order changed")
        documents.append(document)
    merged = {key:value for key, value in documents[0].items() if key != "part"}
    if any(documents[0][key] != documents[1][key] for key in ("schema", "scope", "typeface", "geometryChanged")):
        raise ValueError("Player text-art part scopes differ")
    merged["words"] = documents[0]["words"] + documents[1]["words"]
    return _decode(merged)


def _overlap(a, b):
    x, y, w, h = a
    xx, yy, ww, hh = b
    return x < xx + ww and xx < x + w and y < yy + hh and yy < y + h


def _placements(families):
    if type(families) is not dict or set(families) != set(FAMILIES):
        raise ValueError("Player text-art requires all six final placement families")
    observed = []
    for file, text, atlas, crop, sha, aliases in WORDS:
        if not (0 <= atlas[0] < atlas[0]+atlas[2] <= ATLAS_WIDTH and 0 <= atlas[1] < atlas[1]+atlas[3] <= ATLAS_HEIGHT):
            raise ValueError("Player text-art fixed rectangle out of bounds")
        if any(_overlap(atlas, prior["atlas"]) for prior in observed):
            raise ValueError("Player text-art fixed rectangles overlap")
        observed.append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop), sharedUses=[]))
    seen = set()
    for family in FAMILIES:
        entries = families[family]
        if type(entries) is not list or not 1 <= len(entries) <= 128:
            raise ValueError("Player text-art unbounded placement family")
        for entry in entries:
            if type(entry) is not dict or type(entry.get("file")) is not str:
                raise ValueError("Invalid Player text-art placement")
            name = family + "/" + entry["file"]
            if name in seen:
                raise ValueError("Duplicate Player text-art family placement")
            seen.add(name)
            atlas, crop = entry.get("atlas"), entry.get("sourceCrop")
            if (type(atlas) is not list or len(atlas) != 4 or not all(_integer(v, 0, 2048) for v in atlas)
                    or not atlas[2] or not atlas[3] or atlas[0]+atlas[2] > ATLAS_WIDTH or atlas[1]+atlas[3] > ATLAS_HEIGHT
                    or type(crop) is not list or len(crop) != 4 or not all(_integer(v, 0, 2048) for v in crop)
                    or crop[0] != 0 or crop[2:] != atlas[2:]):
                raise ValueError("Player text-art placement geometry invalid")
            for record, (_, _, fixed, fixed_crop, _, aliases) in zip(observed, WORDS):
                if name in aliases:
                    if atlas != list(fixed) or crop != list(fixed_crop):
                        raise ValueError("Player text-art alias geometry changed")
                    record["sharedUses"].append(name)
                elif _overlap(atlas, fixed):
                    raise ValueError("Player text-art write overlaps unapproved placement")
    for record, (_, _, _, _, _, aliases) in zip(observed, WORDS):
        if record["sharedUses"] != list(aliases):
            raise ValueError("Player text-art missing/reordered/unapproved shared uses")
    return observed


def apply_player_word_art(pixels, family_placements, *, source_dir=None):
    """Replace thirty fixed allocations after packing and the Main override."""
    if type(pixels) is not bytearray or len(pixels) != ATLAS_WIDTH * ATLAS_HEIGHT * 4:
        raise ValueError("Player text-art needs the existing 1024x2048 RGBA bytearray")
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


def verify_player_word_art(metadata, family_placements, *, source_dir=None):
    """Verify selected-project provenance and exact aliases during layout-only work."""
    _load(source_dir)
    checked = _placements(family_placements)
    if (type(metadata) is not dict or set(metadata) != {"sourceHashes", "newRgba", "placements", "changedPixels"}
            or metadata["sourceHashes"] != SOURCE_HASHES or metadata["newRgba"] != _identities()
            or metadata["placements"] != checked
            or not _integer(metadata["changedPixels"], 0, sum(w[2][2]*w[2][3] for w in WORDS))):
        raise ValueError("Player text-art metadata changed")
