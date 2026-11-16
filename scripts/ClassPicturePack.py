"""Build a private three-image resource from explicitly accepted PMA RGBA bytes.
No resizing, image generation, source-identity inference or app integration.
"""
import argparse,hashlib,json,pathlib,struct,zlib
WIDTH,HEIGHT=1224,584
LIMIT=9*1024*1024

def build(paths):
    payload=bytearray(struct.pack('<II',3,0));records=[]
    for role,path in enumerate(paths):
        pixels=path.read_bytes()
        if len(pixels)!=WIDTH*HEIGHT*4:raise ValueError('Expected exact1224x584 RGBA PMA bytes')
        if any(max(pixels[k:k+3])>pixels[k+3] for k in range(0,len(pixels),4)):raise ValueError('Non-PMA pixel')
        payload+=struct.pack('<IIII8f',role,WIDTH,HEIGHT,0,0,0,306,146,0,0,306,146)+pixels
        records.append(dict(role=role,inputSHA256=hashlib.sha256(pixels).hexdigest()))
    file=b'NFSCLPIC'+struct.pack('<II',1,len(payload))+zlib.compress(payload,9)
    if max(len(file),len(payload))>LIMIT:raise ValueError('Resource exceeds9MiB bound')
    return file,len(payload),records

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--accepted-pma',type=pathlib.Path,nargs=3,required=True,metavar=('A','B','C'))
    p.add_argument('--output-dir',type=pathlib.Path,required=True)
    a=p.parse_args();data,size,records=build(a.accepted_pma)
    a.output_dir.mkdir(parents=True,exist_ok=True)
    dest=a.output_dir/'class-picture-ui.ownedz'
    if dest.exists():raise ValueError('Refuse overwrite of existing resource')
    digest=hashlib.sha256(data).hexdigest();dest.write_bytes(data)
    (a.output_dir/'ClassPictureArtworkPin.h').write_text('#pragma once\n#define NFS_CLASS_PICTURE_ARTWORK_FILENAME "class-picture-ui.ownedz"\n#define NFS_CLASS_PICTURE_ARTWORK_SHA256 "'+digest+'"\n#define NFS_CLASS_PICTURE_ARTWORK_FILE_BYTES '+str(len(data))+'u\n#define NFS_CLASS_PICTURE_ARTWORK_PAYLOAD_BYTES '+str(size)+'u\n')
    (a.output_dir/'ClassPictureResourceReceipt.json').write_text(json.dumps(dict(schema=1,sha256=digest,fileBytes=len(data),payloadBytes=size,images=records,sourceAuthority=False,appIntegrated=False),indent=2)+'\n')
if __name__=='__main__':main()
