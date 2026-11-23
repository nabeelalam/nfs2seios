"""Prepare opt-in classic CI from explicitly reviewed, already tracked art only."""
from pathlib import Path
import hashlib,json,os,re,subprocess,sys
from validate_artwork_sha256 import validate
from slideshow_panorama_release import CATALOG as PANORAMA_CATALOG, load as load_panorama_release
ROOT=Path(__file__).resolve().parents[1]
ASSETS=Path('assets/hud/classic-screens')
MANIFEST=ASSETS/'APPROVED_ASSETS.json'
ALLOWED_LICENSES={'OFL-1.1','CC0-1.0','CC-BY-4.0','LicenseRef-Project-Authored-Art'}

def tracked(relative):
    f=ROOT/relative
    if f.is_symlink() or not f.is_file() or ROOT.resolve() not in f.resolve().parents:
        raise ValueError('Reviewed classic input must be a regular in-repository file')
    subprocess.run(['git','ls-files','--error-unmatch','--',str(relative).replace('\\','/')],cwd=ROOT,check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    return f

def approval():
    inventory=json.loads((ROOT/'src/hd/ClassicScreens/PRIVATE_ASSETS.json').read_text())['requiredHeaders']
    if len(inventory)!=9 or len(set(inventory))!=9 or any(Path(n).name!=n or not n.endswith('Data.h')for n in inventory):
        raise ValueError('Unexpected reviewed classic header inventory')
    file=tracked(MANIFEST);data=file.read_bytes()
    if len(data)>65536:raise ValueError('Classic approval manifest is oversized')
    manifest=json.loads(data)
    if manifest.get('schema')!=1 or manifest.get('redistributionApproved')is not True or set(manifest.get('headers',{}))!=set(inventory):
        raise ValueError('Explicit public redistribution approval is required for all nine headers')
    for name in inventory:
        row=manifest['headers'][name];expected=row.get('sha256','')
        if not re.fullmatch('[0-9a-f]{64}',expected)or row.get('license')not in ALLOWED_LICENSES:
            raise ValueError('Invalid classic header approval or license identifier')
        notice=Path(row.get('notice',''))
        if notice.is_absolute()or '..'in notice.parts or notice.suffix not in {'.md','.txt'} or not (notice.is_relative_to(ASSETS)or notice.is_relative_to(Path('assets/licenses/classic-ui'))):
            raise ValueError('Classic license notice must be a tracked Markdown file in the reviewed asset directory')
        tracked(notice)
        if hashlib.sha256(tracked(ASSETS/name).read_bytes()).hexdigest()!=expected:
            raise ValueError('Reviewed classic header SHA mismatch: '+name)
    return hashlib.sha256(data).hexdigest()


# This pair is code metadata, separately approved from the nine drawable headers.
SLIDESHOW_METADATA=ASSETS/'SLIDESHOW_METADATA.json'
SLIDESHOW_HEADERS={'SlideshowCatalogData.h':'source-and-runtime-image-pins',
                   'SlideshowFooterData.h':'footer-resource-pins-and-bindings'}
SLIDESHOW_NOTICE=ASSETS/'SLIDESHOW_METADATA_NOTICE.md'


from slideshow_metadata import (_metadata_initializer, _metadata_digest, _metadata_uint,
                                _slideshow_structure, approved_metadata)

def metadata_approval():
    return approved_metadata(ROOT, tracked)[0]


def main():
    value=os.environ.get('NFS_CLASSIC_UI_REQUESTED','false')
    if value not in {'false','true'}:raise ValueError('Classic UI input must be boolean')
    classic=value=='true';fingerprint='';metadata_fingerprint='';panorama=None
    adaptive = os.environ.get('NFS_ADAPTIVE_RACE_REQUESTED', 'false')
    if adaptive not in {'false', 'true'}:
        raise ValueError('Adaptive race input must be boolean')
    if adaptive == 'true' and not classic:
        raise ValueError('Adaptive race requires the classic prepared sources')
    if classic and os.environ.get('NFS_PUBLISH_RELEASE_REQUESTED','false')=='true':
        raise ValueError('Classic public base requires personal artwork and cannot publish a finished public release')
    if classic:
        if os.environ.get('NFS_BUILD_CONFIGURATION')!='Release':raise ValueError('Classic UI requires an explicit Release configuration')
        validate(os.environ.get('NFS_HD_ARTWORK_SHA256',''),required=True)
        fingerprint=approval()
        metadata_fingerprint=metadata_approval()
        if (ROOT/PANORAMA_CATALOG).exists():
            panorama=load_panorama_release(tracked(PANORAMA_CATALOG))
    if sys.argv[1:]==['--check-only']:return
    if sys.argv[1:]:raise ValueError('Only --check-only is accepted')
    engine='.build/classic-fullmapped'if classic else'.build/fullmapped'
    renderer='.build/classic-renderer'if classic else'.build/renderer'
    commands=[[sys.executable,'scripts/prepare_full_mapped.py','--checked-copy-bundle','--output-dir',engine],[sys.executable,'scripts/prepare_renderer.py']]
    if classic:
        commands[0]+=['--classic-screens','--classic-assets-dir',ASSETS.as_posix()]
        commands[1]+=['--classic-screens','--classic-assets-dir',ASSETS.as_posix(),'--output-dir',renderer]
    for command in commands:subprocess.run(command,cwd=ROOT,check=True)
    if adaptive == 'true':
        from prepare_adaptive_race import prepare

        pair = Path('.build/adaptive-race')
        prepare(ROOT / engine, ROOT / renderer, ROOT / pair)
        engine = (pair / 'engine').as_posix()
        renderer = (pair / 'renderer').as_posix()
    values={'NFS_CLASSIC_UI':'ON'if classic else'OFF','NFS_CLASSIC_DEFER_OWNER_TEMPLATE':'ON'if classic else'OFF','NFS_CI_ENGINE_SOURCE':engine,'NFS_CI_RENDERER_SOURCE':renderer,'NFS_CLASSIC_ASSET_MANIFEST_SHA256':fingerprint,'NFS_CLASSIC_SLIDESHOW_METADATA_SHA256':metadata_fingerprint}
    values.update(NFS_CLASSIC_SLIDESHOW_PANORAMA_SHA256=panorama['sha256']if panorama else'',
                  NFS_CLASSIC_SLIDESHOW_PANORAMA_BYTES=str(panorama['fileBytes'])if panorama else'0')
    target=os.environ.get('GITHUB_ENV')
    if target:
        with open(target,'a',encoding='utf-8')as out:
            for k,v in values.items():out.write(k+'='+v+chr(10))
    else:print(json.dumps(values,sort_keys=True))
if __name__=='__main__':main()
