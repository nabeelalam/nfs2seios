"""Compile eight separately owned Location remasters against exact local originals.
Original coverage is authenticated separately. Every remastered photo uses
one independent rounded card and soft shadow.
"""
# SPDX-License-Identifier: MIT
import argparse,hashlib,json,struct,zlib
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageOps
from assemble_hd_artwork import decode_qfs,inspect_fsh
ROOT=Path(__file__).resolve().parent.parent
LIMIT=32*1024*1024
NAMES=['trk6','trk5','trk8','trk4','trk1','trk3','trk2','trk7']
def sha(b):return hashlib.sha256(b).hexdigest()
def need(ok,why):
 if not ok:raise ValueError(why)
def render_card(master, size):
    scale = 4
    photo_size = (280 * scale, 133 * scale)
    photo = master.convert('RGBA')
    fitted = ImageOps.contain(photo, photo_size, Image.Resampling.LANCZOS)
    offset = ((photo_size[0] - fitted.width) // 2, (photo_size[1] - fitted.height) // 2)
    # Some masters carry transparent artwork edges. Extend their scenery
    # under those edges so the face has a continuous tone to its shared frame.
    # Only this blurred backing is cropped; the sharp art is contained whole.
    width, height = photo.size
    scenery = photo.crop((width // 5, height // 5, width * 4 // 5, height * 4 // 5))
    face = ImageOps.fit(scenery.convert('RGB'), photo_size, Image.Resampling.LANCZOS)
    face = face.filter(ImageFilter.GaussianBlur(30))
    # Feather inward so hidden RGB outside the source silhouette never leaks.
    alpha = fitted.getchannel('A')
    feather = alpha.filter(ImageFilter.GaussianBlur(8))
    face.paste(fitted, offset, ImageChops.darker(alpha, feather))
    shape = Image.new('L', photo_size)
    ImageDraw.Draw(shape).rounded_rectangle(
        (0, 0, photo_size[0] - 1, photo_size[1] - 1), radius=2 * scale, fill=255)
    shadow = Image.new('L', size)
    shadow.paste(shape, (4 * scale, 3 * scale))
    shadow = shadow.filter(ImageFilter.GaussianBlur(2 * scale)).point(lambda value: value * 140 // 255)
    pixels = Image.new('RGBA', size)
    pixels.putalpha(shadow)
    # At the shared 4x resolution the rounded mask is binary. OpenGL's
    # linear sampling resolves its edge, and the shadow is premultiplied.
    pixels.paste(face, (0, 0), shape)
    return pixels.tobytes(), offset, fitted.size

def compile_pack(main,artdir):
 recipe=json.loads((ROOT/'assets/hud/location-photo-sources.json').read_text())
 raw=Path(main).read_bytes();need(sha(raw)==recipe['mainQfsSha256'],'Original Main archive identity differs');data=decode_qfs(raw);need(sha(data)==recipe['decodedMainSha256'],'Decoded Main identity differs');directory=inspect_fsh(data);items=recipe['items'];need(len(items)==8,'All8 source mappings required');payload=bytearray(struct.pack('<II',8,0));rows=[]
 for k,item in enumerate(items):
  need(item['index']==k and item['name']==NAMES[k] and item['width']==307 and item['height']==(147 if k==2 else 146),'Wrong source mapping/bounds')
  off,span=directory[item['name']];need(off==item['offset'] and span>=item['span'],'Wrong original entry offset/span');entry=data[off:off+item['span']];need(sha(entry)==item['sourceSha256'],'Original image identity differs')
  first,w,h=struct.unpack_from('<IHH',entry);pal=first>>8;need((first&255)==0x7b and w==307 and h==item['height'] and pal>=16+w*h and pal+16<=len(entry),'Original image geometry differs');colors=struct.unpack_from('<H',entry,pal+4)[0];need(0<colors<=256 and pal+16+4*colors==len(entry),'Original palette span differs')
  indices=entry[16:16+w*h];need(all(i<colors or i==255 for i in indices),'Original palette index out of range');coverage=bytes(255 if i!=255 and entry[pal+16+i*4+3]>=16 else 0 for i in indices);W,H=w*4,h*4;mask=Image.frombytes('L',(w,h),coverage).resize((W,H),Image.Resampling.NEAREST);need(sha(mask.tobytes())==item['coverageSha256'],'Native coverage differs')
  file=Path(artdir)/item['authoredFile'];art=file.read_bytes();need(sha(art)==item['authoredSha256'],'Authored master identity differs');im=Image.open(file);im.load();need(im.width<=8192 and im.height<=8192 and im.width*im.height<=32*1024*1024,'Authored master bounds')
  pixels,xy,fit_size=render_card(im,(W,H));payload.extend(struct.pack('<IIII8f',k,W,H,0,0.,0.,float(w),float(h),0.,0.,float(w),float(h)));payload.extend(pixels);need(len(payload)<=LIMIT,'Location photo payload exceeds32MiB');rows.append({'index':k,'name':item['name'],'width':W,'height':H,'canonicalPixelSha256':sha(pixels),'originalCoverageSha256':sha(mask.tobytes()),'masterSha256':sha(art),'fitOffset':xy,'fitSize':fit_size})
 file=b'NFSLOCPH'+struct.pack('<II',2,len(payload))+zlib.compress(payload,9);need(len(file)<=LIMIT,'Location photo file exceeds32MiB');return file,{'fileBytes':len(file),'fileSha256':sha(file),'payloadBytes':len(payload),'maxFileBytes':LIMIT,'images':rows,'nativeCoveragePolicy':recipe['policy'],'sourceRgbPixelsEmbedded':False,'displayCardPolicy':'Shared 280x133 face and soft shadow', 'nativeCoverageDerivedFromOriginal':True}
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--main',required=True);p.add_argument('--art-dir',required=True);p.add_argument('--output',required=True);a=p.parse_args();file,receipt=compile_pack(a.main,a.art_dir);out=Path(a.output);out.parent.mkdir(parents=True,exist_ok=True)
 with out.open('xb')as f:f.write(file)
 with out.with_suffix('.json').open('x')as f:json.dump(receipt,f,indent=2);f.write('\n')
 print(json.dumps({'fileBytes':receipt['fileBytes'],'payloadBytes':receipt['payloadBytes'],'sha256':receipt['fileSha256']}))
if __name__=='__main__':main()
