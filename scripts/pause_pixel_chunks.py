"""Bounded run-aligned source chunks for the finite pause RGBA region."""
# SPDX-License-Identifier: MIT
import re

PAUSE_PIXEL_PART_BYTES = 65520
PAUSE_PIXEL_MAX_PARTS = 16


def chunked_pause_pixels(encoded):
    """Return the existing RLE array with finite source includes and their bytes."""
    if (type(encoded) not in (bytes, bytearray) or not encoded or len(encoded) % 6
            or len(encoded) > PAUSE_PIXEL_MAX_PARTS * PAUSE_PIXEL_PART_BYTES):
        raise ValueError("Unbounded or unaligned pause pixel runs")
    outputs = {}
    names = []
    for index, start in enumerate(range(0, len(encoded), PAUSE_PIXEL_PART_BYTES)):
        name = "PauseFontPixelsPart" + str(index) + ".inc"
        data = encoded[start:start+PAUSE_PIXEL_PART_BYTES]
        content = "// SPDX-License-Identifier: MIT\n" + ",\n".join(
            ",".join(str(value) for value in data[offset:offset+24])
            for offset in range(0, len(data), 24)) + ",\n"
        if len(content.encode()) >= 1048576:
            raise ValueError("Pause pixel source exceeds publication size gate")
        outputs[name] = content
        names.append(name)
    aggregate = "static const unsigned char nfs_pause_pixels_rle[]={\n" + "".join(
        '#include "' + name + '"\n' for name in names) + "};"
    return aggregate, outputs


def pause_pixel_parts(text):
    """Admit only one contiguous bounded inventory of generated local includes."""
    if type(text) is not str:
        raise ValueError("Invalid pause pixel header")
    parts = re.findall(r'^#include "(PauseFontPixelsPart[0-9]+\.inc)"\r?$', text, re.M)
    includes = re.findall(r'^\s*#\s*include\b[^\n]*$', text, re.M)
    if (not 1 <= len(parts) <= PAUSE_PIXEL_MAX_PARTS or len(parts) != len(includes)
            or parts != ["PauseFontPixelsPart" + str(index) + ".inc" for index in range(len(parts))]):
        raise ValueError("Unexpected pause pixel header inventory")
    return parts
