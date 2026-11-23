"""Pack eleven separate Credits layers with lossless zlib compression.
Original PNG/SVG masters remain private and unchanged. No app build or install.
"""
from pathlib import Path
import argparse,hashlib,json,struct,zlib
MAGIC=b'NFSCRD01'
ROLES=('backdrop','logo','watermark','heading_en','heading_de','heading_fr','heading_es','heading_it','heading_sv','kenwood','creative_house_y2')
LIMIT=96*1024*1024

def pack(images):
    if set(images)!=set(ROLES):raise ValueError('Exactly the eleven named Credits layers are required')
    payload=bytearray();records=[]
    for role in ROLES:
        w,h,rgba=images[role]
        if not 0<w<=8192 or not 0<h<=8192 or len(rgba)!=w*h*4:raise ValueError('Invalid RGBA dimensions')
        if len(payload)+44+len(rgba)>LIMIT:raise ValueError('Credits payload is too large')
        if any(rgba[k]>rgba[k+3] or rgba[k+1]>rgba[k+3] or rgba[k+2]>rgba[k+3] for k in range(0,len(rgba),4)):raise ValueError('Expected premultiplied RGBA')
        if role=='backdrop' and any(rgba[k]!=255 for k in range(3,len(rgba),4)):raise ValueError('Backdrop must be opaque')
        digest=hashlib.sha256(rgba).digest();payload.extend(struct.pack('<III',w,h,len(rgba))+digest+rgba)
        records.append(dict(role=role,width=w,height=h,bytes=len(rgba),sha256=digest.hex()))
    result=MAGIC+struct.pack('<II',1,len(payload))+zlib.compress(payload,9)
    if zlib.decompress(result[16:])!=payload:raise ValueError('Compression verification failed')
    return result,bytes(payload),records

def main():
    from PIL import Image
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--manifest',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    if a.output.exists():raise ValueError('Output must be new')
    manifest=json.loads(a.manifest.read_text());images={}
    for role in ROLES:
        p=a.manifest.parent/manifest[role]
        with Image.open(p) as im:
            im=im.convert('RGBA');raw=im.tobytes();pma=bytearray(raw)
            for k in range(0,len(pma),4):
                alpha=raw[k+3]
                for c in range(3):pma[k+c]=(raw[k+c]*alpha+127)//255
            images[role]=(im.width,im.height,bytes(pma))
    file,payload,records=pack(images);a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_bytes(file)
    report=dict(schema=1,fileBytes=len(file),payloadBytes=len(payload),sha256=hashlib.sha256(file).hexdigest(),compression='lossless zlib of authenticated premultiplied RGBA',roles=records,artworkRedistributionApproved=False)
    a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='roles'}))
if __name__=='__main__':main()
