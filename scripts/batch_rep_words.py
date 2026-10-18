"""Opt-in bounded copies for exact loops in reviewed generated methods.

This recognizes the pinned generated dialect, not arbitrary C++. Each word
still uses the normal checked transaction accessors. In particular, this is
not memcpy/memmove: overlapping operands and register aliases retain their
original forward word order. A scope owns at most eight words of work.
"""
import re

TARGETS = {
    "Methods_03.cpp": {"42CFB0": 4},
    "Methods_04.cpp": {"430C70": 3, "4326E0": 3},
    "Methods_07.cpp": {"462020": 2},
}
ORIGINAL_LOOP = "\twhile (ecx) //rep\n\t{\n\t\tmovsd();\n\t\t--ecx;\n\t};"
BOUNDED_LOOP = """\twhile (ecx) //rep
\t{
\t\tnfs::GuestCopyTransaction transaction;
\t\tfor(unsigned copied=0;copied<8 && ecx;++copied){
\t\t\tconst int32_t value=transaction.load<int32_t>(esi);
\t\t\ttransaction.store<int32_t>(edi,value);
\t\t\tesi += 4;edi += 4;--ecx;
\t\t}
\t};"""


def batch_rep_words(text, unit):
    """Return rewritten text and a manifest; reject changed target shapes."""
    manifest = []
    for method, expected in TARGETS.get(unit, {}).items():
        declaration = f"Fn(void) Game::_sub_{method}()\n{{"
        entries = list(re.finditer(re.escape(declaration), text))
        if len(entries) != 1 or (entries[0].start() and text[entries[0].start()-1] != "\n"):
            raise ValueError(f"Expected one bounded-copy method entry: {method}")
        start = entries[0].start()
        end = text.find("\nFn(void) ", start + len(declaration))
        if end < 0:
            end = len(text)
        body = text[start:end]
        if any(token in body for token in ('/*', '*/', '"', "'")):
            raise ValueError(f"Unreviewed bounded-copy method syntax: {method}")
        if body.count(ORIGINAL_LOOP) != expected or BOUNDED_LOOP in body:
            raise ValueError(f"Unexpected bounded-copy loop count or shape: {method}")
        changed = body.replace(ORIGINAL_LOOP, BOUNDED_LOOP)
        text = text[:start] + changed + text[end:]
        manifest.append({"method": method, "loops": expected, "maximumWordsPerScope": 8})
    return text, manifest
