"""Install a digest-pinned local scanner, without changing system tools."""
import hashlib
import io
import platform
import tarfile
import urllib.request
import zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
VERSION="8.30.1"
RELEASES={
    "Windows": ("windows_x64.zip","d29144deff3a68aa93ced33dddf84b7fdc26070add4aa0f4513094c8332afc4e","gitleaks.exe"),
    "Linux": ("linux_x64.tar.gz","551f6fc83ea457d62a0d98237cbad105af8d557003051f41f3e7ca7b3f2470eb","gitleaks"),
}
def main():
    suffix,digest,name=RELEASES[platform.system()]
    if platform.machine().lower() not in {"amd64","x86_64"}:raise RuntimeError("Unreviewed scanner architecture")
    url=f"https://github.com/gitleaks/gitleaks/releases/download/v{VERSION}/gitleaks_{VERSION}_{suffix}"
    with urllib.request.urlopen(url,timeout=60) as response:data=response.read(64*1024*1024+1)
    if len(data)>64*1024*1024 or hashlib.sha256(data).hexdigest()!=digest:raise RuntimeError("Scanner release digest mismatch")
    if suffix.endswith("zip"):
        with zipfile.ZipFile(io.BytesIO(data)) as archive:binary=archive.read(name)
    else:
        with tarfile.open(fileobj=io.BytesIO(data),mode="r:gz") as archive:
            entry=archive.getmember(name)
            if not entry.isfile():raise RuntimeError("Unexpected scanner archive entry")
            binary=archive.extractfile(entry).read()
    dest=ROOT/".tools/gitleaks"/name;dest.parent.mkdir(parents=True,exist_ok=True)
    dest.write_bytes(binary);dest.chmod(0o755)
    print(f"Verified Gitleaks {VERSION}; project-local install")
if __name__=="__main__":main()
