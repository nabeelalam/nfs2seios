"""Assemble explicitly supplied private slideshow photos without loss of pixels.

Version3 stores zlib-compressed PNG row-filter bytes, not PNG files. Dimensions,
codec and raw pixel digests remain pinned in the generated private C++ catalog.
Original images/identities and all generated outputs stay outside public source.
"""
from pathlib import Path
from io import BytesIO
import argparse, hashlib, json, struct, zlib
from PIL import Image


def digest(value):
    raw=bytes.fromhex(value)
    if len(raw)!=32 or not any(raw):raise ValueError("Invalid SHA256")
    return raw


def array(value):
    return "{{"+",".join(str(x)for x in digest(value))+"}}"


def filtered_payload(image):
    buffer=BytesIO();image.save(buffer,format="PNG",compress_level=9)
    png=buffer.getvalue()
    if png[:8]!=b"\x89PNG\r\n\x1a\n":raise ValueError("Encoder signature")
    offset=8;chunks=[];seen_header=False;ended=False
    while offset<len(png):
        if offset+12>len(png):raise ValueError("Encoder chunk boundary")
        size=struct.unpack_from(">I",png,offset)[0];kind=png[offset+4:offset+8]
        if size>32*1024**2 or offset+size+12>len(png):raise ValueError("Encoder chunk size")
        data=png[offset+8:offset+8+size]
        if zlib.crc32(kind+data)!=struct.unpack_from(">I",png,offset+8+size)[0]:raise ValueError("Encoder CRC")
        if kind==b"IHDR":
            if seen_header or data!=struct.pack(">IIBBBBB",image.width,image.height,8,6,0,0,0):raise ValueError("Encoder RGBA layout")
            seen_header=True
        elif kind==b"IDAT":
            if not seen_header or ended:raise ValueError("Encoder IDAT order")
            chunks.append(data)
        elif kind==b"IEND":
            if size or not chunks:raise ValueError("Encoder terminator")
            ended=True
        elif not(kind[0]&32):raise ValueError("Unsupported encoder critical chunk")
        offset+=size+12
        if ended:break
    if not ended or offset!=len(png):raise ValueError("Encoder trailing bytes")
    payload=b"".join(chunks);expected=(image.width*4+1)*image.height
    decoder=zlib.decompressobj();decoded=decoder.decompress(payload,expected+1)
    if len(decoded)!=expected or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise ValueError("Encoder inflate layout")
    return payload


def assemble(root,output,preview=False,reuse=None):
    root=Path(root).resolve();output=Path(output)
    manifest=json.loads((root/"DRAFT-MANIFEST.json").read_text(encoding="utf-8-sig"))
    sources_list=json.loads((root/"SOURCE-CATALOG.json").read_text())["images"]
    sources={r["archive"]:r for r in sources_list};rows=sorted(manifest["rows"],key=lambda r:r["archive"])
    if len(rows)!=98 or len(sources_list)!=98 or len(sources)!=98 or len({r["archive"]for r in rows})!=98 or {r["archive"]for r in rows}!=set(sources):
        raise ValueError("Incomplete/duplicate slideshow associations")
    if not preview and any(r.get("reviewComplete") is not True for r in rows):
        raise ValueError("Whole-artwork review incomplete; unfinished previews must be explicit")
    previous_rows={};previous_pack=None;reused=0
    if reuse is not None:
        reuse=Path(reuse).resolve();previous_pack=reuse/'slideshow-ui.ownedz'
        previous=json.loads((reuse/'slideshow-ui.ownedz.json').read_text())
        if previous.get('schema')!=3 or previous_pack.stat().st_size!=previous['fileBytes']:
            raise ValueError("Wrong reusable resource")
        check=hashlib.sha256()
        with previous_pack.open('rb')as stream:
            for block in iter(lambda:stream.read(1024*1024),b''):check.update(block)
        if check.hexdigest()!=previous['packSha256']:raise ValueError("Reusable resource changed")
        previous_rows={r['archive']:r for r in previous['images']}
        if len(previous_rows)!=98:raise ValueError("Reusable associations incomplete")
    if output.exists():raise ValueError("Output version already exists")
    output.mkdir(parents=True);pack=output/"slideshow-ui.ownedz";index=[]
    with pack.open("xb")as stream:
        stream.write(b"NFSSLDR3"+struct.pack("<II",3,98))
        for row in rows:
            relative=Path(row["draft"])
            if relative.is_absolute()or ".."in relative.parts or relative.suffix.lower()!=".png":raise ValueError("Invalid private image name")
            path=(root/relative).resolve(strict=True)
            if not path.is_relative_to(root):raise ValueError("Image leaves private root")
            if path.stat().st_size>64*1024**2:raise ValueError("Source image size bound")
            if hashlib.sha256(path.read_bytes()).hexdigest()!=row["draftSha256"]:raise ValueError("Selected photo changed")
            with Image.open(path)as original:
                w,h=original.size
                if not(0<w<=4096 and 0<h<=4096 and w*3==h*4):raise ValueError("Invalid photo canvas")
                if original.mode not in ("RGB","RGBA"):raise ValueError("Unexpected photo mode")
                image=original.convert("RGBA");raw=image.tobytes();raw_sha=hashlib.sha256(raw).hexdigest()
                source=sources[row['archive']];prior=previous_rows.get(row['archive']);chunk=None
                if prior and all(prior.get(k)==value for k,value in (
                    ('draftSha256',row['draftSha256']),('rgbaSha256',raw_sha),
                    ('sourceSha256',source['archiveSha256']),('entrySha256',source['entrySha256']),
                    ('width',w),('height',h),('predictor',1),('version',3))):
                    if not(16<=prior['offset']<=previous['fileBytes'] and 0<prior['bytes']<=32*1024**2 and prior['bytes']<=previous['fileBytes']-prior['offset']):raise ValueError("Reusable chunk bounds")
                    with previous_pack.open('rb')as prior_stream:
                        prior_stream.seek(prior['offset']);chunk=prior_stream.read(prior['bytes'])
                    if hashlib.sha256(chunk).hexdigest()!=prior['chunkSha256']:raise ValueError("Reusable chunk changed")
                    reused+=1
                if chunk is None:chunk=filtered_payload(image)
            if not 0<len(chunk)<=32*1024**2:raise ValueError("Photo chunk budget")
            source=sources[row["archive"]]
            index.append({"archive":row["archive"],"offset":stream.tell(),"bytes":len(chunk),"width":w,"height":h,"predictor":1,"version":3,"sourceSha256":source["archiveSha256"],"entrySha256":source["entrySha256"],"chunkSha256":hashlib.sha256(chunk).hexdigest(),"rgbaSha256":hashlib.sha256(raw).hexdigest(),"draftSha256":row["draftSha256"],"accepted":row.get("reviewComplete")is True})
            stream.write(chunk)
            if stream.tell()>512*1024**2:raise ValueError("Photo resource budget")
    size=pack.stat().st_size
    lines=['// PRIVATE generated identities; do not publish.', '#pragma once', '#include "hd/SlideshowSourceIdentity.h"', '#include "hd/SlideshowRuntimeImage.h"', 'namespace nfs_slideshow_private {', 'static const nfs_slideshow::SourcePin sources[]={']
    for r in index:
        source=sources[r["archive"]]
        lines.append('{'+','.join(str(source[k])for k in ['archiveBytes','offset','spanBytes','format','width','height'])+','+array(source['archiveSha256'])+','+array(source['entrySha256'])+'},')
    lines+=['};','static const nfs_slideshow::RuntimeImagePin images[]={']
    for r in index:
        lines.append('{'+','.join(map(str,[size,r['offset'],r['bytes']]))+','+','.join(array(r[k])for k in ['sourceSha256','entrySha256','chunkSha256'])+',{'+str(r['width'])+','+str(r['height'])+','+array(r['rgbaSha256'])+',1},'+str(r['accepted']).lower()+',3},')
    lines+=['};','}']
    (output/'SlideshowCatalogData.h').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    pack_hash=hashlib.sha256()
    with pack.open('rb')as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):pack_hash.update(block)
    receipt={'schema':3,'previewOnly':preview,'fileBytes':size,'packSha256':pack_hash.hexdigest(),'codec':'PNG method0 row filters; non-interlaced RGBA8; zlib','pixelsLosslesslyPreserved':True,'reusedChunks':reused,'encodedChunks':98-reused,'images':index,'appIntegrated':False,'physicalIpadVerified':False}
    (output/'slideshow-ui.ownedz.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    return {'images':98,'fileBytes':size,'previewOnly':preview,'schema':3,'reusedChunks':reused,'encodedChunks':98-reused}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('root');parser.add_argument('output');parser.add_argument('--preview',action='store_true');parser.add_argument('--reuse',type=Path,help='Hash-verified previous version3 resource; re-encode changed photos only');args=parser.parse_args()
    print(json.dumps(assemble(args.root,args.output,args.preview,args.reuse)))
