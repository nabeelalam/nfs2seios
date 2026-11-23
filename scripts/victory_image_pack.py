"""Lossless, independently readable victory artwork chunks; private outputs only."""
from pathlib import Path
import argparse,hashlib,json,struct
from PIL import Image
from slideshow_image_pack import filtered_payload
MAGIC=b'NFSVIC01'
MAX_FILE=512*1024*1024
RECORD=struct.Struct('<6IQ32s32s32s')

def sha(data):return hashlib.sha256(data).digest()
def digest(value):
    raw=bytes.fromhex(value)
    if len(raw)!=32 or not any(raw):raise ValueError('Invalid source/image digest')
    return raw

def canvas_shape(aspect,w,h):
    if aspect==0:return w*3==h*4
    delta=w*9-h*16
    return delta==0 or (w>=640 and h>=360 and abs(delta)<=8)

def assemble(manifest_path,output,preview=False):
    manifest_path=Path(manifest_path).resolve();root=manifest_path.parent
    manifest=json.loads(manifest_path.read_text());rows=manifest['images']
    if manifest.get('schema')!=1 or len(rows)not in(16,32):raise ValueError('Incomplete image inventory')
    rows=sorted(rows,key=lambda x:(x['aspect'],x['image']))
    expected=[(a,i)for a in range(len(rows)//16)for i in range(16)]
    if [(x['aspect'],x['image'])for x in rows]!=expected:raise ValueError('Duplicate/missing scene identity')
    if not preview and (len(rows)!=32 or any(x.get('reviewComplete')is not True for x in rows)):
        raise ValueError('Production artwork needs all reviewed 4:3 and 16:9 scenes')
    if len(rows)==32 and any(rows[i]['sourceSha256']!=rows[i+16]['sourceSha256']for i in range(16)):
        raise ValueError('Adaptive source associations differ')
    output=Path(output);output.mkdir(parents=True,exist_ok=True)
    pack=output/'victory-ui.ownedz';temporary=output/'victory-ui.ownedz.partial'
    if temporary.exists():raise ValueError('Existing incomplete output')
    metadata_bytes=24+128*len(rows);offset=metadata_bytes;records=[];report=[]
    try:
        with temporary.open('xb')as stream:
            stream.write(bytes(metadata_bytes))
            for row in rows:
                path=(root/row['master']).resolve()
                if not path.is_relative_to(root)or path.stat().st_size>64*1024*1024:raise ValueError('Master path/size')
                if sha(path.read_bytes())!=digest(row['masterSha256']):raise ValueError('Master differs from authoring pin')
                with Image.open(path)as opened:
                    w,h=opened.size
                    if not(0<w<=4096 and 0<h<=4096)or not canvas_shape(row['aspect'],w,h)or w*h*4>64*1024*1024:raise ValueError('Canvas bounds/aspect')
                    rgba=opened.convert('RGBA')
                    if rgba.getchannel('A').getextrema()!=(255,255):raise ValueError('Victory plates must be opaque')
                    raw=rgba.tobytes();chunk=filtered_payload(rgba.convert('RGB'))
                if not chunk or len(chunk)>32*1024*1024 or offset+len(chunk)>MAX_FILE:raise ValueError('Chunk/file bound')
                records.append(RECORD.pack(row['aspect'],row['image'],w,h,2,len(chunk),offset,sha(raw),sha(chunk),digest(row['sourceSha256'])))
                stream.write(chunk)
                report.append({'aspect':row['aspect'],'image':row['image'],'width':w,'height':h,'offset':offset,'bytes':len(chunk),'rgbaSha256':sha(raw).hex(),'chunkSha256':sha(chunk).hex(),'sourceSha256':row['sourceSha256']})
                offset+=len(chunk)
            metadata=struct.pack('<8s4I',MAGIC,1,len(rows),int(preview),128)+b''.join(records)
            assert len(metadata)==metadata_bytes
            stream.seek(0);stream.write(metadata)
        temporary.replace(pack)
    except BaseException:
        if temporary.exists():temporary.unlink()
        raise
    result={'schema':1,'previewOnly':preview,'fileBytes':offset,'metadataBytes':metadata_bytes,'metadataSha256':sha(metadata).hex(),'packSha256':sha(pack.read_bytes()).hex(),'codec':'PNG method0 row filters, opaque RGB8, zlib','losslessPixels':True,'images':report,'appIntegrated':False,'physicalDeviceVerified':False}
    (output/'victory-ui.ownedz.json').write_text(json.dumps(result,indent=2)+'\n')
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('manifest',type=Path);p.add_argument('output',type=Path);p.add_argument('--preview',action='store_true');a=p.parse_args();r=assemble(a.manifest,a.output,a.preview);print(json.dumps({k:v for k,v in r.items()if k!='images'}))
