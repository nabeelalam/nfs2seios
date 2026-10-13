"""Fetch a pinned candidate and prepare disposable source with reviewed fixes."""
from pathlib import Path
import argparse
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PIN = "fde04b571533ba20a60c473ab99d2431bb5cfd60"
URL = "https://github.com/Link4Electronics/NFSIISE.git"

def prepare():
    source = ROOT / "candidates/link4"
    if not source.exists():
        subprocess.run(["git", "clone", URL, str(source)], check=True)
        subprocess.run(["git", "-C", str(source), "checkout", "--detach", PIN], check=True)
    sha = subprocess.check_output(["git", "-C", str(source), "rev-parse", "HEAD"], text=True).strip()
    if sha != PIN:
        raise RuntimeError(f"Expected {PIN}, found {sha}; refusing an unreviewed revision")
    dest = ROOT / ".build/engine"
    # Only overwrite generated source files; never mutate the candidate checkout.
    shutil.copytree(source / "src", dest, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns("Android", "Windows", "Asm"))
    fpu = dest / "Cpp/FPU.h"
    text = fpu.read_text(encoding="utf-8")
    needle = "control.word = 0x037F;\n\t\tstatus.flags = 0x0000;"
    if text.count(needle) != 1:
        raise RuntimeError("FPU initialization patch no longer applies")
    fpu.write_text(text.replace(needle, needle + "\n\t\tstatus.top = 0;"), encoding="utf-8")
    config = dest / "Cpp/config.h"
    config.write_text(config.read_text(encoding="utf-8").replace("#include <byteswap.h>",
        "#include \"ByteSwapPortable.h\""), encoding="utf-8")
    shutil.copyfile(ROOT / "src/ByteSwapPortable.h", dest / "Cpp/ByteSwapPortable.h")
    application=dest / "Cpp/Application.h"
    text=application.read_text(encoding="utf-8")
    begin='\ttemplate<typename T1, typename T2>\n\tFnInl(void) shr('
    end='\ttemplate<typename T>\n\tFnInl(void) neg('
    if text.count(begin)!=1 or text.count(end)!=1:raise RuntimeError("Original shift helper patch no longer applies")
    first=text.index(begin);last=text.index(end,first)
    text=text[:first]+(ROOT / "src/InstructionShifts.inc").read_text(encoding="utf-8")+'\n'+text[last:]
    shrd='val1 = (UNSIGNED(val1) >> cnt) | (val2 << ((sizeof val1 << 3) - cnt));'
    if text.count(shrd)!=1:raise RuntimeError("Original double-shift helper patch no longer applies")
    text=text.replace(shrd,'const unsigned count=uint32_t(cnt)&31;\n\t\tif(!count)return;\n\t\tconst uint64_t combined=uint64_t(UNSIGNED(val1))|(uint64_t(UNSIGNED(val2))<<(sizeof val1*8));\n\t\tUNSIGNED_REF(val1)=combined>>count;')
    application.write_text(text,encoding="utf-8")
    # FNTF is a four-byte binary magic, not a NUL-terminated string. The fork's
    # generated char[5] shifted every following binary font header word by one.
    data=dest / "Cpp/DATA.h"
    text=data.read_text(encoding="utf-8")
    font="struct packed {char x00[5];int32_t x01[539];} dword_4DF380;"
    if text.count(font)!=1:raise RuntimeError("Embedded font layout patch no longer applies")
    data.write_text(text.replace(font,font.replace("x00[5]","x00[4]")),encoding="utf-8")
    memory=dest / "Cpp/Memory.cpp"
    text=memory.read_text(encoding="utf-8")
    if text.count('{"FNTF",0x86D,')!=1:raise RuntimeError("Embedded font magic patch no longer applies")
    memory.write_text(text.replace('{"FNTF",0x86D,',"{{'F','N','T','F'},0x86D,"),encoding="utf-8")
    print(f"Prepared {sha} in {dest}")
    return dest

if __name__ == "__main__":
    prepare()
