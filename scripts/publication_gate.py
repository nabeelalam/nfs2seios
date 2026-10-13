"""Require both publication-policy checks and an independent secret scanner."""
import argparse
import json
import subprocess
from pathlib import Path
from publication_audit import ROOT, audit, git, SAFE_EMAIL

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode",choices=("staged","history"),required=True)
    args=parser.parse_args();report=audit(args.mode)
    if args.mode=="staged":
        for identity in ("GIT_AUTHOR_IDENT","GIT_COMMITTER_IDENT"):
            value=git("var",identity).decode();email=value.split("<",1)[-1].split(">",1)[0]
            if not SAFE_EMAIL.fullmatch(email):report["findings"].append({"file":"<new commit metadata>","category":"private_email_review"})
        report["passed"]=not report["findings"]
    executable=ROOT/".tools/gitleaks"/("gitleaks.exe" if __import__("os").name=="nt" else "gitleaks")
    if not executable.is_file():
        print("Publication blocked: install the pinned independent secret scanner first")
        return 1
    command=[str(executable),"git","--redact=100","--no-banner"]
    command+=( ["--pre-commit","--staged"] if args.mode=="staged" else ["--log-opts=--all"] )
    # Scanner diagnostics may include commit author metadata; print counts only.
    scan=subprocess.run(command+[str(ROOT)],cwd=ROOT,capture_output=True)
    summary={"policyPassed":report["passed"],"independentSecretScanPassed":scan.returncode==0,
        "findingCategories":sorted({item["category"] for item in report["findings"]})}
    print(json.dumps(summary))
    return 0 if report["passed"] and scan.returncode==0 else 1
if __name__=="__main__":raise SystemExit(main())
