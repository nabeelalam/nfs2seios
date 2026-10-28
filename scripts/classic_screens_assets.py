"""Explicit private-input boundary for opt-in classic-screen generation."""
from pathlib import Path
import json, shutil

def require_assets(root, directory):
    if directory is None: raise ValueError('--classic-screens requires --classic-assets-dir')
    directory=Path(directory).resolve()
    names=json.loads((Path(root)/'src/hd/ClassicScreens/PRIVATE_ASSETS.json').read_text())['requiredHeaders']
    if any(Path(n).name!=n or not n.endswith('Data.h') for n in names):raise ValueError('Invalid classic asset inventory')
    missing=[n for n in names if not (directory/n).is_file()]
    if missing:raise ValueError('Missing explicit private classic assets: '+', '.join(missing))
    return directory,names

def stage_assets(root,directory,destination):
    directory,names=require_assets(root,directory)
    target=Path(destination)/'hd/ClassicScreens/private';target.mkdir(parents=True,exist_ok=True)
    for n in names:shutil.copyfile(directory/n,target/n)
    (target/'.gitignore').write_text('*\n')
