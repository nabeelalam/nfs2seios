"""Encode finite authored steering PNGs, verifying pinned straight RGBA identities.
No original game imagery is read. RGB is premultiplied exactly once, rounded.
"""
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse
import hashlib
import json
from PIL import Image
WIDTH,HEIGHT=1424,196
STRAIGHT=("8ea18c208104e589f4a17c722fdcfdac59b5820bc2682486a9360709efa2d031","895b1c850b6fcad951aa2750318022ce0013a6efcdc41d9173ebd23f013b584e","85caaa9782bb245d7614a72134a782f6d4fe6f18250fd82ca03564d751d6dbee","5cef6e20d8267dd0ddba9f03a221f4cc06e6d8865096a8205a6a7fd91e92f6d6")
def generate(art,output):
    manifest=json.loads((art/'rendered-art.json').read_text())
    rows=manifest['images']
    if len(rows)!=4 or [r['level']for r in rows]!=list(range(4)):raise ValueError('Four ordered authored levels required')
    encoded=[];pins=[];counts=[]
    for level,row in enumerate(rows):
        path=art/f'steering-band-{level}.png'
        if hashlib.sha256(path.read_bytes()).hexdigest()!=row['pngSha256']:raise ValueError('PNG identity mismatch')
        with Image.open(path)as image:
            if image.size!=(WIDTH,HEIGHT)or image.mode!='RGBA':raise ValueError('Expected authored straight RGBA dimensions')
            raw=image.tobytes()
        if row['width']!=WIDTH or row['height']!=HEIGHT or row['rgbaSha256']!=STRAIGHT[level]or hashlib.sha256(raw).hexdigest()!=STRAIGHT[level]:raise ValueError('Authored straight RGBA identity mismatch')
        rgba=bytearray(raw)
        for i in range(0,len(rgba),4):
            a=rgba[i+3]
            for c in range(3):rgba[i+c]=(rgba[i+c]*a+127)//255
        for y in range(HEIGHT):
            for x in range(WIDTH):
                if(x<4 or x>=WIDTH-4 or y<4 or y>=HEIGHT-4)and rgba[(y*WIDTH+x)*4+3]:raise ValueError('Missing transparent four-pixel padding')
        pins.append(hashlib.sha256(rgba).digest());runs=[];previous=None;count=0
        for i in range(0,len(rgba),4):
            color=int.from_bytes(rgba[i:i+4],'little')
            if color==previous and count<0xffffffff:count+=1
            else:
                if previous is not None:runs.extend((count,previous))
                previous=color;count=1
        runs.extend((count,previous));counts.append(len(runs));encoded.append(runs)
    paths=[output/name for name in('ControlsSteeringBarIdentity.h','ControlsSteeringBarData.h','ControlsSteeringBarData.c')]+[output/f'ControlsSteeringBarLevel{level}.inc' for level in range(4)]
    if any(p.exists()for p in paths):raise FileExistsError('Generated output already exists; preserve or compare it explicitly')
    identity='// SPDX-License-Identifier: MIT\n#ifndef NFS_CONTROLS_STEERING_BAR_IDENTITY_H\n#define NFS_CONTROLS_STEERING_BAR_IDENTITY_H\nstatic const unsigned char nfs_controls_steering_bar_sha256[4][32]={\n'+',\n'.join('{'+','.join(f'0x{x:02x}'for x in h)+'}'for h in pins)+'\n};\n#endif\n'
    header='// SPDX-License-Identifier: MIT\n#ifndef NFS_CONTROLS_STEERING_BAR_DATA_H\n#define NFS_CONTROLS_STEERING_BAR_DATA_H\n#include <stdint.h>\n#include <stddef.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n/* Pairs: uint32 run length, little-endian packed premultiplied RGBA. */\n'
    data='// SPDX-License-Identifier: MIT\n// Generated only from pinned code-authored artwork.\n#include "ControlsSteeringBarData.h"\n'
    chunks=[]
    for level,runs in enumerate(encoded):
        header+=f'extern const uint32_t nfs_controls_steering_bar_rle_{level}[{len(runs)}];\n'
        part=f'const uint32_t nfs_controls_steering_bar_rle_{level}[{len(runs)}]={{\n'
        part+='\n'.join(','.join(f'0x{x:08x}u'for x in runs[i:i+12])+','for i in range(0,len(runs),12))+'\n};\n'
        chunks.append('// SPDX-License-Identifier: MIT\n// One pinned code-authored sensitivity level.\n'+part)
        data+=f'#include "ControlsSteeringBarLevel{level}.inc"\n'
    header+='extern const uint32_t*const nfs_controls_steering_bar_rle[4];\nextern const size_t nfs_controls_steering_bar_rle_count[4];\n#ifdef __cplusplus\n}\n#endif\n#endif\n'
    data+='const uint32_t*const nfs_controls_steering_bar_rle[4]={'+','.join(f'nfs_controls_steering_bar_rle_{i}'for i in range(4))+'};\nconst size_t nfs_controls_steering_bar_rle_count[4]={'+','.join(str(n)for n in counts)+'};\n'
    output.mkdir(parents=True,exist_ok=True)
    for path,text in zip(paths,(identity,header,data,*chunks)):path.write_text(text,newline='\n')
    return {'levels':4,'size':[WIDTH,HEIGHT],'premultipliedSha256':[h.hex()for h in pins],'rleWords':counts,'originalGamePixelsRead':False}
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--art-dir',required=True,type=Path);p.add_argument('--output-dir',required=True,type=Path);a=p.parse_args();print(json.dumps(generate(a.art_dir,a.output_dir),indent=2))
