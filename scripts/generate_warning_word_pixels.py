"""Generate fixed classic WRONG WAY pixels in proven unused auxiliary banks."""
# SPDX-License-Identifier: MIT
import argparse,hashlib,json
from pathlib import Path
from rasterize_hud_svg import rasterize
ROOT=Path(__file__).resolve().parents[1]
SOURCE='assets/hud/warning/wrong-way.svg'
SOURCE_SHA='8b428311c5a566d1c75faf98409617e76df6b27c5ecf0c4b456b0324061457b1'
PUNCT_SOURCE='assets/hud/warning/exclamation.svg'
PUNCT_SHA='4b9d7b1c180bee8d0eae680386b92d3cd8d13d05a8e6f6d38541d76eccfb987c'
BANKS=((0,318,1,963),(318,126,1,129),(444,62,129,1),(506,62,129,61),(568,50,633,1))
def reviewed_warning_word_source(name,data):return (name==SOURCE and hashlib.sha256(data).hexdigest()==SOURCE_SHA) or (name==PUNCT_SOURCE and hashlib.sha256(data).hexdigest()==PUNCT_SHA)

def outputs(root=None):
 root=ROOT if root is None else Path(root);source=(root/SOURCE).read_bytes()
 if not reviewed_warning_word_source(SOURCE,source):raise ValueError('Warning vector pin differs')
 rgba=rasterize(source,669,63)
 if any(rgba[i]or rgba[i+1]or rgba[i+2]for i in range(0,len(rgba),4)):raise ValueError('Warning ink must remain black')
 if any(rgba[(y*669+x)*4+3]for y in range(63)for x in range(669)if not(51<=x<669 and 3<=y<60)):raise ValueError('Warning ink escaped original margins')
 alpha=b''.join(rgba[(y*669+51)*4+3:(y*669+669)*4:4]for y in range(3,60))
 if len(alpha)!=35226 or not any(alpha):raise ValueError('Warning crop missing')
 punct=(root/PUNCT_SOURCE).read_bytes()
 if not reviewed_warning_word_source(PUNCT_SOURCE,punct):raise ValueError('Warning punctuation vector pin differs')
 prgba=rasterize(punct,18,57)
 if any(prgba[i]or prgba[i+1]or prgba[i+2]for i in range(0,len(prgba),4)):raise ValueError('Warning punctuation ink must remain black')
 alpha+=prgba[3::4]
 runs=bytearray();at=0
 while at<len(alpha):
  value=alpha[at];n=1
  while at+n<len(alpha)and n<65535 and alpha[at+n]==value:n+=1
  runs+=bytes((n&255,n>>8,value));at+=n
 array=lambda name,data:'static const unsigned char '+name+'[]={\n'+',\n'.join(','.join(map(str,data[i:i+24]))for i in range(0,len(data),24))+'\n};\n'
 text='// SPDX-License-Identifier: MIT\n// Outlined Oxanium700: assets/hud/fonts/Oxanium-OFL.txt. No original pixels.\n#include "TextureDigest.h"\n#include <stddef.h>\n#include <string.h>\n'
 text+=array('nfs_warning_word_rle',runs)+array('nfs_warning_word_rle_sha256',hashlib.sha256(runs).digest())
 text+='typedef struct {unsigned start,width,x,y;} NfsWarningWordBank;\nstatic const NfsWarningWordBank nfs_warning_word_banks[5]={'+','.join('{'+','.join(map(str,b))+'}'for b in BANKS)+'};\n'
 text+='''
static int nfs_warning_word_expand_checked(unsigned char*rgba,size_t capacity,const unsigned char*rle,size_t bytes){
 if(!rgba||capacity<4194304||!rle||!bytes||bytes%3||bytes>36252u*3u)return 0;
 unsigned total=0;for(size_t i=0;i<bytes;i+=3){unsigned n=rle[i]|((unsigned)rle[i+1]<<8);if(!n||n>36252u-total)return 0;total+=n;}if(total!=36252)return 0;
 unsigned char sha[32];if(!nfs_texture_sha256(rle,bytes,sha)||memcmp(sha,nfs_warning_word_rle_sha256,32))return 0;
 unsigned pos=0;for(size_t i=0;i<bytes;i+=3){unsigned n=rle[i]|((unsigned)rle[i+1]<<8);for(unsigned j=0;j<n;++j,++pos){if(pos>=35226){unsigned at=pos-35226;unsigned char*p=rgba+((size_t)(129+at/18)*1024+129+at%18)*4;p[0]=p[1]=p[2]=0;p[3]=rle[i+2];continue;}unsigned x=pos%618,y=pos/618,k=0;while(k<4&&x>=nfs_warning_word_banks[k+1].start)++k;const NfsWarningWordBank*b=&nfs_warning_word_banks[k];unsigned char*p=rgba+((size_t)(b->y+y)*1024+b->x+x-b->start)*4;p[0]=p[1]=p[2]=0;p[3]=rle[i+2];}}
 for(unsigned k=0;k<5;++k){const NfsWarningWordBank*b=&nfs_warning_word_banks[k];
  for(unsigned y=0;y<57;++y){unsigned char*left=rgba+((size_t)(b->y+y)*1024+b->x-1)*4;unsigned char*right=left+(b->width+1)*4;
   if(k){const NfsWarningWordBank*p=&nfs_warning_word_banks[k-1];memcpy(left,rgba+((size_t)(p->y+y)*1024+p->x+p->width-1)*4,4);}else memset(left,0,4);
   if(k<4){const NfsWarningWordBank*p=&nfs_warning_word_banks[k+1];memcpy(right,rgba+((size_t)(p->y+y)*1024+p->x)*4,4);}else memset(right,0,4);
  }
  memset(rgba+((size_t)(b->y-1)*1024+b->x-1)*4,0,(b->width+2)*4);memset(rgba+((size_t)(b->y+57)*1024+b->x-1)*4,0,(b->width+2)*4);
 }
 for(unsigned y=128;y<187;++y)for(unsigned x=128;x<148;++x)if(x==128||x==147||y==128||y==186)memset(rgba+((size_t)y*1024+x)*4,0,4);
 return 1;
}
static int nfs_warning_word_expand(unsigned char*rgba,size_t capacity){return nfs_warning_word_expand_checked(rgba,capacity,nfs_warning_word_rle,sizeof(nfs_warning_word_rle));}
'''
 shader='if(vTexCoord.z>75.5 && vTexCoord.z<77.5){vec2 source=hdCoord*256.0/(255.0/256.0);bool single=vTexCoord.z>76.5;vec2 p=(source-vec2(17.0,1.0))*vec2(single?618.0/213.0:3.0,3.0);pauseOutside=p.x<0.0 || p.x>=618.0 || p.y<0.0 || p.y>=57.0;'
 for i,(start,width,x,y)in enumerate(BANKS):
  shader+=('if'if not i else 'else if'if i<4 else 'else')+(' (p.x<'+str(start+width)+'.0)'if i<4 else '')+' hdCoord=(vec2('+str(x)+'.0,'+str(y)+'.0)+p-vec2('+str(start)+'.0,0.0))/1024.0;'
 shader+='if(single && source.x>=233.0 && source.x<239.0){p=(source-vec2(233.0,1.0))*3.0;pauseOutside=p.y<0.0 || p.y>=57.0;hdCoord=(vec2(129.0,129.0)+p)/1024.0;}}\n';shader='// SPDX-License-Identifier: MIT\n// Guttered banks; original highp/basic boundary retained.\n'+json.dumps(shader)+'\n'
 meta=dict(schema=1,scope='Fixed classic black warning word; no original game pixels',text='WRONG WAY',font='Outlined project Oxanium700/OFL1.1',sourceSha256=SOURCE_SHA,punctuationSha256=PUNCT_SHA,punctuationBank=[129,129,18,57],singleWordInk=[17,1,230,20],logicalSize=[223,21],logicalInk=[17,1,223,20],scale=3,croppedSize=[618,57],banks=[list(b)for b in BANKS],alphaSha256=hashlib.sha256(alpha).hexdigest(),rleSha256=hashlib.sha256(runs).hexdigest(),rleBytes=len(runs),atlasSize=[1024,1024],headerSha256=hashlib.sha256(text.encode()).hexdigest(),shaderSha256=hashlib.sha256(shader.encode()).hexdigest())
 return {'src/WarningWordPixels.inc':text.encode(),'src/WarningWordShader.inc':shader.encode(),'assets/hud/warning-generated.json':(json.dumps(meta,indent=2)+'\n').encode()}
def generate(check=False,root=None):
 root=ROOT if root is None else Path(root)
 for name,data in outputs(root).items():
  p=root/name
  if check:
   if p.read_bytes()!=data:raise ValueError('Stale warning output '+name)
  else:p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--check',action='store_true');a=p.parse_args();generate(a.check);print('PASS warning output pins'if a.check else 'Generated fixed warning word pixels')
