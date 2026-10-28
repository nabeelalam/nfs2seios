"""Stage completed classic-screen source with an explicit private artwork boundary.
No compilation, execution, generated-engine copying, or publication is performed.
"""
from pathlib import Path
import argparse, json, re

INCLUDE = re.compile(r'(#include\s+")([^"]+)(")')

def consolidate(seed, roots, destination, private_assets, dependencies=()):
    seed, destination, private_assets = map(lambda p: Path(p).resolve(), (seed, destination, private_assets))
    roots = [Path(p).resolve() for p in roots] + [seed, seed/'queue-overlay', seed/'issuer-overlay'] + [Path(p).resolve() for p in dependencies]
    if destination.exists() or private_assets.exists():
        raise ValueError('Outputs must be new directories; existing work is never overwritten')
    if '.build' not in private_assets.parts:
        raise ValueError('Private artwork output must be inside an ignored .build directory')
    sources = {}
    def resolve(name, current=None):
        # Explicit overlay roots take precedence over checkpoint-local dependencies.
        for root in roots:
            p = root / Path(name).name
            if p.is_file(): return p
        p = current.parent/name if current else seed/name
        return p.resolve() if p.is_file() else None
    queue = list(seed.glob('*.h')) + list(seed.glob('*.inc'))
    queue += [seed/'queue-overlay/LiveQueueAdapter.inc', seed/'issuer-overlay/LiveGameHooks.inc']
    while queue:
        source = queue.pop()
        if not source.is_file(): continue
        source = resolve(source.name, source) or source
        if source.name in sources: continue
        text = source.read_text(encoding='utf-8-sig')
        sources[source.name] = (source, text)
        for match in INCLUDE.finditer(text):
            name = match[2]
            if name.startswith('hd/'): continue
            dependency = resolve(name, source)
            if dependency: queue.append(dependency)
            elif '..' in Path(name).parts:
                raise ValueError('Unresolved experimental dependency: '+name)
    public, private = {}, {}
    for name, (source, text) in sources.items():
        def rewrite(match):
            dependency = Path(match[2]).name
            target = ('hd/ClassicScreens/private/'+dependency) if dependency in sources and dependency.endswith('Data.h') else (dependency if dependency in sources else match[2])
            return match[1] + target + match[3]
        text = INCLUDE.sub(rewrite, text)
        if re.search(r'[A-Za-z]:[\\/]|\.build[\\/]|\.\./hd-', text):
            raise ValueError('Nonportable source needs manual adaptation: '+name)
        (private if name.endswith('Data.h') else public)[name] = text
    destination.mkdir(parents=True)
    private_assets.mkdir(parents=True)
    for group, output in ((public,destination),(private,private_assets)):
        for name,text in sorted(group.items()): (output/name).write_text(text,encoding='utf-8')
    (destination/'PRIVATE_ASSETS.json').write_text(json.dumps({'schema':1,'requiredHeaders':sorted(private),'redistributionApproved':False,'configuration':'Supply these headers only through an explicitly configured ignored private include directory.'},indent=2)+'\n')
    (private_assets/'.gitignore').write_text('*\n')
    return {'sourceFiles':len(public),'privateAssetHeaders':len(private)}

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--seed',type=Path,required=True)
    ap.add_argument('--overlay',type=Path,action='append',default=[])
    ap.add_argument('--dependency',type=Path,action='append',default=[])
    ap.add_argument('--destination',type=Path,required=True)
    ap.add_argument('--private-assets',type=Path,required=True)
    a=ap.parse_args();print(json.dumps(consolidate(a.seed,a.overlay,a.destination,a.private_assets,a.dependency)))
if __name__=='__main__': main()
