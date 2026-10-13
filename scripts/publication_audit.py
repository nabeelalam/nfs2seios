"""Fail closed on publication hazards; reports never print matched values."""
import argparse
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATTERNS = {
    "personal_directory": re.compile(rb"(?i)([a-z]:[\\/](?:Users|Program Files)[\\/]|/mnt/[a-z]/" + rb"Users/|/(?:home|Users)/[^/\s\"']+/)"),
    "private_key": re.compile(rb"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----"),
    "github_token": re.compile(rb"(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})"),
    "other_token": re.compile(rb"(?:AKIA[A-Z0-9]{16}|xox[baprs]-[A-Za-z0-9-]{20,}|sk-proj-[A-Za-z0-9_-]{30,})"),
}
BLOCKED = re.compile(r"(?i)(^(?:game|gamedata|fedata|\.build|\.tools|candidates|upstream|\.aws|\.codex)/|(?:^|/)(?:gamedata|fedata)/|\.(?:p12|pfx|pem|key|mobileprovision|ipa|exe|dll|iso|bin|viv|qfs|fsh|mus|bnk|raw|wav|zip|log)$|(?:^|/)\.env(?:\.|$))")
ALLOWED_SUFFIXES = {".md", ".json", ".py", ".cpp", ".c", ".h", ".inc", ".mm", ".plist", ".yml", ".yaml"}
SAFE_EMAIL = re.compile(r"(?i)^(?:[^<>\s]+@users\.noreply\.github\.com|codex@(?:localhost|local|invalid)|[^<>\s]+@example\.(?:com|org|net))$")

def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT)

def content_findings(name, data, mode="100644"):
    found = []
    if BLOCKED.search(name): found.append("forbidden_file")
    if mode == "120000": found.append("symlink")
    if Path(name).suffix not in ALLOWED_SUFFIXES and name not in {".gitignore", "CMakeLists.txt", ".githooks/pre-commit", ".githooks/pre-push"}:
        found.append("unreviewed_file_type")
    if len(data) > 1024 * 1024: found.append("large_file_review")
    if b"\0" in data: found.append("binary_file")
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
