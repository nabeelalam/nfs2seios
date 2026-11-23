"""Cross-language pack writer and actual lazy-file-loader integration."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--decoder", required=True, type=Path)
    ap.add_argument("--work-root", required=True, type=Path)
    args = ap.parse_args()
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="history-panorama-fixture-", dir=args.work_root.resolve()) as temporary:
        work = Path(temporary)
        rgba = bytes(v for y in range(72) for x in range(128) for v in (x % 251, y % 251, (x ^ y) % 251, 255))
        source = work / "synthetic.rgba"
        source.write_bytes(rgba)
        pins = []
        for index in (0, 7):
            target = work / (str(index) + ".ownedz")
            command = [sys.executable, str(root / "scripts/history_panorama_pack.py"), "--rgba", str(source), "--width", "128", "--height", "72", "--source-index", str(index), "--output", str(target)]
            result = subprocess.run(command, check=True, capture_output=True, text=True)
            receipt = json.loads(result.stdout)
            assert receipt["losslessVerified"] and receipt["rgbaSHA256"] == hashlib.sha256(rgba).hexdigest()
            assert receipt["fileSHA256"] == hashlib.sha256(target.read_bytes()).hexdigest()
            repeated = subprocess.run(command, capture_output=True)
            assert repeated.returncode and receipt["fileSHA256"] == hashlib.sha256(target.read_bytes()).hexdigest()
            pins.extend((str(target), receipt["fileSHA256"]))
        subprocess.run([str(args.decoder.resolve()), *pins], check=True)
    print("History panorama Python writer / C++ lazy-file integration passed")


if __name__ == "__main__":
    main()
