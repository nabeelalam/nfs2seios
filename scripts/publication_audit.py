"""Fail closed on publication hazards; reports never print matched values."""
import argparse
import hashlib
import json
import re
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path
from hud_asset_review import reviewed_font_file, reviewed_remaster_svg

ROOT = Path(__file__).resolve().parents[1]
PATTERNS = {
    "personal_directory": re.compile(rb"(?i)([a-z]:[\\/]+(?:Users|Program Files|GPT)[\\/]+|/mnt/[a-z]/" + rb"(?:Users|GPT)/|/(?:home|Users)/[^/\s\"']+/)"),
    "private_key": re.compile(rb"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----"),
    "github_token": re.compile(rb"(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})"),
    "other_token": re.compile(rb"(?:AKIA[A-Z0-9]{16}|xox[baprs]-[A-Za-z0-9-]{20,}|sk-proj-[A-Za-z0-9_-]{30,})"),
}
BLOCKED = re.compile(r"(?i)(^(?:game|gamedata|fedata|\.build|\.tools|candidates|upstream|\.aws|\.codex)/|(?:^|/)(?:gamedata|fedata)/|\.(?:p12|pfx|pem|key|mobileprovision|ipa|exe|dll|iso|bin|viv|qfs|fsh|mus|bnk|raw|wav|zip|log)$|(?:^|/)\.env(?:\.|$))")
ALLOWED_SUFFIXES = {".md", ".json", ".py", ".cpp", ".c", ".h", ".inc", ".mm", ".plist", ".yml", ".yaml", ".cmake"}
# Exact reviewed authored source payloads only; no general size/binary exception.
REVIEWED_LARGE_SOURCE = {'assets/hud/classic-screens/BackdropAllCrestData.h': (8686021, 'd68fb69295ff143302c140808f7fecc45c316d0eb86c7d9d08b0bdc5adc04200'), 'assets/hud/classic-screens/BackdropCrestData.h': (2007984, '0187b221ae34a5c1d13cc52a8d52519b9415989e189cc93758cf4b6e5cabf76d'), 'assets/hud/classic-screens/BackdropLabelData.h': (1186805, 'a60ff659f27151cfc42853709329e35128bdd42a36963f43c1a2d39c60e97e54'), 'assets/hud/classic-screens/ResultsHeadingData.h': (2482711, '877573e62c25f56efd9b1acacf7f3405fcf0c575f99b8250c4b86d147d5e2f24'), 'src/hd/SetupBadgeData.h': (1964698, '4df2e62404a858e899501083c98541401d4cc0ac4464ecc69b92ab6a47336d8d')}

def reviewed_large_source(name, data):
    pin = REVIEWED_LARGE_SOURCE.get(name)
    return bool(pin and len(data) == pin[0] and data.isascii() and hashlib.sha256(data).hexdigest() == pin[1])

SAFE_EMAIL = re.compile(r"(?i)^(?:[^<>\s]+@users\.noreply\.github\.com|codex@(?:localhost|local|invalid)|[^<>\s]+@example\.(?:com|org|net))$")

def reviewed_timing_svg(name, data):
    """Allow only bounded path/gradient HUD glyph sources; no embedded images."""
    if reviewed_remaster_svg(name,data):return True
    match=re.fullmatch(r"assets/hud/(?:(timing|speed)-([0-9])|small-([A-Z])|record-([1-9]|dot|colon))\.svg",name)
    if not match or len(data)>65536:
        return False
    speed=match[1]=="speed"
    small=bool(match[3]);record=bool(match[4])
    gradient="gold" if speed or small or record else "face"
    if any(token in data.lower() for token in (b"<!",b"<?",b"data:",b"href")):
        return False
    try:
        root=ET.fromstring(data)
        namespace="{http://www.w3.org/2000/svg}"
        if root.tag!=namespace+"svg":return False
        expected={"width":"29","height":"24","viewBox":"0 0 29 24"} if speed else {"width":"12","height":"27","viewBox":"0 0 12 27"}
        if small:
            width="4" if match[3]=="I" else "8"
            expected={"width":width,"height":"8","viewBox":"0 0 "+width+" 8"}
        if record:
            width="4" if match[4] in ("1","dot","colon") else "8"
            expected={"width":width,"height":"8","viewBox":"0 0 "+width+" 8"}
        if root.attrib!=expected:return False
        permitted={"defs":set(),"linearGradient":{"id","x1","y1","x2","y2"},
                   "stop":{"offset","stop-color"},
                   "path":{"d","fill","stroke","stroke-width","stroke-linecap","stroke-linejoin"}}
        for node in root.iter():
            if (node.text or "").strip() or (node.tail or "").strip():return False
            if node is root:continue
            if not node.tag.startswith(namespace):return False
            tag=node.tag[len(namespace):]
            if tag not in permitted or set(node.attrib)-permitted[tag]:return False
            for key,value in node.attrib.items():
                if key=="d":
                    if not value or not re.fullmatch(r"[MmZzLlHhVvCcSsQqTtAaEe0-9+.,\s-]+",value):return False
                elif key in ("fill","stroke","stop-color"):
                    if value not in ("none","url(#"+gradient+")") and not re.fullmatch(r"#[0-9a-fA-F]{3}(?:[0-9a-fA-F]{3})?",value):return False
                elif key=="id":
                    if value!=gradient:return False
                elif key=="stroke-linecap":
                    if value not in ("butt","round","square"):return False
                elif key=="stroke-linejoin":
                    if value not in ("miter","round","bevel"):return False
                elif not re.fullmatch(r"[0-9]+(?:\.[0-9]*)?|\.[0-9]+",value) or float(value)>4096:
                    return False
        return True
    except (ET.ParseError,ValueError,OverflowError):return False

def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT)

def content_findings(name, data, mode="100644"):
    found = []
    pinned_font=reviewed_font_file(name,data)
    if BLOCKED.search(name): found.append("forbidden_file")
    if mode == "120000": found.append("symlink")
    if Path(name).suffix==".svg":
        if not reviewed_timing_svg(name,data):found.append("unreviewed_svg")
    elif not pinned_font and Path(name).suffix not in ALLOWED_SUFFIXES and name not in {".gitignore", "CMakeLists.txt", ".githooks/pre-commit", ".githooks/pre-push"}:
        found.append("unreviewed_file_type")
    if len(data) > 1024 * 1024 and not reviewed_large_source(name, data): found.append("large_file_review")
    if b"\0" in data and not pinned_font: found.append("binary_file")
    for category, pattern in PATTERNS.items():
        if pattern.search(data): found.append(category)
    return found

def audit(mode):
    findings=[]; checked=0; commits=0; seen=set()
    def check(name, data, permissions="100644", revision=None):
        nonlocal checked
        checked+=1
        # Even the reported filename could contain a private directory.
        label="<redacted filename>" if PATTERNS["personal_directory"].search(name.encode()) else name
        for category in content_findings(name,data,permissions):
            findings.append({"file":label,"category":category, **({"revision":revision} if revision else {})})
    if mode == "worktree":
        names=git("ls-files","--cached","--others","--exclude-standard","-z").decode().split("\0")
        for name in sorted(set(names)-{""}):
            path=ROOT/name
            if path.is_symlink(): check(name,str(path.readlink()).encode(),"120000")
            elif path.is_file(): check(name,path.read_bytes())
    elif mode == "staged":
        for entry in git("ls-files","--stage","-z").decode().split("\0"):
            if not entry: continue
            metadata,name=entry.split("\t",1); permissions,oid,stage=metadata.split()
            if stage!="0": findings.append({"file":"<index>","category":"unmerged_index"});continue
            check(name,git("cat-file","blob",oid),permissions)
    else:
        for revision in git("rev-list","--all").decode().splitlines():
            commits+=1
            metadata=git("show","-s","--format=%aE%n%cE%n%B",revision)
            for category,pattern in PATTERNS.items():
                if pattern.search(metadata): findings.append({"file":"<commit metadata>","category":category,"revision":revision})
            for email in metadata.decode(errors="replace").splitlines()[:2]:
                if not SAFE_EMAIL.fullmatch(email):
                    findings.append({"file":"<commit metadata>","category":"private_email_review","revision":revision})
            for entry in git("ls-tree","-rz",revision).decode().split("\0"):
                if not entry: continue
                header,name=entry.split("\t",1); permissions,kind,oid=header.split()
                if kind!="blob": findings.append({"file":name,"category":"non_blob_entry"});continue
                key=(oid,name,permissions)
                if key in seen: continue
                seen.add(key);check(name,git("cat-file","blob",oid),permissions,revision)
    # Do not include private values, native absolute paths, or contact addresses.
    return {"scope":mode,"checkedFiles":checked,"checkedCommits":commits,"passed":not findings,"findings":findings}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode",choices=("worktree","staged","history"),default="staged")
    parser.add_argument("--report",type=Path)
    args=parser.parse_args();report=audit(args.mode)
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True)
        args.report.write_text(json.dumps(report,indent=2)+"\n")
    counts={category:sum(item["category"]==category for item in report["findings"]) for category in sorted({item["category"] for item in report["findings"]})}
    print(json.dumps({key:report[key] for key in ("scope","checkedFiles","checkedCommits","passed")} | {"findingCounts":counts}))
    return 0 if report["passed"] else 1

if __name__=="__main__":
    raise SystemExit(main())
