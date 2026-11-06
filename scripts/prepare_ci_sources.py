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


# This pair is code metadata, separately approved from the nine drawable headers.
SLIDESHOW_METADATA=ASSETS/'SLIDESHOW_METADATA.json'
SLIDESHOW_HEADERS={'SlideshowCatalogData.h':'source-and-runtime-image-pins',
                   'SlideshowFooterData.h':'footer-resource-pins-and-bindings'}
SLIDESHOW_NOTICE=ASSETS/'SLIDESHOW_METADATA_NOTICE.md'


def _metadata_initializer(body):
    """Read only bounded decimal aggregate initializers, never arbitrary C++."""
    if len(body)>131072:raise ValueError('Slideshow metadata initializer is oversized')
    tokens=re.findall(r'\s+|[{},]|true\b|false\b|[0-9]+',body)
    if ''.join(tokens)!=body:raise ValueError('Slideshow metadata contains non-metadata syntax')
    tokens=[t for t in tokens if not t.isspace()]
    if len(tokens)>50000:raise ValueError('Too many slideshow metadata tokens')
    pos=0
    def read(depth=0):
        nonlocal pos
        if depth>8 or pos>=len(tokens):raise ValueError('Malformed slideshow metadata aggregate')
        token=tokens[pos];pos+=1
        if token=='{':
            out=[]
            while pos<len(tokens) and tokens[pos]!='}':
                out.append(read(depth+1))
                if pos<len(tokens) and tokens[pos]==',':pos+=1
                elif pos>=len(tokens) or tokens[pos]!='}':raise ValueError('Malformed metadata separator')
            if pos>=len(tokens):raise ValueError('Unclosed metadata aggregate')
            pos+=1;return out
        if token in {'true','false'}:return token=='true'
        if re.fullmatch('[0-9]+',token) and len(token)<=20:
            value=int(token)
            if value<=0xffffffffffffffff:return value
        raise ValueError('Invalid slideshow metadata scalar')
    result=read()
    if pos!=len(tokens):raise ValueError('Trailing slideshow metadata syntax')
    return result


def _metadata_digest(value):
    if not isinstance(value,list) or len(value)!=1 or not isinstance(value[0],list):
        raise ValueError('Invalid slideshow digest shape')
    raw=value[0]
    if len(raw)!=32 or any(type(x)is not int or not 0<=x<=255 for x in raw) or not any(raw):
        raise ValueError('Invalid slideshow SHA-256 digest')
    return bytes(raw)


def _metadata_uint(value,maximum=0xffffffff):
    if type(value)is not int or not 0<=value<=maximum:raise ValueError('Invalid slideshow integer')
    return value


def _slideshow_structure(catalog,footer):
    """Full-file grammars permit four fixed pin tables, not image/blob arrays."""
    def clean(data):
        if len(data)>131072:raise ValueError('Slideshow metadata header is oversized')
        try:out=data.decode('ascii')
        except UnicodeDecodeError:raise ValueError('Slideshow metadata must be ASCII')from None
        if '\x00'in out or re.search(r'(?i)([a-z]:[\\/]|/mnt/|/(?:home|Users)/|(?:GAMEDATA|FEDATA)/)',out):
            raise ValueError('Slideshow metadata contains a private path')
        prefix='// SPDX-License-Identifier: MIT\n// Slideshow compatibility fingerprints and resource-layout metadata only; no image pixels.\n'
        out=out.replace('\r\n','\n')
        if not out.startswith(prefix)or '//'in out[len(prefix):]or '/*'in out[len(prefix):]:
            raise ValueError('Unexpected slideshow metadata notice or extra comments')
        return out[len(prefix):].strip()
    c=clean(catalog);f=clean(footer)
    cm=re.fullmatch(r'#pragma once\s+#include "hd/SlideshowSourceIdentity.h"\s+#include "hd/SlideshowRuntimeImage.h"\s+namespace nfs_slideshow_private\s*\{\s*static const nfs_slideshow::SourcePin sources\[\]\s*=\s*(\{.*?\});\s*static const nfs_slideshow::RuntimeImagePin images\[\]\s*=\s*(\{.*?\});\s*\}',c,re.S)
    fm=re.fullmatch(r'#pragma once\s+#include "hd/SlideshowFooterResource.h"\s+namespace nfs_slideshow_footer_private\s*\{\s*static constexpr uint64_t fileBytes=([0-9]+);\s*static const nfs_slideshow_footer::ResourcePin images\[\]\s*=\s*(\{.*?\});\s*inline std::vector<nfs_slideshow_footer::Binding> bindings\(\)\s*\{return\s*(\{.*?\});\s*\}\s*\}',f,re.S)
    if not cm or not fm:raise ValueError('Unexpected slideshow metadata header structure')
    sources,images=(_metadata_initializer(x)for x in cm.groups())
    resources,bindings=(_metadata_initializer(x)for x in fm.groups()[1:])
    if tuple(map(len,(sources,images,resources,bindings)))!=(98,98,17,98):
        raise ValueError('Unexpected slideshow metadata table counts')
    sourceKeys=[]
    for row in sources:
        if not isinstance(row,list)or len(row)!=8:raise ValueError('Invalid SourcePin metadata')
        size,offset,span,fmt,w,h=(_metadata_uint(x)for x in row[:6])
        if not 16<=size<=1024*1024 or offset<16 or span!=16+w*h*2 or offset>size or span>size-offset or fmt not in {120,126}or(w,h)!=(640,480):
            raise ValueError('Invalid original source layout metadata')
        sourceKeys.append((_metadata_digest(row[6]),_metadata_digest(row[7])))
    if len(set(sourceKeys))!=98:raise ValueError('Duplicate slideshow source fingerprints')
    previous=16;fileSize=None;runtimeKeys=[]
    for row in images:
        if not isinstance(row,list)or len(row)!=9:raise ValueError('Invalid RuntimeImagePin metadata')
        size=_metadata_uint(row[0],512*1024*1024);offset=_metadata_uint(row[1],512*1024*1024);span=_metadata_uint(row[2],32*1024*1024)
        if size<16 or offset!=previous or not span or offset>size or span>size-offset or(fileSize is not None and size!=fileSize):
            raise ValueError('Invalid slideshow resource layout metadata')
        fileSize=size;previous=offset+span;runtimeKeys.append((_metadata_digest(row[3]),_metadata_digest(row[4])));_metadata_digest(row[5])
        pixel=row[6]
        if not isinstance(pixel,list)or len(pixel)!=4:raise ValueError('Invalid pixel layout metadata')
        w,h=(_metadata_uint(x,4096)for x in pixel[:2]);_metadata_digest(pixel[2])
        if not w or not h or w*3!=h*4 or type(pixel[3])is not int or pixel[3]!=1 or row[7]is not True or type(row[8])is not int or row[8]!=3:
            raise ValueError('Unaccepted slideshow version/layout metadata')
    if previous!=fileSize or runtimeKeys!=sourceKeys:raise ValueError('Slideshow source/resource fingerprint mismatch')
    footerSize=int(fm[1]);previous=16
    if not 16<=footerSize<=256*1024*1024:raise ValueError('Invalid footer resource size')
    for row in resources:
        if not isinstance(row,list)or len(row)!=6:raise ValueError('Invalid ResourcePin metadata')
        offset=_metadata_uint(row[0],footerSize);span=_metadata_uint(row[1],32*1024*1024);w,h=(_metadata_uint(x,4096)for x in row[2:4])
        if offset!=previous or not span or span>footerSize-offset or not w or not h or w*3!=h*4:
            raise ValueError('Invalid footer resource layout metadata')
        previous=offset+span;_metadata_digest(row[4]);_metadata_digest(row[5])
    if previous!=footerSize:raise ValueError('Invalid footer terminal offset')
    bindingKeys=[]
    for row in bindings:
        if not isinstance(row,list)or len(row)!=3:raise ValueError('Invalid Binding metadata')
        bindingKeys.append((_metadata_digest(row[0]),_metadata_digest(row[1])));_metadata_uint(row[2],16)
    if bindingKeys!=sourceKeys:raise ValueError('Footer binding/source fingerprint mismatch')
    return {'sourcePins':98,'runtimeImagePins':98,'resourcePins':17,'bindings':98}


def metadata_approval():
    data=tracked(SLIDESHOW_METADATA).read_bytes()
    if len(data)>8192:raise ValueError('Slideshow metadata approval manifest is oversized')
    manifest=json.loads(data)
    if set(manifest)!={'schema','kind','pairRequired','imagePayloads','metadataApproved','headers','counts'}or manifest['schema']!=1 or manifest['kind']!='slideshow-compatibility-metadata'or manifest['pairRequired']is not True or manifest['imagePayloads']is not False or manifest['metadataApproved']is not True or set(manifest['headers'])!=set(SLIDESHOW_HEADERS):
        raise ValueError('Separate exact-pair metadata approval is required')
    payloads={}
    for name,kind in SLIDESHOW_HEADERS.items():
        row=manifest['headers'][name]
        if set(row)!={'sha256','kind','license','notice'}or row['kind']!=kind or row['license']!='MIT'or row['notice']!=SLIDESHOW_NOTICE.as_posix()or not re.fullmatch('[0-9a-f]{64}',row['sha256']):
            raise ValueError('Invalid slideshow metadata approval or license')
        tracked(SLIDESHOW_NOTICE)
        payloads[name]=tracked(ASSETS/name).read_bytes()
        if hashlib.sha256(payloads[name]).hexdigest()!=row['sha256']:raise ValueError('Slideshow metadata SHA mismatch: '+name)
    counts=_slideshow_structure(payloads['SlideshowCatalogData.h'],payloads['SlideshowFooterData.h'])
    if manifest['counts']!=counts:raise ValueError('Slideshow metadata approval count mismatch')
    return hashlib.sha256(data).hexdigest()


def main():
    value=os.environ.get('NFS_CLASSIC_UI_REQUESTED','false')
    if value not in {'false','true'}:raise ValueError('Classic UI input must be boolean')
    classic=value=='true';fingerprint='';metadata_fingerprint=''
    if classic and os.environ.get('NFS_PUBLISH_RELEASE_REQUESTED','false')=='true':
        raise ValueError('Classic public base requires personal artwork and cannot publish a finished public release')
    if classic:
        if os.environ.get('NFS_BUILD_CONFIGURATION')!='Release':raise ValueError('Classic UI requires an explicit Release configuration')
        if not re.fullmatch('[0-9a-fA-F]{64}',os.environ.get('NFS_HD_ARTWORK_SHA256','')):raise ValueError('Classic UI requires the reviewed artwork fingerprint')
        fingerprint=approval()
        metadata_fingerprint=metadata_approval()
    if sys.argv[1:]==['--check-only']:return
    if sys.argv[1:]:raise ValueError('Only --check-only is accepted')
    engine='.build/classic-fullmapped'if classic else'.build/fullmapped'
    renderer='.build/classic-renderer'if classic else'.build/renderer'
    commands=[[sys.executable,'scripts/prepare_full_mapped.py','--checked-copy-bundle','--output-dir',engine],[sys.executable,'scripts/prepare_renderer.py']]
    if classic:
        commands[0]+=['--classic-screens','--classic-assets-dir',ASSETS.as_posix()]
        commands[1]+=['--classic-screens','--classic-assets-dir',ASSETS.as_posix(),'--output-dir',renderer]
    for command in commands:subprocess.run(command,cwd=ROOT,check=True)
    values={'NFS_CLASSIC_UI':'ON'if classic else'OFF','NFS_CLASSIC_DEFER_OWNER_TEMPLATE':'ON'if classic else'OFF','NFS_CI_ENGINE_SOURCE':engine,'NFS_CI_RENDERER_SOURCE':renderer,'NFS_CLASSIC_ASSET_MANIFEST_SHA256':fingerprint,'NFS_CLASSIC_SLIDESHOW_METADATA_SHA256':metadata_fingerprint}
    target=os.environ.get('GITHUB_ENV')
    if target:
        with open(target,'a',encoding='utf-8')as out:
            for k,v in values.items():out.write(k+'='+v+chr(10))
    else:print(json.dumps(values,sort_keys=True))
if __name__=='__main__':main()
