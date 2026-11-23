"""Bounded, metadata-only pin for optional local slideshow side-strip delivery."""
import json
import re
from pathlib import Path

CATALOG = Path('src/hd/ClassicScreens/SlideshowPanoramaRelease.json')
MAX_BYTES = 32 + 216 * 98 + 256 * 1024 * 1024


def load(path):
    path = Path(path)
    if not path.exists():
        return None
    if path.is_symlink() or not path.is_file() or path.stat().st_size > 4096:
        raise ValueError('Invalid slideshow panorama release metadata file')
    value = json.loads(path.read_bytes())
    if (set(value) != {'schema', 'filename', 'fileBytes', 'sha256', 'imageCount'}
            or type(value['schema']) is not int or value['schema'] != 1
            or value['filename'] != 'slideshow-panorama.ownedz'
            or type(value['fileBytes']) is not int or not 32 < value['fileBytes'] <= MAX_BYTES
            or type(value['imageCount']) is not int or value['imageCount'] != 98
            or not isinstance(value['sha256'], str)
            or not re.fullmatch('[0-9a-f]{64}', value['sha256'])
            or value['sha256'] == '0' * 64):
        raise ValueError('Invalid slideshow panorama release metadata')
    return value
