"""Package an authored transparent emblem; no original game pixels are emitted."""
import argparse,hashlib,json,zlib
from pathlib import Path
from PIL import Image

def prepare(master,identities,output):
    sources=json.loads(identities.read_text())
    assert [x['entry']for x in sources]==['tko5','tkod']
    assert all(len(x['convertedSha256'])==64 and len(bytes.fromhex(x['convertedSha256']))==32 for x in sources)
    assert all(x['sourceDimensions']==[85,85]and x['canvasOffset']==[22,0]for x in sources)
    image=Image.open(master).convert('RGBA');alpha=image.getchannel('A')
    box=alpha.point(lambda v:255 if v>=4 else 0).getbbox()
    if not box:raise ValueError('No visible authored emblem')
    box=(max(0,box[0]-2),max(0,box[1]-2),min(image.width,box[2]+2),min(image.height,box[3]+2))
    emblem=image.crop(box);emblem.thumbnail((340,340),Image.Resampling.LANCZOS)
    canvas=Image.new('RGBA',(512,512));canvas.paste(emblem,(88+(340-emblem.width)//2,(340-emblem.height)//2))
    pixels=bytearray(canvas.tobytes())
    for at in range(0,len(pixels),4):
        for c in range(3):pixels[at+c]=(pixels[at+c]*pixels[at+3]+127)//255
    disabled=bytearray(pixels)
    for at in range(0,len(disabled),4):
        for c in range(3):disabled[at+c]=(disabled[at+c]*48+50)//100
    payload=bytes(pixels+disabled);compressed=zlib.compress(payload,9)
    assert zlib.decompress(compressed)==payload
    output.mkdir(parents=True,exist_ok=True)
    chunks=[compressed[k:k+49152]for k in range(0,len(compressed),49152)]
    for i,chunk in enumerate(chunks):
        text='// Authored image data, losslessly deflated.\n'+''.join(','.join('0x%02x'%v for v in chunk[k:k+16])+',\n'for k in range(0,len(chunk),16))
        (output/('KnockoutEmblemPixelsPart%02d.inc'%i)).write_text(text)
    def values(s):return ','.join('0x'+s[k:k+2]for k in range(0,64,2))
    arts=[hashlib.sha256(bytes(x)).hexdigest()for x in [pixels,disabled]]
    header='// SPDX-License-Identifier: MIT\n#pragma once\nstatic const unsigned char nfs_knockout_deflated[]={\n'
    header+=''.join('#include "KnockoutEmblemPixelsPart%02d.inc"\n'%i for i in range(len(chunks)))+'};\n'
    header+='static const unsigned char nfs_knockout_source_sha[2][32]={'+','.join('{'+values(x['convertedSha256'])+'}'for x in sources)+'};\n'
    header+='static const unsigned char nfs_knockout_art_sha[2][32]={'+','.join('{'+values(x)+'}'for x in arts)+'};\n'
    (output/'KnockoutEmblemData.h').write_text(header)
    result={'authoredMasterSha256':hashlib.sha256(master.read_bytes()).hexdigest(),'sourceIdentities':sources,'canvas':[512,512],'logicalCanvas':[128,128],'logicalOriginalPlacement':[22,0,85,85],'authoredPixelSha256':arts,'decodedBytes':len(payload),'compressedBytes':len(compressed),'losslessRoundtrip':True,'originalPixelsEmbedded':False,'parts':len(chunks)}
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--master',type=Path,required=True);p.add_argument('--identities',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--receipt',type=Path,required=True);a=p.parse_args();result=prepare(a.master,a.identities,a.output);a.receipt.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
