"""Stop before compilation if the ephemeral runner lacks reviewed disk headroom."""
import shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MIN_FREE=6*1024*1024*1024
free=shutil.disk_usage(ROOT).free
print(f"Runner free disk: {free} bytes; required headroom: {MIN_FREE}")
if free<MIN_FREE:raise SystemExit("Runner disk headroom is below policy; no build started")
