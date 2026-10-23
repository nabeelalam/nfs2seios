"""Pinned finite remaining menu whole-word graphics; no font dependencies."""
# SPDX-License-Identifier: MIT
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ART_DIR = ROOT / "assets/hud/frontend"
SOURCE_FILES = (('remaining-menu-word-art-0.json', 824252, '09bcaa210ace3475355c000bf5c011a4d6062f8b9d28978dd1d8c425a953ed62'), ('remaining-menu-word-art-1.json', 811951, '7f8b0509e724aa915a555a4962c06622566d098d7956acfff96f4eb390e4cae9'), ('remaining-menu-word-art-2.json', 762740, 'd909a10035496398a7a906c3364ead826f2f01408af14095f9061651d46f47a2'))
PART_COUNTS = (16, 17, 17)
SOURCE_HASHES = {name: sha for name, _, sha in SOURCE_FILES}
ATLAS_WIDTH, ATLAS_HEIGHT = 1024, 2048
FAMILIES = ("main", "location", "setup", "player", "opponents", "options")
SCOPE = "Fifty fixed remaining menu body/value graphics; no font file, alphabet or reusable glyph font."
WORDS = (('track.svg', 'track', (878, 961, 132, 76), (0, 6, 132, 76), '031127e6a6859f21262591c47dccd7feeea41c46054e4a7bf0069ce92d3b2f2f', ('location/track.svg',)), ('north-country.svg', 'North Country', (366, 297, 352, 96), (0, 6, 352, 96), '0e22ca6b809a8116d3963f1780888918a79bf47e0745b98de75c96071f54555b', ('location/north-country.svg',)), ('laps.svg', 'laps', (898, 1938, 108, 96), (0, 6, 108, 96), '0b2b2fb90e456f11a9076c353720e25886f525b96425c0ad1a4eb16f354e7d08', ('location/laps.svg',)), ('lap-two.svg', '2', (960, 1607, 40, 71), (0, 11, 40, 71), '2467429c1b2b1ea001800101a19429f9d74954feabb1771e82983c70188475a7', ('location/lap-two.svg',)), ('track-info.svg', 'track info...', (576, 1039, 280, 76), (0, 6, 280, 76), 'e3b79c9997b32710861db6c06238159ebef0177a00844f967d6f72c5fd2585b0', ('location/track-info.svg',)), ('track-records.svg', 'track records...', (374, 219, 368, 76), (0, 6, 368, 76), '8da3449e4fd6bf317e7b046e246730fd34a5db8f813cd7fcf7b836a287630d68', ('location/track-records.svg',)), ('personal-statistics.svg', 'personal statistics...', (0, 0, 496, 96), (0, 6, 496, 96), 'b5017a6c702be807601baf87d2dee84101bf625ba9e92b54a211c187fca5f779', ('location/personal-statistics.svg',)), ('pacific-spirit.svg', 'Pacific Spirit', (704, 395, 312, 96), (0, 6, 312, 96), '75b8617ce38f067b44508da2b9c0b355522afcc907ea2e87c9f44e02c0c2ece3', ('location/pacific-spirit.svg',)), ('mediterraneo.svg', 'Mediterraneo', (342, 702, 336, 76), (0, 6, 336, 76), '77d64afb38167e63bd3994593c573dadf2cfd2c0dd736fd71c9d66c227d07d2a', ('location/mediterraneo.svg',)), ('mystic-peaks.svg', 'Mystic Peaks', (692, 516, 328, 96), (0, 6, 328, 96), '2a0da59d830593fb850d0260b58532b6481a6361c592f7c24c78773401b64b98', ('location/mystic-peaks.svg',)), ('proving-grounds.svg', 'Proving Grounds', (0, 98, 408, 96), (0, 6, 408, 96), '2902665a53aa88e7fef1c3bf08d2d1ec7f592578eecad463e521de55a4c62b6c', ('location/proving-grounds.svg',)), ('outback.svg', 'Outback', (0, 1757, 212, 76), (0, 6, 212, 76), 'f2a5f1581301b2bc481fe7ff9c3154fdb86cae3d7796e5910155dafa3f356f8b', ('location/outback.svg',)), ('last-resort.svg', 'Last Resort', (290, 1035, 284, 71), (0, 11, 284, 71), 'bba575373a317c6ffefc7fe65bc62247b7d19b302f6d914c55afd77029240a69', ('location/last-resort.svg',)), ('lap-4.svg', '4', (698, 1617, 40, 71), (0, 11, 40, 71), '76e6a9e29273ac39f114c1f934bedc97cbb1fcc5ca663e080b634b6c87db7f6c', ('location/lap-4.svg',)), ('lap-8.svg', '8', (646, 1715, 40, 71), (0, 11, 40, 71), 'e5ec1e25f8f11ee8f4bb9f9f8d69108bdfa2e9055c40eee3d67d68487686dc15', ('location/lap-8.svg',)), ('game-type.svg', 'game type', (0, 1234, 264, 91), (0, 11, 264, 91), '83109ad34fb658abddcb2461bf21eee9d2a432fdaed5051e790aa26a5f2afe46', ('setup/game-type.svg',)), ('race-type.svg', 'race type', (504, 1524, 236, 91), (0, 11, 236, 91), '05ac82f617ed620acad3eeb5bbd408be5ef154a95d2b2d47d0591e7e07fe83f2', ('setup/race-type.svg',)), ('style.svg', 'style', (892, 1705, 128, 96), (0, 6, 128, 96), 'e885c2c3202ccc8737c584d0edd03b58e58d7a94760fba2f84468e65dbace132', ('setup/style.svg',)), ('catch-up.svg', 'catch up', (480, 1617, 216, 96), (0, 6, 216, 96), '63d7189309a17cb07f9f9dcafef97273a206db915e95346655c87feb17e10a18', ('setup/catch-up.svg',)), ('backwards.svg', 'backwards', (556, 1117, 268, 76), (0, 6, 268, 76), '8118e87eb161d7ee24442bf059a53ad196dc5a9d18aa38f27c3e99aa2f352f4e', ('setup/backwards.svg',)), ('mirrored.svg', 'mirrored', (742, 1529, 220, 76), (0, 6, 220, 76), 'be829699d033618c3180bdf4f91536f9b85b0a56e01bee334589cdfdaf99e320', ('setup/mirrored.svg',)), ('one-player.svg', 'one player', (274, 1206, 264, 96), (0, 6, 264, 96), 'fba8fd1a571b94476c8cd8157c2e409ec860eb91833e2ef0ed41b4306b751db8', ('setup/one-player.svg',)), ('split-screen.svg', 'split screen', (302, 859, 288, 96), (0, 6, 288, 96), '32ddadd12cc1aff5ee6ef92df2fbf98e126e01bd8e0ccdd584049fd343273c0b', ('setup/split-screen.svg',)), ('modem.svg', 'modem', (452, 1715, 192, 76), (0, 6, 192, 76), '5a01ea756b946fee239ace80ad3b6ce8568546f0bc799d0b2b35618c04a91f33', ('setup/modem.svg',)), ('serial-link.svg', 'serial link', (0, 1600, 240, 77), (0, 6, 240, 77), '5d6590c17b9603c2f1bb0f261e39c8b725bab18d6a0b14569a273f64f4e2e021', ('setup/serial-link.svg',)), ('network.svg', 'network', (238, 1680, 212, 76), (0, 6, 212, 76), 'e5c79f5cbbe31bce6ea1b21970cbb6f60f40ac60cdceec574a6f935d8ef076bd', ('setup/network.svg',)), ('single-race.svg', 'single race', (282, 1108, 272, 96), (0, 6, 272, 96), 'a77b5e883217ba2f774e1dde2fa6bc9ba9b0c358213becf062d4e1196034f798', ('setup/single-race.svg',)), ('tournament.svg', 'tournament', (720, 297, 296, 71), (0, 11, 296, 71), 'b804c9040ae81e4c6a7a92b1d892f0ddc81460da3c07c1b6e949650552b2923c', ('setup/tournament.svg',)), ('knockout.svg', 'knockout', (0, 1679, 236, 76), (0, 6, 236, 76), '59892e06a84581a3ec0844930ddc38ce45ad15d605c188c88c78148ef1f7935c', ('setup/knockout.svg',)), ('wild.svg', 'wild', (446, 1960, 108, 77), (0, 6, 108, 77), '7119d1a763a1b89c6929f601877ee74a793415a2a25fd58375b7c46964c60e00', ('setup/wild.svg',)), ('arcade.svg', 'arcade', (364, 1871, 172, 76), (0, 6, 172, 76), 'd449c5019f3c302a6b3ed58ec1ca7685fa925d0ab2e1db61e10de841101a0a4e', ('setup/arcade.svg',)), ('simulation.svg', 'simulation', (540, 1296, 264, 77), (0, 6, 264, 77), '609e7eb1e5640e3f393fa5af11000e0023bfccdfd51ed04707060877e3809df4', ('setup/simulation.svg',)), ('number.svg', 'number', (826, 1117, 196, 76), (0, 6, 196, 76), '0c9c68c6bada87999bc85e771d4a473872cb79adcbf9ba413bda6e629e823ec8', ('opponents/number.svg',)), ('skill-level.svg', 'skill level', (242, 1601, 236, 77), (0, 6, 236, 77), '11e3653b752886677b17a8859f6d90898a472e8a415770e92682c1ded8747847', ('opponents/skill-level.svg',)), ('traffic.svg', 'traffic', (858, 1039, 160, 76), (0, 6, 160, 76), 'ac9a014065e38fcc3bb2bdd5381c4639c5ccc37ee8abd38cbd66f861439813b0', ('opponents/traffic.svg',)), ('no-opponents.svg', 'no opponents', (346, 609, 344, 91), (0, 11, 344, 91), '11b9e787364d72ee09baca2f85c2ad8513e387e84a7f42f26b64c6263b0e04ed', ('opponents/no-opponents.svg',)), ('full-grid.svg', 'full grid', (822, 1195, 196, 96), (0, 6, 196, 96), '85f444c1723d99ab2d53bee7ad4be6f5ed65ec413bed0ba1a761e63b3a780c84', ('opponents/full-grid.svg',)), ('none.svg', 'none', (768, 1803, 132, 55), (0, 27, 132, 55), 'ab397f3ee372a3eb70cc7809ebf835e8d1fc066c0d5deb98c95e233551be41e4', ('opponents/none.svg',)), ('one.svg', 'one', (902, 1803, 100, 55), (0, 27, 100, 55), '3f78f4c39e57ff1227bacc95a7914ef7a19ecfcffe106ce09f2022baacc234d8', ('opponents/one.svg',)), ('beginner.svg', 'beginner', (792, 98, 224, 96), (0, 6, 224, 96), 'cd21d0cc461dbfddff1762ed954a30790641e3d9958571fccdc48512133aa9eb', ('opponents/beginner.svg',)), ('advanced.svg', 'advanced', (766, 1451, 240, 76), (0, 6, 240, 76), '08f23e65946dced163f331597f8e6ae3e2511069df0f4716ccdda69c98ef673b', ('opponents/advanced.svg',)), ('speedometer.svg', 'speedometer', (680, 702, 332, 96), (0, 6, 332, 96), 'e9d110567d257ac8297914174d095a01e3bb290e050ec560b7d7c73b9ce37b30', ('options/speedometer.svg',)), ('mph.svg', 'mph', (328, 1949, 116, 96), (0, 6, 116, 96), '0c5125abc74007e927cc4fbd1fdc4843915aaa37fd179cf36b968d96d29838b4', ('options/mph.svg',)), ('kmh.svg', 'kmh', (898, 1860, 116, 76), (0, 6, 116, 76), '02c96e94764cadfc88d958ece6306fa303da63c49cc7ad358149a11779ea8262', ('options/kmh.svg',)), ('personal-stats.svg', 'personal stats', (0, 392, 352, 96), (0, 6, 352, 96), '6b4757a383aeea1ade84787f528213ddb2fa1e17f31a72818085949265b8d18f', ('options/personal-stats.svg',)), ('audio.svg', 'audio...', (404, 1793, 184, 76), (0, 6, 184, 76), '6d6aa017103b4cb969827a5807eea023c0be494629f850a7c2ceb3bb01cd929d', ('options/audio.svg',)), ('controls.svg', 'controls...', (0, 1425, 252, 77), (0, 6, 252, 77), '9bf25c2de36183aeb452d1600eba77348f06b8d76db3df060a158007e9a1829d', ('options/controls.svg',)), ('load-save.svg', 'load/save...', (592, 878, 288, 81), (0, 6, 288, 81), 'd9ffc7c08fc224d9f9a4f715d0a5318cc58f0b7738715a1249a59f58fe8aad90', ('options/load-save.svg',)), ('graphics.svg', 'graphics...', (0, 1327, 256, 96), (0, 6, 256, 96), 'c6b0e743848bc3924ed0461057d9d73fbe7d1ec96df772b83c63bc41e4824a90', ('options/graphics.svg',)), ('credits.svg', 'credits...', (806, 1296, 216, 76), (0, 6, 216, 76), 'f21cab19ed63027976831ef14b35e1ade2a19184acc4bed6ee0c32b79d2588e7', ('options/credits.svg',)))


def _integer(value, low, high):
    return type(value) is int and low <= value <= high


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate Remaining menu text-art JSON key")
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
        raise ValueError("Remaining menu text-art schema/scope changed")
    if type(document["words"]) is not list or len(document["words"]) != 50:
        raise ValueError("Remaining menu text-art inventory changed")
    result = []
    for word, expected in zip(document["words"], _identities()):
        if type(word) is not dict or set(word) != set(expected) | {"alphaRuns"}:
            raise ValueError("Remaining menu text-art fields changed")
        if {key:word[key] for key in expected} != expected:
            raise ValueError("Remaining menu text-art identity/order/geometry/aliases changed")
        for key in ("atlas", "sourceCrop"):
            if type(word[key]) is not list or any(type(v) is not int for v in word[key]):
                raise ValueError("Remaining menu text-art noninteger geometry")
        _, _, width, height = expected["atlas"]
        count, runs = width * height, word["alphaRuns"]
        if type(runs) is not list or not runs or len(runs) % 2 or len(runs) > count * 2:
            raise ValueError("Remaining menu text-art unbounded/odd runs")
        alpha = bytearray()
        for i in range(0, len(runs), 2):
            length, value = runs[i:i+2]
            if not _integer(length, 1, 65535) or not _integer(value, 0, 255) or length > count - len(alpha):
                raise ValueError("Remaining menu text-art run overflow/value")
            alpha.extend(bytes((value,)) * length)
        if len(alpha) != count or not any(alpha):
            raise ValueError("Remaining menu text-art decoded length/ink changed")
        rgba = bytearray(b"\xff" * (count * 4))
        rgba[3::4] = alpha
        if hashlib.sha256(rgba).hexdigest() != expected["rgbaSha256"]:
            raise ValueError("Remaining menu text-art decoded digest changed")
        result.append(bytes(rgba))
    return tuple(result)


def _load(source_dir=None):
    directory = Path(ART_DIR if source_dir is None else source_dir)
    raw_parts = []
    # Authenticate every bounded part before decoding any image or writing pixels.
    for name, size, sha in SOURCE_FILES:
        with (directory / name).open("rb") as stream:
            raw = stream.read(size + 1)
        if len(raw) != size or hashlib.sha256(raw).hexdigest() != sha:
            raise ValueError("Remaining menu text-art source hash/length changed")
        raw_parts.append(raw)
    documents = []
    for index, raw in enumerate(raw_parts):
        try:
            document = json.loads(raw, object_pairs_hook=_pairs)
        except (UnicodeError, json.JSONDecodeError) as error:
            raise ValueError("Invalid Remaining menu text-art JSON") from error
        if (type(document) is not dict or set(document) != {"schema", "scope", "typeface", "geometryChanged", "words", "part"}
                or type(document["part"]) is not int or document["part"] != index
                or type(document["words"]) is not list or len(document["words"]) != PART_COUNTS[index]):
            raise ValueError("Remaining menu text-art part inventory/order changed")
        documents.append(document)
    merged = {key:value for key, value in documents[0].items() if key != "part"}
    if any(documents[0][key] != part[key] for part in documents[1:]
           for key in ("schema", "scope", "typeface", "geometryChanged")):
        raise ValueError("Remaining menu text-art part scopes differ")
    merged["words"] = [word for part in documents for word in part["words"]]
    return _decode(merged)


def _overlap(a, b):
    x, y, w, h = a
    xx, yy, ww, hh = b
    return x < xx + ww and xx < x + w and y < yy + hh and yy < y + h


def _placements(families):
    if type(families) is not dict or set(families) != set(FAMILIES):
        raise ValueError("Remaining menu text-art requires all six final placement families")
    observed = []
    for file, text, atlas, crop, sha, aliases in WORDS:
        if not (0 <= atlas[0] < atlas[0]+atlas[2] <= ATLAS_WIDTH and 0 <= atlas[1] < atlas[1]+atlas[3] <= ATLAS_HEIGHT):
            raise ValueError("Remaining menu text-art fixed rectangle out of bounds")
        if any(_overlap(atlas, prior["atlas"]) for prior in observed):
            raise ValueError("Remaining menu text-art fixed rectangles overlap")
        observed.append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop), sharedUses=[]))
    seen = set()
    for family in FAMILIES:
        entries = families[family]
        if type(entries) is not list or not 1 <= len(entries) <= 128:
            raise ValueError("Remaining menu text-art unbounded placement family")
        for entry in entries:
            if type(entry) is not dict or type(entry.get("file")) is not str:
                raise ValueError("Invalid Remaining menu text-art placement")
            name = family + "/" + entry["file"]
            if name in seen:
                raise ValueError("Duplicate Remaining menu text-art family placement")
            seen.add(name)
            atlas, crop = entry.get("atlas"), entry.get("sourceCrop")
            if (type(atlas) is not list or len(atlas) != 4 or not all(_integer(v, 0, 2048) for v in atlas)
                    or not atlas[2] or not atlas[3] or atlas[0]+atlas[2] > ATLAS_WIDTH or atlas[1]+atlas[3] > ATLAS_HEIGHT
                    or type(crop) is not list or len(crop) != 4 or not all(_integer(v, 0, 2048) for v in crop)
                    or crop[0] != 0 or crop[2:] != atlas[2:]):
                raise ValueError("Remaining menu text-art placement geometry invalid")
            for record, (_, _, fixed, fixed_crop, _, aliases) in zip(observed, WORDS):
                if name in aliases:
                    if atlas != list(fixed) or crop != list(fixed_crop):
                        raise ValueError("Remaining menu text-art alias geometry changed")
                    record["sharedUses"].append(name)
                elif _overlap(atlas, fixed):
                    raise ValueError("Remaining menu text-art write overlaps unapproved placement")
    for record, (_, _, _, _, _, aliases) in zip(observed, WORDS):
        if record["sharedUses"] != list(aliases):
            raise ValueError("Remaining menu text-art missing/reordered/unapproved shared uses")
    return observed


def apply_remaining_word_art(pixels, family_placements, *, source_dir=None):
    """Replace fifty fixed allocations after packing and the earlier finite overrides."""
    if type(pixels) is not bytearray or len(pixels) != ATLAS_WIDTH * ATLAS_HEIGHT * 4:
        raise ValueError("Remaining menu text-art needs the existing 1024x2048 RGBA bytearray")
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


def verify_remaining_word_art(metadata, family_placements, *, source_dir=None):
    """Verify selected-project provenance and exact aliases during layout-only work."""
    _load(source_dir)
    checked = _placements(family_placements)
    if (type(metadata) is not dict or set(metadata) != {"sourceHashes", "newRgba", "placements", "changedPixels"}
            or metadata["sourceHashes"] != SOURCE_HASHES or metadata["newRgba"] != _identities()
            or metadata["placements"] != checked
            or not _integer(metadata["changedPixels"], 0, sum(w[2][2]*w[2][3] for w in WORDS))):
        raise ValueError("Remaining menu text-art metadata changed")
