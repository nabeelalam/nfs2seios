"""Insertion-only classic screens and loading pipeline for generated engine units.
Applies maintained LoadSave and font-lifetime hooks before the screen insertions.
"""
from pathlib import Path
import argparse, shutil, subprocess, sys

def prepare(source,output):
    source,output=Path(source).resolve(),Path(output).resolve()
    if output.exists(): raise ValueError('Output must be new')
    output.mkdir(parents=True)
    current=output/'loadsave-prepared';current.mkdir()
    from classic_screens.loadsave_hooks import add_loadsave_hooks
    from classic_screens.font_lifetime_hooks import add_font_lifetime_hooks
    for p in source.glob('Methods_*.cpp'):
        text=p.read_text();text=add_loadsave_hooks(text,p.name);text=add_font_lifetime_hooks(text,p.name)
        (current/p.name).write_text(text)
    scripts=Path(__file__).parent/'classic_screens'
    for index,name in enumerate(('records','stats','results','victory','credits','child_dialog','child_material','child_actual','loading')):
        inserted=output/(str(index)+'-'+name+'-inserted')
        option='--output-dir' if name in ('stats','credits','child_dialog','child_material','child_actual') else '--output'
        subprocess.run([sys.executable,str(scripts/(name+'_hooks.py')),'--source-dir',str(current),option,str(inserted)],check=True)
        merged=output/(str(index)+'-'+name+'-merged');merged.mkdir()
        for p in current.glob('Methods_*.cpp'):shutil.copyfile(p,merged/p.name)
        for p in inserted.glob('Methods_*.cpp'):shutil.copyfile(p,merged/p.name)
        current=merged
    return current

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-dir',type=Path,required=True);ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();print(prepare(a.source_dir,a.output))
if __name__=='__main__': main()
