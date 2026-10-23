"""Prove finite menu artwork fits preserved original and scaled summary roles."""
# SPDX-License-Identifier: MIT
import hashlib
import json
import math
from pathlib import Path
import re
from frontend_right_labels import right_label_rows

FAMILIES = ("main", "location", "setup", "player", "opponents", "options")
# Original accepted output positions, independent of the new artwork and rows.
OUTPUT_X = {
    'location/track.svg': (343.0, 344.0),
    'location/north-country.svg': (395.0, 396.0),
    'location/laps.svg': (349.0, 350.0),
    'location/lap-two.svg': (394.0, 395.0),
    'location/track-info.svg': (318.0, 319.0),
    'location/track-records.svg': (296.0, 297.0),
    'location/personal-statistics.svg': (264.0, 265.0),
    'location/pacific-spirit.svg': (395.0, 396.0),
    'location/mediterraneo.svg': (395.0, 396.0),
    'location/mystic-peaks.svg': (395.0, 396.0),
    'location/proving-grounds.svg': (395.0, 396.0),
    'location/outback.svg': (395.0, 396.0),
    'location/last-resort.svg': (395.0, 396.0),
    'location/lap-4.svg': (394.0, 395.0),
    'location/lap-8.svg': (394.0, 395.0),
    'setup/game-type.svg': (285.0, 286.0),
    'setup/race-type.svg': (292.0, 293.0),
    'setup/style.svg': (319.0, 320.0),
    'setup/catch-up.svg': (297.0, 298.0),
    'setup/backwards.svg': (284.0, 285.0),
    'setup/mirrored.svg': (296.0, 297.0),
    'setup/one-player.svg': (370.0, 371.0),
    'setup/split-screen.svg': (370.0, 371.0),
    'setup/modem.svg': (370.0, 371.0),
    'setup/serial-link.svg': (370.0, 371.0),
    'setup/network.svg': (370.0, 371.0),
    'setup/single-race.svg': (370.0, 371.0),
    'setup/tournament.svg': (370.0, 371.0),
    'setup/knockout.svg': (370.0, 371.0),
    'setup/wild.svg': (370.0, 371.0),
    'setup/arcade.svg': (370.0, 371.0),
    'setup/simulation.svg': (370.0, 371.0),
    'opponents/number.svg': (298.0, 297.0),
    'opponents/skill-level.svg': (288.0, 287.0),
    'opponents/traffic.svg': (307.0, 306.0),
    'opponents/no-opponents.svg': (366.0, 365.0),
    'opponents/full-grid.svg': (366.0, 365.0),
    'opponents/none.svg': (366.0, 365.0),
    'opponents/one.svg': (366.0, 365.0),
    'opponents/beginner.svg': (366.0, 365.0),
    'opponents/advanced.svg': (366.0, 365.0),
    'options/speedometer.svg': (264.0, 263.0),
    'options/mph.svg': (366.0, 365.0),
    'options/kmh.svg': (365.5, 364.5),
    'options/personal-stats.svg': (259.0, 258.0),
    'options/audio.svg': (312.0, 313.0),
    'options/controls.svg': (295.0, 296.0),
    'options/load-save.svg': (286.0, 287.0),
    'options/graphics.svg': (294.0, 295.0),
    'options/credits.svg': (304.0, 305.0),
}
SUMMARY_CATALOG_SHA256 = "78ca798c2819fd98b1b69d8b867f202c6d40f739bff941226c97806e05502cb3"
ROW = re.compile(r"^\{([^{}]+),UINT64_C\(0x([0-9a-f]{16})\)(?:,([^{}]+))?\}$")


def _identity(text):
    value = 14695981039346656037
    for byte in text.encode("latin1"):
        value = ((value ^ byte) * 1099511628211) & 0xffffffffffffffff
    return value


def _parse(row, summary=False):
    if type(row) is not str:
        raise ValueError("Invalid remaining artwork layout row")
    match = ROW.fullmatch(row.rstrip(","))
    if not match:
        raise ValueError("Invalid remaining artwork layout row")
    try:
        values = list(map(float, match[1].split(",")))
        scales = None if match[3] is None else list(map(float, match[3].split(",")))
    except ValueError as error:
        raise ValueError("Non-numeric remaining artwork layout") from error
    if (len(values) != 13 or not all(math.isfinite(v) for v in values)
            or (summary and (scales is None or len(scales) != 2
                             or not all(math.isfinite(v) and v > 0 for v in scales)))
            or (not summary and scales is not None)):
        raise ValueError("Invalid remaining artwork layout fields/scales")
    return values, int(match[2], 16), scales


def _same(actual, expected):
    return len(actual) == len(expected) and all(
        math.isclose(a, b, rel_tol=0, abs_tol=1e-10) for a, b in zip(actual, expected))


def _ink(pixels, record):
    x, y, width, height = record[2]
    if not (0 <= x < x+width <= 1024 and 0 <= y < y+height <= 2048):
        raise ValueError("Remaining artwork allocation out of bounds")
    crop = record[3]
    if (len(crop) != 4 or crop[0] != 0 or crop[2:] != (width, height)
            or not 0 <= crop[1] < crop[1]+height <= 204):
        raise ValueError("Remaining artwork crop changed")
    ink = [(xx, yy) for yy in range(height) for xx in range(width)
           if pixels[((y+yy)*1024+x+xx)*4+3]]
    if not ink:
        raise ValueError("Empty remaining artwork")
    low, high = min(xx for xx, _ in ink), max(xx for xx, _ in ink)
    top, bottom = min(yy for _, yy in ink), max(yy for _, yy in ink)
    if not (0 < top <= bottom < height-1):
        raise ValueError("Remaining artwork leaves padded original vertical band")
    return low, high, top, bottom


def remaining_ink_fit(directory, pixels, rows, placements, summary_rows):
    """Check 100 catalog and four scaled Main-summary placements before adoption."""
    from frontend_remaining_word_art import WORDS
    directory = Path(directory)
    if len(pixels) != 8388608 or type(rows) is not list or type(summary_rows) is not list:
        raise ValueError("Incomplete remaining artwork inputs")
    if len(WORDS) != 50:
        raise ValueError("Incomplete remaining artwork inventory")
    allowed = {}
    for record in WORDS:
        if len(record) != 6:
            raise ValueError("Invalid remaining artwork identity")
        for alias in record[-1]:
            if alias in allowed:
                raise ValueError("Duplicate remaining artwork alias")
            allowed[alias] = record
    if set(allowed) != set(OUTPUT_X):
        raise ValueError("Remaining artwork original role inventory changed")
    if set(placements) != {family+"Placements" for family in FAMILIES}:
        raise ValueError("Incomplete remaining artwork placement families")
    parsed = [_parse(row) for row in rows]
    inks = {alias:_ink(pixels, record) for alias, record in allowed.items()}
    index, result, seen = 0, [], set()
    for family in FAMILIES:
        catalog = json.loads((directory/("catalog.json" if family == "main"
                                        else family+"/catalog.json")).read_text())["words"]
        packed = placements[family+"Placements"]
        if len(catalog) != len(packed):
            raise ValueError("Incomplete remaining artwork placement family")
        for word, place in zip(catalog, packed):
            if word["file"] != place["file"]:
                raise ValueError("Remaining artwork placement order changed")
            alias = family+"/"+word["file"]
            groups = [dict(glyphs=word["glyphs"])] if family == "main" else word["placements"]
            for group_index, group in enumerate(groups):
                count = len(group["glyphs"])
                batch = parsed[index:index+count]; index += count
                if alias not in allowed:
                    continue
                seen.add(alias)
                record = allowed[alias]; x, y, width, height = record[2]; crop = record[3]
                if (place["atlas"] != list(record[2]) or place["sourceCrop"] != list(crop)
                        or len(groups) != 2 or len(batch) != count or not count):
                    raise ValueError("Remaining artwork original allocation/group changed")
                left = OUTPUT_X[alias][group_index]
                for (values, token, _), glyph in zip(batch, group["glyphs"]):
                    original = [glyph[key] for key in ("cell","x","y","w","h","left","right")]
                    if (not _same(values[:7], original)
                            or not _same(values[7:], [x/4,y/8,width/4,left,crop[1]/4,height/4])
                            or token != _identity(glyph["drawText"])):
                        raise ValueError("Remaining artwork original layout/context changed")
                low, high, top, bottom = inks[alias]
                if not (group["x"] < left+(low+.5)/4
                        <= left+(high+.5)/4 < group["right"]):
                    raise ValueError("Remaining artwork leaves original horizontal bounds")
                gy = group["y"]
                if not (gy < gy+(crop[1]+top+.5)/4
                        <= gy+(crop[1]+bottom+.5)/4
                        < gy+min(glyph["h"] for glyph in group["glyphs"])):
                    raise ValueError("Remaining artwork leaves original vertical bounds")
                result.append(dict(use=alias, kind="catalog", shadow=group["shadow"],
                                   x=group["x"],y=gy,right=group["right"],
                                   bottom=gy+min(glyph["h"] for glyph in group["glyphs"]),
                                   outsideOriginalBounds=0))
    if index != len(rows) or len(result) != 100 or seen != set(allowed):
        raise ValueError("Incomplete remaining artwork catalog coverage")
    raw_catalog = (directory/"right-labels/catalog.json").read_bytes()
    if hashlib.sha256(raw_catalog).hexdigest() != SUMMARY_CATALOG_SHA256:
        raise ValueError("Original Main-summary source catalog changed")
    catalog = json.loads(raw_catalog)
    expected = right_label_rows(catalog, placements["setupPlacements"], _identity)
    if (len(summary_rows) != 42
            or [row.rstrip(",") for row in summary_rows] != expected):
        raise ValueError("Original Main-summary layout/scales changed")
    summary = [_parse(row, summary=True) for row in summary_rows]
    index = 0
    for word in catalog["words"]:
        alias = "setup/"+word["reuse"]
        record = allowed[alias]; x, y, width, height = record[2]; crop = record[3]
        low, high, top, bottom = inks[alias]; count = len(word["glyphs"])
        for shadow in (0, 1):
            batch = summary[index:index+count]; index += count
            values, _, scales = batch[0]
            left, gy = word["x"]+shadow, word["y"]+shadow
            sx, sy = scales
            if (not _same(values[7:10], [x/4,y/8,width/4])
                    or not _same(values[10:13], [left,crop[1]/4,height/4])
                    or not _same(scales, [word["width"]/(width/4),word["height"]/51])):
                raise ValueError("Main-summary artwork sampling/scale changed")
            if not (left < left+(low+.5)/4*sx
                    <= left+(high+.5)/4*sx < left+word["width"]
                    and gy < gy+(crop[1]+top+.5)/4*sy
                    <= gy+(crop[1]+bottom+.5)/4*sy < gy+word["height"]):
                raise ValueError("Remaining artwork leaves original scaled summary bounds")
            result.append(dict(use="main-summary/"+word["reuse"],kind="summary",shadow=bool(shadow),
                               x=left,y=gy,right=left+word["width"],bottom=gy+word["height"],
                               outsideOriginalBounds=0))
    if index != 42 or len(result) != 104:
        raise ValueError("Incomplete remaining artwork summary coverage")
    return dict(images=50,catalogUses=50,catalogPlacements=100,
                summaryUses=2,summaryPlacements=4,placements=result)
