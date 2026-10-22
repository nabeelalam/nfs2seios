"""Asserted original-preserving hooks for the exact optional Location caption.

No original source, asset pixels, content keys or private paths are embedded.
The producer remains disabled without configured artwork and an owned font.
"""
import re

MARKER = "NFS_LOCATION_CAPTION"
ENTRIES = {
    "Methods_01.cpp": {"40C160": "OwnerScope", "40C110": "TailScope", "405CF0": "StringScope"},
    "Methods_09.cpp": {"4824CC": "SelectionScope"},
    "Methods_10.cpp": {"482DA0": "retire-descriptor"},
    "Methods_13.cpp": {"4A5830": "GlyphScope"},
    "Methods_14.cpp": {"4AC934": "retire-selected", "4AC9C8": "retire-descriptor"},
}


def add_location_caption_hooks(text, name):
    if name not in ENTRIES:
        return text
    if MARKER in text:
        raise ValueError("Location caption hooks already present")

    def block(tag, code):
        return "\n// " + MARKER + "_BEGIN:" + tag + "\n" + code + "\n// " + MARKER + "_END:" + tag + "\n"

    anchor = '#include "FunctionIds.h"\n'
    if text.count(anchor) != 1:
        raise ValueError("Location caption include anchor changed: " + name)
    text = text.replace(anchor, anchor + block("include", '#include "hd/LocationCaptionGameHooks.inc"'), 1)
    for symbol, role in ENTRIES[name].items():
        anchor = "Fn(void) Game::_sub_" + symbol + "()\n{\n"
        if text.count(anchor) != 1:
            raise ValueError("Location caption entry changed: " + symbol)
        if role == "retire-descriptor":
            code = " nfs_location_caption::beforeDescriptorRetire(*this,uint32_t(eax));"
        elif role == "retire" or role == "retire-selected":
            code = " nfs_location_caption::beforeOwnerRetire(*this,uint32_t(eax)" + (",true" if role == "retire-selected" else "") + ");"
        else:
            code = " nfs_location_caption::" + role + " nfs_location_" + symbol.lower() + "(*this);"
        text = text.replace(anchor, anchor + block("entry-" + symbol, code), 1)
    if name in ("Methods_13.cpp", "Methods_14.cpp"):
        symbol, count = ("4A5830", 4) if name == "Methods_13.cpp" else ("4B3EE0", 1)
        pattern = r"Fn\(void\) Game::_sub_" + symbol + r"\(\)\n\{.*?(?=\nFn\(void\) Game::|\Z)"
        match = re.search(pattern, text, re.S)
        if not match:
            raise ValueError("Location glyph body changed: " + symbol)
        old = match.group()
        store = "guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
        if old.count(store) != count:
            raise ValueError("Location original glyph stores changed: " + symbol)
        parts = old.split(store)
        new = parts[0]
        for k, part in enumerate(parts[1:]):
            new += block("word-open-" + str(k), " { nfs_location_caption::WordCapture nfs_location_word(*this,uint32_t(esi),uint16_t(bx));")
            new += store
            new += block("word-close-" + str(k), " }") + part
        text = text[:match.start()] + new + text[match.end():]
    return text
