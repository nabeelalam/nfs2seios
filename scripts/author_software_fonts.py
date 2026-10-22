# SPDX-License-Identifier: MIT
"""Generate authored software-menu glyphs from the pinned project fonts only.

Authoring environment: Pango 1.52.1, Cairo antialias=gray, hinting disabled.
No game assets or original pixels are read. Generated data is compiled into the
application; authoring libraries are not runtime or Apple-build dependencies.
"""
from pathlib import Path
import argparse, ctypes as c, ctypes.util, json, hashlib, struct
from hud_font_environment import project_fonts
ROOT=Path(__file__).resolve().parents[1]
class Rect(c.Structure):_fields_=[(n,c.c_int) for n in ('x','y','width','height')]
class FontAuthor:
 def __init__(self):
  self.p=c.CDLL(ctypes.util.find_library('pango-1.0'));self.pc=c.CDLL(ctypes.util.find_library('pangocairo-1.0'));self.ca=c.CDLL(ctypes.util.find_library('cairo'));self.go=c.CDLL(ctypes.util.find_library('gobject-2.0'))
  def f(lib,name,args,res):q=getattr(lib,name);q.argtypes=args;q.restype=res;return q
  ptr=c.c_void_p;ii=c.c_int;dd=c.c_double
  self.unref=f(self.go,'g_object_unref',[ptr],None)
  self.map=f(self.pc,'pango_cairo_font_map_new',[],ptr)();self.context=f(self.p,'pango_font_map_create_context',[ptr],ptr)(self.map)
  f(self.p,'pango_context_set_round_glyph_positions',[ptr,ii],None)(self.context,0)
  options=f(self.ca,'cairo_font_options_create',[],ptr)()
  for name,value in [('antialias',2),('hint_style',1),('hint_metrics',1)]:f(self.ca,'cairo_font_options_set_'+name,[ptr,ii],None)(options,value)
  f(self.pc,'pango_cairo_context_set_font_options',[ptr,ptr],None)(self.context,options);f(self.ca,'cairo_font_options_destroy',[ptr],None)(options)
  self.layout=f(self.p,'pango_layout_new',[ptr],ptr)(self.context)
  f(self.p,'pango_layout_set_auto_dir',[ptr,ii],None)(self.layout,0);f(self.p,'pango_layout_set_single_paragraph_mode',[ptr,ii],None)(self.layout,1)
  self.descnew=f(self.p,'pango_font_description_new',[],ptr);self.descfree=f(self.p,'pango_font_description_free',[ptr],None)
  self.setfamily=f(self.p,'pango_font_description_set_family',[ptr,c.c_char_p],None);self.setsize=f(self.p,'pango_font_description_set_absolute_size',[ptr,dd],None);self.setweight=f(self.p,'pango_font_description_set_weight',[ptr,ii],None);self.setdesc=f(self.p,'pango_layout_set_font_description',[ptr,ptr],None)
  attrs=f(self.p,'pango_attr_list_new',[],ptr)();attribute=f(self.p,'pango_attr_font_features_new',[c.c_char_p],ptr)(b'kern=1,liga=0,clig=0,dlig=0,calt=0');f(self.p,'pango_attr_list_insert',[ptr,ptr],None)(attrs,attribute);f(self.p,'pango_layout_set_attributes',[ptr,ptr],None)(self.layout,attrs);f(self.p,'pango_attr_list_unref',[ptr],None)(attrs)
  self.settext=f(self.p,'pango_layout_set_text',[ptr,c.c_char_p,ii],None);self.unknown=f(self.p,'pango_layout_get_unknown_glyphs_count',[ptr],ii)
  self.getsize=f(self.p,'pango_layout_get_size',[ptr,c.POINTER(ii),c.POINTER(ii)],None);self.pos=f(self.p,'pango_layout_index_to_pos',[ptr,ii,c.POINTER(Rect)],None);self.baseline=f(self.p,'pango_layout_get_baseline',[ptr],ii)
  self.surface=f(self.ca,'cairo_image_surface_create',[ii,ii,ii],ptr);self.sfree=f(self.ca,'cairo_surface_destroy',[ptr],None);self.ctx=f(self.ca,'cairo_create',[ptr],ptr);self.cfree=f(self.ca,'cairo_destroy',[ptr],None)
  self.translate=f(self.ca,'cairo_translate',[ptr,dd,dd],None);self.scale=f(self.ca,'cairo_scale',[ptr,dd,dd],None);self.move=f(self.ca,'cairo_move_to',[ptr,dd,dd],None);self.colour=f(self.ca,'cairo_set_source_rgba',[ptr,dd,dd,dd,dd],None);self.show=f(self.pc,'pango_cairo_show_layout',[ptr,ptr],None)
  self.flush=f(self.ca,'cairo_surface_flush',[ptr],None);self.data=f(self.ca,'cairo_image_surface_get_data',[ptr],ptr);self.stride=f(self.ca,'cairo_image_surface_get_stride',[ptr],ii);self.status=f(self.ca,'cairo_status',[ptr],ii)
  self.version=f(self.p,'pango_version_string',[],c.c_char_p)().decode()
 def font(self,family,size,weight,condensation):
  d=self.descnew();self.setfamily(d,family.encode());self.setsize(d,size*4*1024);self.setweight(d,weight);self.setdesc(self.layout,d);self.descfree(d);self.condensation=condensation
 def text(self,text):
  raw=text.encode('utf-8');self.settext(self.layout,raw,len(raw));assert self.unknown(self.layout)==0,text
  w=c.c_int();h=c.c_int();self.getsize(self.layout,c.byref(w),c.byref(h));positions=[];index=0
  for char in text:
   rect=Rect();self.pos(self.layout,index,c.byref(rect));positions.append(rect.x/1024*self.condensation/4);index+=len(char.encode('utf-8'))
  return w.value/1024*self.condensation/4,positions
 def raster(self,text,width=256,height=256,origin=64,baseline=128):
  self.text(text);surface=self.surface(0,width,height);context=self.ctx(surface)
  try:
   self.translate(context,origin,baseline);self.scale(context,self.condensation,1);self.move(context,0,-self.baseline(self.layout)/1024);self.colour(context,1,1,1,1);self.show(context,self.layout);assert self.status(context)==0;self.flush(surface)
   stride=self.stride(surface);raw=c.string_at(self.data(surface),stride*height);alpha=bytes(raw[y*stride+x*4+3] for y in range(height) for x in range(width));return alpha
  finally:self.cfree(context);self.sfree(surface)
 def close(self):
  for v in (self.layout,self.context,self.map):self.unref(v)
def bounds(p,w,h):
 points=[(i%w,i//w) for i,a in enumerate(p) if a]
 if not points:return None
 return (min(x for x,y in points),min(y for x,y in points),max(x for x,y in points)+1,max(y for x,y in points)+1)
def cut(p,w,box):x,y,r,b=box;return b''.join(p[j*w+x:j*w+r] for j in range(y,b))
def subtract(free,placed):
 px,py,pw,ph=placed;result=[]
 for x,y,w,h in free:
  if px>=x+w or px+pw<=x or py>=y+h or py+ph<=y:result.append((x,y,w,h));continue
  if px>x:result.append((x,y,px-x,h))
  if px+pw<x+w:result.append((px+pw,y,x+w-px-pw,h))
  if py>y:result.append((x,y,w,py-y))
  if py+ph<y+h:result.append((x,py+ph,w,y+h-py-ph))
 result=list(dict.fromkeys(result));return [a for i,a in enumerate(result) if not any(i!=j and a[0]>=z[0] and a[1]>=z[1] and a[0]+a[2]<=z[0]+z[2] and a[1]+a[3]<=z[1]+z[3] for j,z in enumerate(result))]
def pack(images,width,height):
 free=[(0,0,width,height)];positions={}
 for key,image in sorted(images.items(),key=lambda v:(-max(v[1]['w'],v[1]['h']),-v[1]['w']*v[1]['h'],v[0])):
  w,h=image['w']+4,image['h']+4;candidates=[(min(fw-w,fh-h),max(fw-w,fh-h),y,x,i) for i,(x,y,fw,fh) in enumerate(free) if w<=fw and h<=fh]
  if not candidates:return None
  _,_,y,x,_=min(candidates);positions[key]=(x+2,y+2);free=subtract(free,(x,y,w,h))
 return positions
def generate():
 codes=list(range(32,127))+[224];images={};roles=[]
 with project_fonts():
  a=FontAuthor()
  try:
   if a.version!='1.52.1':raise ValueError('Pinned Pango authoring version differs')
   for role,family,size,weight,cond,baseline in [('heading','Bebas Neue',25,400,.75,23),('body','Oxanium',24,500,.54,20)]:
    a.font(family,size,weight,cond);glyphs=[];advances=[]
    for code in codes:
     advance,_=a.text(chr(code));advances.append(advance);rgba=a.raster(chr(code));box=bounds(rgba,256,256)
     if box:
      assert box[0]>0 and box[1]>0 and box[2]<256 and box[3]<256
      pixels=cut(rgba,256,box);width,height=box[2]-box[0],box[3]-box[1];key=hashlib.sha256(width.to_bytes(2,'little')+height.to_bytes(2,'little')+pixels).hexdigest();images.setdefault(key,dict(w=width,h=height,alpha=pixels))
      glyphs.append(dict(image=key,left=box[0]-64,top=box[1]-128,w=width,h=height,advance=round(advance*65536)))
     else:
      assert code==32;glyphs.append(dict(image=None,left=0,top=0,w=0,h=0,advance=round(advance*65536)))
    kern=[]
    for i,first in enumerate(codes):
     for j,second in enumerate(codes):
      width,positions=a.text(chr(first)+chr(second));delta=width-advances[i]-advances[j]
      if abs(delta-(positions[1]-advances[i]))>1/16384:raise ValueError('Pair layout is not representable')
      kern.append(round(delta*65536))
    roles.append(dict(role=role,family=family,size=size,weight=weight,condensation=cond,baseline=baseline,glyphs=glyphs,kern=kern))
  finally:a.close()
 positions=pack(images,1024,512)
 if positions is None:raise ValueError('Reviewed atlas dimensions exceeded')
 atlas=bytearray(1024*512)
 for key,(x,y) in positions.items():
  image=images[key]
  for row in range(image['h']):atlas[(y+row)*1024+x:(y+row)*1024+x+image['w']]=image['alpha'][row*image['w']:(row+1)*image['w']]
 for role in roles:
  for glyph in role['glyphs']:glyph['x'],glyph['y']=positions[glyph['image']] if glyph['image'] else (0,0)
 encoded=bytearray();at=0
 while at<len(atlas):
  end=at+1
  while end<len(atlas) and atlas[end]==atlas[at] and end-at<65535:end+=1
  encoded+=struct.pack('<HB',end-at,atlas[at]);at=end
 metadata=bytearray()
 for role in roles:
  for glyph in role['glyphs']:metadata+=struct.pack('<6hi',*(glyph[k] for k in ('x','y','w','h','left','top','advance')))
 for role in roles:metadata+=struct.pack('<9216i',*role['kern'])
 metadata+=struct.pack('<2i',*(role['baseline'] for role in roles))
 coverageSha=hashlib.sha256(atlas).digest();metadataSha=hashlib.sha256(metadata).digest();identity=int.from_bytes(hashlib.sha256(metadata+atlas).digest()[:8],'little')
 lines=['// Generated by scripts/author_software_fonts.py; no original game pixels.', '// Glyphs derived from pinned Bebas Neue and Oxanium; SIL OFL 1.1.', '// See assets/hud/fonts/*-OFL.txt and software-fonts.json.', '#pragma once','#include <cstdint>','namespace nfs_soft_authored_data {','struct Glyph {int16_t x,y,w,h,left,top;int32_t advance;};','static constexpr Glyph glyphs[2][96]={']
 for role in roles:
  lines.append('{')
  for glyph in role['glyphs']:lines.append('{'+','.join(str(glyph[k])for k in ('x','y','w','h','left','top','advance'))+'},')
  lines.append('},')
 lines+=['};','static constexpr int32_t kern[2][9216]={']
 for role in roles:lines+=['{'+','.join(map(str,role['kern']))+'},']
 lines+=['};','static constexpr int32_t baseline[2]={23,20};','static constexpr uint8_t coverageRle[]={']
 for at in range(0,len(encoded),60):lines.append(','.join(map(str,encoded[at:at+60]))+',')
 lines+=['};']
 for name,value in [('coverageSha256',coverageSha),('metadataSha256',metadataSha)]:lines.append('static constexpr uint8_t '+name+'[32]={'+','.join(map(str,value))+'};')
 lines+=['static constexpr uint64_t identity='+str(identity)+'ULL;','} // namespace nfs_soft_authored_data']
 source=json.loads((ROOT/'assets/hud/fonts/sources.json').read_text())
 provenance=dict(scope='Authored software-menu glyph coverage; no original game pixels',fonts=[font for font in source['fonts'] if font['family'] in ('Bebas Neue','Oxanium')],roles=[{k:role[k]for k in ('role','family','size','weight','condensation','baseline')}for role in roles],pangoVersion='1.52.1',features='kern=1,liga=0,clig=0,dlig=0,calt=0',glyphPositionsRounded=False,atlasSize=[1024,512],codes=codes,coverageSha256=coverageSha.hex(),metadataSha256=metadataSha.hex(),metadataEncoding='LE role-major96*(six int16 + int32 advance), role-major9216 int32 kern, two int32 baselines',coverageRleBytes=len(encoded),identity=identity,originalGameDataRead=False)
 return ('\n'.join(lines)+'\n').encode(),(json.dumps(provenance,indent=2)+'\n').encode(),bytes(atlas),bytes(metadata)

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
 header,provenance,_,_=generate()
 for relative,data in [('src/hd/SoftwareFontData.h',header),('assets/hud/fonts/software-fonts.json',provenance)]:
  path=ROOT/relative
  if args.check:
   if not path.exists() or path.read_bytes()!=data:raise ValueError('Generated software-font source differs: '+relative)
  else:path.write_bytes(data)
 print(json.dumps(dict(generatedHeaderSha256=hashlib.sha256(header).hexdigest(),provenanceSha256=hashlib.sha256(provenance).hexdigest(),checked=args.check)))
if __name__=='__main__':main()
