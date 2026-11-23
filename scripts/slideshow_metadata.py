"""Bounded approved slideshow metadata and ARM64 catalog serialization."""
from pathlib import Path
import hashlib,json,re,struct
ASSETS=Path('assets/hud/classic-screens')
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


def _slideshow_structure(catalog,footer,approved_counts=None, *, return_tables=False):
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
    counts={'sourcePins':len(sources),'runtimeImagePins':len(images),'resourcePins':len(resources),'bindings':len(bindings)}
    if (approved_counts is None or set(approved_counts)!=set(counts)
            or any(type(x)is not int for x in approved_counts.values())
            or approved_counts['sourcePins']!=98 or approved_counts['runtimeImagePins']!=98
            or approved_counts['bindings']!=98 or not 1<=approved_counts['resourcePins']<=98
            or counts!=approved_counts):
        raise ValueError('Unexpected slideshow metadata table counts')
    sourceKeys=[]
    for row in sources:
        if not isinstance(row,list)or len(row)!=8:raise ValueError('Invalid SourcePin metadata')
        size,offset,span,fmt,w,h=(_metadata_uint(x)for x in row[:6])
        if not 16<=size<=1024*1024 or offset<16 or span!=16+w*h*2 or offset>size or span>size-offset or fmt not in {120,126}or(w,h)!=(640,480):
            raise ValueError('Invalid original source layout metadata')
        sourceKeys.append((_metadata_digest(row[6]),_metadata_digest(row[7])))
    if len(set(sourceKeys))!=98:raise ValueError('Duplicate slideshow source fingerprints')
    previous=16;fileSize=None;runtimeVersion=None;runtimeKeys=[]
    for row in images:
        if not isinstance(row,list)or len(row)!=9:raise ValueError('Invalid RuntimeImagePin metadata')
        size=_metadata_uint(row[0],512*1024*1024);offset=_metadata_uint(row[1],512*1024*1024);span=_metadata_uint(row[2],32*1024*1024)
        if size<16 or offset!=previous or not span or offset>size or span>size-offset or(fileSize is not None and size!=fileSize):
            raise ValueError('Invalid slideshow resource layout metadata')
        fileSize=size;previous=offset+span;runtimeKeys.append((_metadata_digest(row[3]),_metadata_digest(row[4])));_metadata_digest(row[5])
        pixel=row[6]
        if not isinstance(pixel,list)or len(pixel)!=4:raise ValueError('Invalid pixel layout metadata')
        w,h=(_metadata_uint(x,4096)for x in pixel[:2]);_metadata_digest(pixel[2])
        version=row[8]
        if not w or not h or w*3!=h*4 or type(version)is not int or version not in (3,4) or type(pixel[3])is not int or pixel[3]!=version-2 or row[7]is not True or (runtimeVersion is not None and version!=runtimeVersion):
            raise ValueError('Unaccepted slideshow version/layout metadata')
        runtimeVersion=version
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
        bindingKeys.append((_metadata_digest(row[0]),_metadata_digest(row[1])));_metadata_uint(row[2],len(resources)-1)
    if bindingKeys!=sourceKeys:raise ValueError('Footer binding/source fingerprint mismatch')
    return (counts, (sources,images,resources,bindings)) if return_tables else counts


def approved_metadata(root, reader=None):
    root=Path(root)
    if reader is None:
        def reader(relative):
            file=root/relative
            if file.is_symlink() or not file.is_file() or root.resolve() not in file.resolve().parents:
                raise ValueError("Slideshow metadata must be a regular in-repository file")
            return file
    def bounded(relative, limit):
        file=reader(relative)
        if file.stat().st_size>limit:raise ValueError("Slideshow metadata file is oversized")
        with file.open("rb") as stream:data=stream.read(limit+1)
        if len(data)>limit:raise ValueError("Slideshow metadata file is oversized")
        return data
    data=bounded(SLIDESHOW_METADATA,8192)
    if len(data)>8192:raise ValueError('Slideshow metadata approval manifest is oversized')
    manifest=json.loads(data)
    if not isinstance(manifest,dict) or set(manifest)!={'schema','kind','pairRequired','imagePayloads','metadataApproved','headers','counts'}or type(manifest['schema'])is not int or manifest['schema']!=1 or manifest['kind']!='slideshow-compatibility-metadata'or manifest['pairRequired']is not True or manifest['imagePayloads']is not False or manifest['metadataApproved']is not True or not isinstance(manifest['headers'],dict) or set(manifest['headers'])!=set(SLIDESHOW_HEADERS) or not isinstance(manifest['counts'],dict):
        raise ValueError('Separate exact-pair metadata approval is required')
    payloads={}
    for name,kind in SLIDESHOW_HEADERS.items():
        row=manifest['headers'][name]
        if not isinstance(row,dict) or set(row)!={'sha256','kind','license','notice'}or row['kind']!=kind or row['license']!='MIT'or row['notice']!=SLIDESHOW_NOTICE.as_posix()or not isinstance(row['sha256'],str) or not re.fullmatch('[0-9a-f]{64}',row['sha256']):
            raise ValueError('Invalid slideshow metadata approval or license')
        reader(SLIDESHOW_NOTICE)
        payloads[name]=bounded(ASSETS/name,131072)
        if hashlib.sha256(payloads[name]).hexdigest()!=row['sha256']:raise ValueError('Slideshow metadata SHA mismatch: '+name)
    counts=_slideshow_structure(payloads['SlideshowCatalogData.h'],payloads['SlideshowFooterData.h'],manifest['counts'])
    if manifest['counts']!=counts:raise ValueError('Slideshow metadata approval count mismatch')
    return hashlib.sha256(data).hexdigest(), payloads, manifest["counts"]


def compiled_catalogs(root):
    """Authenticate headers and serialize four complete ordered ARM64 tables."""
    _,payloads,counts=approved_metadata(root)
    _,tables=_slideshow_structure(payloads['SlideshowCatalogData.h'],payloads['SlideshowFooterData.h'],counts,return_tables=True)
    sources,images,resources,bindings=tables
    d=_metadata_digest
    return {
        'slideshow:sources': b''.join(struct.pack('<IIIIII32s32s',*r[:6],d(r[6]),d(r[7])) for r in sources),
        'slideshow:runtime-images': b''.join(struct.pack('<QQI32s32s32sII32sI?3xI',*r[:3],*(d(x) for x in r[3:6]),*r[6][:2],d(r[6][2]),r[6][3],r[7],r[8]) for r in images),
        'slideshow:footer-resources': b''.join(struct.pack('<QIII32s32s4x',*r[:4],d(r[4]),d(r[5])) for r in resources),
        'slideshow:bindings': b''.join(struct.pack('<32s32sI',d(r[0]),d(r[1]),r[2]) for r in bindings),
    }
