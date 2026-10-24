"""Export verified finite Setup-control word images; never export a font alphabet."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,hashlib,json

def export(manifest,output):
    value=json.loads(manifest.read_text());rows=value['words']
    if value.get('schema')!=1 or not value.get('wholeWordGraphicsOnly') or len(rows)!=28:
        raise ValueError('Expected finite whole-word manifest')
    identity=['// SPDX-License-Identifier: MIT','#ifndef NFS_CONTROLS_SETUP_WORD_IDENTITY_H','#define NFS_CONTROLS_SETUP_WORD_IDENTITY_H','#include <stdint.h>']
    data=['// SPDX-License-Identifier: MIT','#ifndef NFS_CONTROLS_SETUP_WORD_DATA_H','#define NFS_CONTROLS_SETUP_WORD_DATA_H','// Generated complete-word alpha images only; no font or alphabet.']
    for i,row in enumerate(rows):
        width,height=row['width'],row['height'];runs=row['alphaRuns']
        if row['image']!=i+1 or height!=128 or width!=row['logicalAdvance']*4 or row['sourceSize']not in(16,18) or len(runs)%2:
            raise ValueError('Invalid finite image geometry')
        alpha=bytearray()
        for n,a in zip(runs[::2],runs[1::2]):
            if not 0<n<=65535 or not 0<=a<=255 or len(alpha)+n>width*height:raise ValueError('Invalid alpha run')
            alpha.extend(bytes([a])*n)
        if len(alpha)!=width*height:raise ValueError('Incomplete alpha image')
        if any(alpha[:width])or any(alpha[-width:])or any(alpha[y*width]or alpha[y*width+width-1]for y in range(height)):
            raise ValueError('Missing transparent padding')
        rgba=bytearray([255])*(width*height*4);rgba[3::4]=alpha
        if hashlib.sha256(rgba).hexdigest()!=row['rgbaSha256']:raise ValueError('Image digest mismatch')
        data.append('static const unsigned short nfs_controls_setup_word_alpha_rle_%d[]={%s};'%(i+1,','.join(map(str,runs))))
    for name,key in [('width','width'),('advance','logicalAdvance'),('source_size','sourceSize')]:
        identity.append('static const unsigned nfs_controls_setup_word_%s[28]={%s};'%(name,','.join(str(r[key])for r in rows)))
    identity.append('static const char*const nfs_controls_setup_word_text[28]={%s};'%','.join(json.dumps(r['text'],ensure_ascii=True)for r in rows))
    identity.append('static const unsigned char nfs_controls_setup_word_sha256[28][32]={%s};'%','.join('{'+','.join(str(x)for x in bytes.fromhex(r['rgbaSha256']))+'}'for r in rows))
    identity.append('#endif');data.append('#endif');output.mkdir(parents=True,exist_ok=True)
    # Keep generated source reviewable below the publication size limit. Split
    # only between complete word arrays; image pixels and digests are unchanged.
    arrays=data[4:-1];chunks=[];current=[];size=0
    for line in arrays:
        if len(line.encode())>512000:raise ValueError('Complete word array exceeds source chunk budget')
        if current and size+len(line.encode())+1>512000:
            chunks.append(current);current=[];size=0
        current.append(line);size+=len(line.encode())+1
    if current:chunks.append(current)
    data=data[:4]
    for index,chunk in enumerate(chunks,1):
        name='ControlsSetupWordDataPart%02d.inc'%index
        (output/name).write_bytes(('// SPDX-License-Identifier: MIT\n'+'\n'.join(chunk)+'\n').encode())
        data.append('#include "'+name+'"')
    data.append('#endif')
    for name,lines in [('ControlsSetupWordIdentity.h',identity),('ControlsSetupWordData.h',data)]:
        target=output/name;encoded=('\n'.join(lines)+'\n').encode();target.write_bytes(encoded)
    return {'finiteImages':len(rows),'allPayloadsHashVerified':True,'fontAlphabetExported':False}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--manifest',required=True,type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args();print(json.dumps(export(a.manifest,a.output)))
