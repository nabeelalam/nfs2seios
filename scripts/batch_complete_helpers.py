"""Consolidate checked copies in two pinned, loop-free generated helpers.

Whole-method input and output hashes restrict this to the reviewed dialect.
Each original typed access and cancellation checkpoint remains checked. Push
arguments are sampled at their original point; later registers cannot be
cached across guest stores because the CPU itself can be a guest operand.
"""
import hashlib
import re
from prepare_mapped_engine import closing

TARGETS = {
    "Methods_10.cpp": ("___CHP", 8,
        "ae13c17233875f2a5822a98db8f95815c1115b2e096be980143c23d1ffd931e3",
        "ee4180f1e55d42981dec88881033700c0889745329a032fc60c3c561ea2adbe6"),
    "Methods_13.cpp": ("_sub_49EAC8", 29,
        "e0ebd6963035d658985a5dc7bc439902b02e31c102ca694a069d2b9d1969c8b4",
        "45a6cc3313b9fbc7de7f57f35582ce3c9546c4e2ef232f9e389875a2c33fd50d"),
}
BEGIN = "\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction guest_transaction;\n"
END = "\t} // checked copy transaction end\n"


def replace_once(text, before, after):
    if text.count(before) != 1:
        raise ValueError("Unexpected complete-helper statement shape")
    return text.replace(before, after, 1)


def lower_push(register, transaction, first=False):
    # The original first push decrements ESP before its stop/lock boundary.
    return (f"\tconst int32_t saved_{register}={register};\n\tesp -= 4;\n" +
            (f"\tnfs::GuestCopyTransaction {transaction};\n" if first else "") +
            f"\t{transaction}.store<int32_t>(esp,saved_{register});\n")


def rewrite_helper(body, method):
    body = replace_once(body, BEGIN, "")
    body = replace_once(body, END, "")
    if method == "___CHP":
        body = replace_once(body, "\tpush32(eax);\n", lower_push("eax", "transaction", True))
        body = body.replace("guest_transaction.", "transaction.")
        body = replace_once(body, "\tpush32(to32i(esp));\n",
            "\tconst int32_t saved_word=transaction.load<int32_t>(esp);\n"
            "\tesp -= 4;\n\ttransaction.store<int32_t>(esp,saved_word);\n")
        body = replace_once(body, "guest_store<int8_t>(", "transaction.store<int8_t>(")
        for address in ("esp", "esp+4"):
            body = replace_once(body, f"fldcw(to16i({address}));",
                                f"fldcw(transaction.load<int16_t>({address}));")
    else:
        for index, register in enumerate(("ecx", "esi", "edi", "ebp")):
            body = replace_once(body, f"\tpush32({register});\n",
                                lower_push(register, "guest_transaction", index == 0))
        for register in ("ebp", "edi", "esi", "ecx"):
            body = replace_once(body, f"\tpop32({register});\n",
                f"\t{register} = guest_transaction.load<int32_t>(esp);\n\tesp += 4;\n")
    return body


def batch_complete_helpers(text, unit):
    if unit not in TARGETS:
        return text, []
    method, copies, before_hash, after_hash = TARGETS[unit]
    entry = f"Fn(void) Game::{method}()\n{{"
    # Preserve offsets while excluding comments and literals from entry
    # selection. Unrelated generated methods can contain reviewed comments.
    masked = re.sub(r'''//[^\n]*|/\*.*?(?:\*/|\Z)|"(?:\\.|[^"\\])*(?:"|\Z)|'(?:\\.|[^'\\])*(?:'|\Z)''',
                    lambda m: ''.join('\n' if c == '\n' else ' ' for c in m.group()),
                    text, flags=re.S)
    entries = list(re.finditer(r"(?m)^" + re.escape(entry), masked))
    if len(entries) != 1:
        raise ValueError("Expected one complete-helper entry")
    start = entries[0].start()
    end = closing(text, text.index("{", start), "{", "}") + 1
    if text[end:end+1] not in ("", "\n"):
        raise ValueError("Unexpected complete-helper terminator")
    original = text[start:end]
    digest = lambda value: hashlib.sha256(value.encode("utf-8")).hexdigest()
    if digest(original) != before_hash:
        raise ValueError("Complete-helper source differs from reviewed input")
    changed = rewrite_helper(original, method)
    if digest(changed) != after_hash:
        raise ValueError("Complete-helper output differs from validated candidate")
    return text[:start] + changed + text[end:], [{"method": method,
        "maximumCopiesPerScope": copies, "inputSha256": before_hash,
        "outputSha256": after_hash}]
