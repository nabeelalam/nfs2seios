"""Bound upload inputs; account-wide storage must also be reviewed before upload."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MAX_BYTES=64*1024*1024
def main():
    files=list((ROOT/".build").glob("NFSIISE*-unsigned.ipa"))+list((ROOT/".build").glob("NFSIISE*-unsigned.json"))
    if not files:raise RuntimeError("No validated package payload")
    if any(path.is_symlink() for path in files):raise RuntimeError("Unexpected artifact symlink")
    size=sum(path.stat().st_size for path in files)
    if size>MAX_BYTES:raise RuntimeError("Artifact payload exceeds the reviewed 64 MiB run budget; upload refused")
    print(f"Artifact payload: {size} bytes; within 64 MiB run budget")
if __name__=="__main__":main()
