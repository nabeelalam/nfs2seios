"""Export authenticated whole binding phrases in bounded source chunks."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,hashlib,json

def export(manifest,output):
    v=json.loads(manifest.read_text());rows=v['rows']
    if v.get('schema')!=1 or not v.get('wholePhraseImagesOnly')or v.get('reusableFontAlphabet')or len(rows)!=360:raise ValueError('Expected complete-phrase manifest')
    identity=['// SPDX-License-Identifier: MIT','#pragma once','#include <stddef.h>','#include <string.h>','#define NFS_CONTROLS_BINDING_PHRASE_IMAGES 360u',
      'typedef struct NfsControlsBindingPhraseIdentity { unsigned image,sourceSize,kind,device,number,count,width,height; const char*name;const char*tokens[4];unsigned positions[4],advances[4];unsigned char sha256[32];} NfsControlsBindingPhraseIdentity;',
      'static const NfsControlsBindingPhraseIdentity nfs_controls_binding_phrase_images[360]={']
    arrays=[];pointers=[]
    for i,r in enumerate(rows):
        count=len(r['tokens']);w,h=r['width'],r['height'];runs=r['alphaRuns']
        if r['image']!=i+1 or r['sourceSize']not in(16,18)or not 1<=count<=4 or len(r['positions'])!=count or len(r['advances'])!=count or w!=r['logicalWidth']*4 or h!=128 or len(runs)%2:raise ValueError('Invalid complete phrase geometry')
        alpha=bytearray()
        for n,a in zip(runs[::2],runs[1::2]):
            if not 0<n<=65535 or not 0<=a<=255 or len(alpha)+n>w*h:raise ValueError('Invalid image RLE')
            alpha.extend(bytes([a])*n)
        if len(alpha)!=w*h or any(alpha[:w])or any(alpha[-w:])or any(alpha[y*w]or alpha[y*w+w-1]for y in range(h)):raise ValueError('Invalid phrase padding')
        rgba=bytearray([255])*(w*h*4);rgba[3::4]=alpha
        if hashlib.sha256(rgba).hexdigest()!=r['rgbaSha256']:raise ValueError('Phrase digest mismatch')
        tokens=','.join(json.dumps(t,ensure_ascii=True)for t in r['tokens'])+(',0'*(4-count));positions=','.join(map(str,r['positions']+[0]*(4-count)));advances=','.join(map(str,r['advances']+[0]*(4-count)));digest=','.join(str(b)for b in bytes.fromhex(r['rgbaSha256']))
        nums=','.join(str(r[k])for k in ('image','sourceSize','kind','device','number'))
        identity.append('{%s,%d,%d,%d,%s,{%s},{%s},{%s},{%s}},'%(nums,count,w,h,json.dumps(r['name'],ensure_ascii=True),tokens,positions,advances,digest))
        name='nfs_controls_binding_phrase_rle_%d'%(i+1);arrays.append('static const unsigned short %s[]={%s};'%(name,','.join(map(str,runs))));pointers.append('{%s,sizeof(%s)/sizeof(unsigned short)}'%(name,name))
    identity += ['};','static const NfsControlsBindingPhraseIdentity*nfs_controls_binding_phrase_identity(unsigned image){return image&&image<=NFS_CONTROLS_BINDING_PHRASE_IMAGES?nfs_controls_binding_phrase_images+image-1:0;}',
      'static unsigned nfs_controls_binding_phrase_find(unsigned size,unsigned kind,unsigned device,const char*value){unsigned base,i;if(size!=16&&size!=18)return 0;base=size==18?180u:0u;if(kind==0){if(!value)return 0;for(i=0;i<83;++i)if(!strcmp(value,nfs_controls_binding_phrase_images[base+i].name))return base+i+1;return 0;}if(kind==3)return base+180;if(!value||device<1||device>2)return 0;if(kind==1){for(i=0;i<16;++i)if(!strcmp(value,nfs_controls_binding_phrase_images[base+83+(device-1)*16+i].name))return base+84+(device-1)*16+i;}if(kind==2&&value[0]>=65&&value[0]<=96&&!value[1])return base+116+(device-1)*32+unsigned((unsigned char)value[0]-65);return 0;}',]
    # Keep this metadata header C-compatible for the maintained GLES C unit.
    identity[-1]=identity[-1].replace('unsigned((unsigned char)value[0]-65)','(unsigned)((unsigned char)value[0]-65)')
    output.mkdir(parents=True,exist_ok=True);chunks=[];current=[];size=0
    for line in arrays:
        if len(line.encode())>512000:raise ValueError('Phrase array exceeds source chunk limit')
        if current and size+len(line)+1>512000:chunks.append(current);current=[];size=0
        current.append(line);size+=len(line)+1
    if current:chunks.append(current)
    data=['// SPDX-License-Identifier: MIT','#pragma once']
    for i,chunk in enumerate(chunks,1):
        name='ControlsBindingPhraseDataPart%02d.inc'%i;(output/name).write_text('// SPDX-License-Identifier: MIT\n'+'\n'.join(chunk)+'\n');data.append('#include "'+name+'"')
    data += ['typedef struct NfsControlsBindingPhraseRuns {const unsigned short*data;size_t count;}NfsControlsBindingPhraseRuns;','static const NfsControlsBindingPhraseRuns nfs_controls_binding_phrase_runs[360]={'+','.join(pointers)+'};']
    (output/'ControlsBindingPhraseIdentity.h').write_text('\n'.join(identity)+'\n');(output/'ControlsBindingPhraseData.h').write_text('\n'.join(data)+'\n');return dict(completePhrases=len(rows),parts=len(chunks),allPixelHashesVerified=True,reusableAlphabetExported=False)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--manifest',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args();print(json.dumps(export(a.manifest,a.output)))
