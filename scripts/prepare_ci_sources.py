"""Prepare opt-in classic CI from explicitly reviewed, already tracked art only."""
from pathlib import Path
import hashlib,json,os,re,subprocess,sys
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

def main():
    value=os.environ.get('NFS_CLASSIC_UI_REQUESTED','false')
    if value not in {'false','true'}:raise ValueError('Classic UI input must be boolean')
    classic=value=='true';fingerprint=''
    if classic and os.environ.get('NFS_PUBLISH_RELEASE_REQUESTED','false')=='true':
        raise ValueError('Classic public base requires personal artwork and cannot publish a finished public release')
    if classic:
        if os.environ.get('NFS_BUILD_CONFIGURATION')!='Release':raise ValueError('Classic UI requires an explicit Release configuration')
        if not re.fullmatch('[0-9a-fA-F]{64}',os.environ.get('NFS_HD_ARTWORK_SHA256','')):raise ValueError('Classic UI requires the reviewed artwork fingerprint')
        fingerprint=approval()
    if sys.argv[1:]==['--check-only']:return
    if sys.argv[1:]:raise ValueError('Only --check-only is accepted')
    engine='.build/classic-fullmapped'if classic else'.build/fullmapped'
    renderer='.build/classic-renderer'if classic else'.build/renderer'
    commands=[[sys.executable,'scripts/prepare_full_mapped.py','--checked-copy-bundle','--output-dir',engine],[sys.executable,'scripts/prepare_renderer.py']]
    if classic:
        commands[0]+=['--classic-screens','--classic-assets-dir',ASSETS.as_posix()]
        commands[1]+=['--classic-screens','--classic-assets-dir',ASSETS.as_posix(),'--output-dir',renderer]
    for command in commands:subprocess.run(command,cwd=ROOT,check=True)
    values={'NFS_CLASSIC_UI':'ON'if classic else'OFF','NFS_CLASSIC_DEFER_OWNER_TEMPLATE':'ON'if classic else'OFF','NFS_CI_ENGINE_SOURCE':engine,'NFS_CI_RENDERER_SOURCE':renderer,'NFS_CLASSIC_ASSET_MANIFEST_SHA256':fingerprint}
    target=os.environ.get('GITHUB_ENV')
    if target:
        with open(target,'a',encoding='utf-8')as out:
            for k,v in values.items():out.write(k+'='+v+chr(10))
    else:print(json.dumps(values,sort_keys=True))
if __name__=='__main__':main()
