"""Export only audited source files, without Git metadata or publication."""
import hashlib
import json
import shutil
from pathlib import Path
from publication_audit import ROOT, audit, git, content_findings

def main():
    if not audit("worktree")["passed"]:raise RuntimeError("Publication policy failed; no export created")
    parent=ROOT/".build/publication";index=1
    while (parent/f"candidate-{index:03}").exists():index+=1
    dest=parent/f"candidate-{index:03}";dest.mkdir(parents=True)
    names=sorted(set(git("ls-files","--cached","--others","--exclude-standard","-z").decode().split("\0"))-{ "" })
    manifest={}
    for name in names:
        source=ROOT/name
        if not source.is_file():continue
        data=source.read_bytes()
        if source.is_symlink() or content_findings(name,data):raise RuntimeError("Export changed during audit; publication refused")
        target=dest/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
        manifest[name]=hashlib.sha256(data).hexdigest()
    # Manifest stays beside the candidate, outside the public source tree.
    dest.with_suffix(".json").write_text(json.dumps({"files":manifest,"gitMetadataIncluded":False,"published":False},indent=2)+"\n")
    print(f"Prepared .build/publication/{dest.name}: {len(manifest)} audited source files, no Git metadata")
    return dest
if __name__=="__main__":main()
