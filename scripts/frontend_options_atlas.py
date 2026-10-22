"""Add authored Options words by repacking only immutable authored frontend pixels.

Original source-cell geometry, text identities, output positions, font proportions
and hit areas remain unchanged. Eight Main words lose only fully zero lower rows;
all current visible RGBA and the existing 8 MiB texture allocation are preserved.
"""
# SPDX-License-Identifier: MIT
import hashlib
import json
import math
import re
from frontend_player_atlas import _subtract

FAMILIES=("main","location","setup","player","opponents")
EXPECTED_WORDS=(
 ("speedometer.svg","speedometer","label",83,None),
 ("colon.svg",":","colon",6,"location"),
 ("mph.svg","mph","value",29,None),("kmh.svg","kmh","value",29,None),
 ("personal-stats.svg","personal stats","label",88,None),
 ("on.svg","on","value",18,"setup"),("off.svg","off","value",20,"setup"),
 ("done.svg","DONE","button",32,"location"),
 ("audio.svg","audio...","label",46,None),
 ("controls.svg","controls...","label",63,None),
 ("load-save.svg","load/save...","label",72,None),
 ("graphics.svg","graphics...","label",64,None),
 ("credits.svg","credits...","label",54,None))
EXPECTED_PLACEMENTS=(
 ((263,145,84),(262,144,84)),
 ((355,145,4),(354,144,4),(355,275,4),(354,274,4)),
 ((366,145,27),(365,144,27)),((366,145,26),(365,144,26)),
 ((255,275,92),(254,274,92)),
 ((366,275,16),(365,274,16)),((366,275,18),(365,274,18)),
 ((312,344,35),(311,343,35)),
 ((310,170,48),(311,171,48)),((294,196,64),(295,197,64)),
 ((282,222,76),(283,223,76)),((291,248,67),(292,249,67)),
 ((302,300,56),(303,301,56)))

# Original small-font mode1 glyph advances, independently fixed for this catalog.
SOURCE_WIDTHS={'s': 7.0, 'p': 8.0, 'e': 8.0, 'd': 8.0, 'o': 8.0, 'm': 11.0, 't': 5.0, 'r': 5.0, ':': 4.0, 'h': 8.0, 'k': 7.0, 'n': 8.0, 'a': 8.0, 'l': 4.0, ' ': 4.0, 'f': 5.0, 'D': 9.0, 'O': 9.0, 'N': 9.0, 'E': 8.0, 'u': 8.0, 'i': 4.0, '.': 4.0, 'c': 7.0, '/': 6.0, 'v': 7.0, 'g': 8.0}

def _hash(data):return hashlib.sha256(data).hexdigest()
def _finite(v):return isinstance(v,(int,float)) and not isinstance(v,bool) and math.isfinite(v)
def _hex(v):return isinstance(v,str) and re.fullmatch(r"[0-9a-f]{64}",v) is not None

def validate_words(words):
    if not isinstance(words,list) or len(words)!=13:raise ValueError("Unexpected Options word inventory")
    total=0
    for word,expected,placements in zip(words,EXPECTED_WORDS,EXPECTED_PLACEMENTS):
        if tuple(word.get(k) for k in ("file","text","kind","width","sharedFamily"))!=expected:
            raise ValueError("Unreviewed Options word, role, width or shared source")
        caps=word['kind']=='button'
        if tuple(word.get(k)for k in ('font','size','condensation','baseline'))!=(("Bebas Neue",25,.75,21)if caps else("Oxanium",24,.54,20)):
            raise ValueError("Unreviewed Options font proportions")
        ps=word.get('placements')
        if not isinstance(ps,list)or len(ps)!=len(placements):raise ValueError("Incomplete Options placements")
        shadows=(True,False,True,False)if word['file']=='colon.svg'else(False,True)if word['file']in('audio.svg','controls.svg','load-save.svg','graphics.svg','credits.svg')else(True,False)
        for p,expected_place,shadow in zip(ps,placements,shadows):
            if any(not _finite(p.get(k))for k in ('x','y','right','width','authoredLeft'))or tuple(p[k]for k in ('x','y','width'))!=expected_place or p['right']!=p['x']+p['width']:
                raise ValueError("Changed original Options extent")
            if type(p.get('shadow'))is not bool or p['shadow']!=shadow:raise ValueError("Changed original Options shadow pass")
            align='left'if word['kind']=='value'else'center'if caps else'right'
            if p.get('alignment')!=align:raise ValueError("Unreviewed Options alignment")
            glyphs=p.get('glyphs')
            if not isinstance(glyphs,list)or len(glyphs)!=len(word['text']):raise ValueError("Incomplete Options glyph sequence")
            at=p['x'];offset=0
            for char,g in zip(word['text'],glyphs):
                if g.get('cell')!=ord(char)-4 or g.get('sourcePage')!=0:raise ValueError("Changed Options source glyph")
                if any(not _finite(g.get(k))for k in ('x','y','w','h','left','right'))or not(g['x']==at and g['y']==p['y']and g['w']==SOURCE_WIDTHS[char] and g['h']==51):raise ValueError("Changed original Options glyph geometry")
                if abs(g['left']-(at-p['x'])/p['width'])>1e-12 or abs(g['right']-(at+g['w']-p['x'])/p['width'])>1e-12:raise ValueError("Changed Options glyph partition")
                text=word['text'];draw=text[:-3]if text.endswith('...')and offset<len(text)-3 else'...'if text.endswith('...')else text
                if g.get('drawText')!=draw:raise ValueError("Unrelated Options complete draw string")
                at+=g['w'];offset+=1;total+=1
            if at!=p['right']:raise ValueError("Incomplete original Options word extent")
    if total!=188:raise ValueError("Unexpected Options glyph coverage")

def _rows(rows):
    if not isinstance(rows,list)or len(rows)!=1933:raise ValueError("Unexpected prior frontend glyph coverage")
    out=[]
    for row in rows:
        fields=row.strip('{}').split(',')
        if len(fields)!=14:raise ValueError("Unexpected prior frontend word row")
        v=list(map(float,fields[:-1]));token=re.fullmatch(r'UINT64_C\(0x([0-9a-f]{16})\)',fields[-1])
        if not token or not all(math.isfinite(x)for x in v):raise ValueError("Invalid prior frontend geometry/context")
        out.append((v,int(token[1],16)))
    return out

def _recipe(recipe):
    if not isinstance(recipe,dict)or set(recipe)!={'gutterPixels','dimensions','heuristic','imageSha256Priority','previousAtlasSha256','previousRowsSha256','catalogSha256','mainTrailingCrop','atlasSha256'}or recipe['gutterPixels']!=2 or recipe['dimensions']!=[1024,2048]or recipe['heuristic']!='area':raise ValueError("Unexpected Options packing recipe")
    priority=recipe['imageSha256Priority'];trims=recipe['mainTrailingCrop']
    if not isinstance(priority,list)or len(priority)!=92 or len(set(priority))!=92 or not all(_hex(v)for v in priority):raise ValueError("Invalid Options image priority")
    if not all(_hex(recipe[k])for k in ('previousAtlasSha256','previousRowsSha256','catalogSha256','atlasSha256'))or not isinstance(trims,dict)or len(trims)!=8:raise ValueError("Incomplete Options packing hashes")
    for key,value in trims.items():
        if not _hex(key)or not isinstance(value,dict)or set(value)!={'height','rgbaSha256'}or type(value['height'])is not int or not 2<=value['height']<204 or not _hex(value['rgbaSha256']):raise ValueError("Invalid Main transparent crop proof")

def repack_options_words(directory,words,old_pixels,old_rows,prior,recipe,rasterize,identity):
    """Pure composition after authenticated source loading; no files are written."""
    validate_words(words);_recipe(recipe);parsed=_rows(old_rows)
    if _hash(('\n'.join(old_rows)+'\n').encode())!=recipe['previousRowsSha256']:raise ValueError("Prior reviewed frontend glyph geometry differs")
    if len(old_pixels)!=8388608 or _hash(old_pixels)!=recipe['previousAtlasSha256']:raise ValueError("Prior reviewed frontend raster differs")
    images={};old_keys=[];index=0;main_keys=set()
    for family in FAMILIES:
        folder=directory if family=='main'else directory/family
        catalog=json.loads((folder/'catalog.json').read_text())['words'];places=prior.get(family+'Placements')
        if not isinstance(places,list)or len(places)!=len(catalog):raise ValueError("Incomplete prior frontend metadata")
        for word,place in zip(catalog,places):
            if not isinstance(word.get('file'),str)or not re.fullmatch(r'[a-z0-9-]+\.svg',word['file']):raise ValueError("Invalid prior authored source filename")
            shared=word.get('sharedFamily')or('location'if word.get('shared')else family)
            if shared not in FAMILIES:raise ValueError("Invalid prior shared family")
            source=directory/word['file']if family=='main'else directory/shared/word['file'];key=_hash(source.read_bytes())
            count=len(word['glyphs'])if family=='main'else sum(len(p['glyphs'])for p in word['placements']);rows=parsed[index:index+count]
            if not rows or len(rows)!=count:raise ValueError("Incomplete prior word rows")
            v=rows[0][0];x,y,w,h,top=v[7]*4,v[8]*8,v[9]*4,v[12]*4,v[11]*4
            if not all(n==int(n)for n in (x,y,w,h,top)):raise ValueError("Nonintegral prior source rectangle")
            x,y,w,h,top=map(int,(x,y,w,h,top))
            if not(0<=x<x+w<=1024 and 0<=y<y+h<=2048 and w==word['width']*4)or place.get('file')!=word['file']or place.get('atlas')!=[x,y,w,h]or place.get('sourceCrop')!=[0,top,w,h]:raise ValueError("Prior frontend metadata/source disagrees")
            if any((r[0][7],r[0][8],r[0][9],r[0][11],r[0][12])!=(v[7],v[8],v[9],v[11],v[12])for r in rows):raise ValueError("Prior word source changed")
            rgba=b''.join(old_pixels[((y+line)*1024+x)*4:((y+line)*1024+x+w)*4]for line in range(h));image=dict(width=w,height=h,top=top,rgba=rgba)
            if key in images and images[key]!=image:raise ValueError("Shared authored source pixels disagree")
            images[key]=image;old_keys.extend([key]*count);index+=count
            if family=='main':main_keys.add(key)
    if index!=1933 or len(images)!=83 or len(main_keys)!=8 or set(recipe['mainTrailingCrop'])!=main_keys:raise ValueError("Unexpected prior authored image inventory")
    old_images={k:dict(v)for k,v in images.items()};trims=[]
    for key in sorted(main_keys):
        i=images[key];w,h=i['width'],i['height'];proof=recipe['mainTrailingCrop'][key]
        if h!=204 or i['top']!=0:raise ValueError("Prior Main image geometry changed")
        occupied=[y for y in range(h)if any(i['rgba'][(y*w+x)*4+3]for x in range(w))]
        if not occupied:raise ValueError("Empty Main image")
        bottom=max(occupied)+3
        if bottom!=proof['height']or bottom>=h or any(i['rgba'][bottom*w*4:]):raise ValueError("Main tail is not the reviewed transparent crop")
        prefix=i['rgba'][:bottom*w*4]
        if _hash(prefix)!=proof['rgbaSha256']:raise ValueError("Main retained pixels changed")
        trims.append(dict(sourceSha256=key,oldHeight=h,newHeight=bottom,removedRgbaBytes=(h-bottom)*w*4,allRemovedZero=True,clearRowsAfterLastAlpha=2))
        i['height']=bottom;i['rgba']=prefix
    word_keys={}
    for word in words:
        family=word['sharedFamily'];path=directory/family/word['file']if family else directory/'options'/word['file'];data=path.read_bytes();key=_hash(data);width=word['width']*4
        if key not in images:
            rgba=rasterize(path,width,204)
            if len(rgba)!=width*204*4:raise ValueError("Incorrect Options authored raster size")
            occupied=[y for y in range(204)if any(rgba[(y*width+x)*4+3]for x in range(width))]
            if not occupied:raise ValueError("Empty Options artwork")
            top,bottom=max(0,min(occupied)-2),min(204,max(occupied)+3)
            if bottom>104:raise ValueError("Options artwork crosses next source row")
            images[key]=dict(width=width,height=bottom-top,top=top,rgba=rgba[top*width*4:bottom*width*4])
        if images[key]['width']!=width:raise ValueError("Shared Options image width differs")
        word_keys[word['file']]=key
    priority=recipe['imageSha256Priority']
    if len(images)!=92 or set(images)!=set(priority):raise ValueError("Options reviewed image inventory differs")
    placed={};free=[(0,0,1024,2048)]
    for key in priority:
        i=images[key];w,h=i['width']+2,i['height']+2;choices=[]
        for x,y,fw,fh in free:
            if w<=fw and h<=fh:choices.append(((fw*fh-w*h,min(fw-w,fh-h),y,x),x,y))
        if not choices:raise ValueError("Reviewed Options artwork no longer fits")
        _,x,y=min(choices)
        if not(0<=x<x+w<=1024 and 0<=y<y+h<=2048)or any(not(x+w<=a or a+c<=x or y+h<=z or z+d<=y)for a,z,c,d in placed.values()):raise ValueError("Overlapping Options authored allocations")
        placed[key]=(x,y,w,h);free=_subtract(free,(x,y,w,h))
    pixels=bytearray(8388608)
    for key,i in images.items():
        x,y,_,_=placed[key];w=i['width']
        for line in range(i['height']):
            at=((y+line)*1024+x)*4;pixels[at:at+w*4]=i['rgba'][line*w*4:(line+1)*w*4]
    if _hash(pixels)!=recipe['atlasSha256']:raise ValueError("Reviewed Options atlas digest differs")
    output=[]
    for (values,token),key in zip(parsed,old_keys):
        v=values[:];x,y,_,_=placed[key];v[7:9]=[x/4,y/8]
        if key in main_keys:v[12]=images[key]['height']/4
        output.append((v,token))
    options_placements=[]
    for word in words:
        key=word_keys[word['file']];i=images[key];x,y,_,_=placed[key];width=word['width'];ink_x=[xx for yy in range(i['height'])for xx in range(i['width'])if i['rgba'][(yy*i['width']+xx)*4+3]]
        for p in word['placements']:
            target=p['x']if p['alignment']=='left'else p['x']+(p['width']-width)/2 if p['alignment']=='center'else p['right']-width
            candidates=[(abs(round(target*4)/4+n/4-target),round(target*4)/4+n/4)for n in range(-32,33)if p['x']<round(target*4)/4+n/4+(min(ink_x)+.5)/4 and round(target*4)/4+n/4+(max(ink_x)+.5)/4<p['right']]
            if not candidates:raise ValueError("Options authored ink exceeds original bounds")
            left=min(candidates)[1]
            if left!=p['authoredLeft']:raise ValueError("Reviewed Options authored alignment changed")
            for g in p['glyphs']:
                v=[g[k]for k in ('cell','x','y','w','h','left','right')]+[x/4,y/8,width,left,i['top']/4,i['height']/4]
                if not all(math.isfinite(n)for n in v):raise ValueError("Nonfinite Options output glyph")
                output.append((v,identity(g['drawText'])))
        options_placements.append(dict(file=word['file'],atlas=[x,y,i['width'],i['height']],sourceCrop=[0,i['top'],i['width'],i['height']],inkFitsOriginalHorizontalTextBounds=True))
    if len(output)!=2121:raise ValueError("Unexpected final Options glyph coverage")
    rows=['{'+','.join(format(v,'.12g')for v in values)+',UINT64_C(0x'+format(token,'016x')+')}'for values,token in output]
    metadata={};index=0
    for family in FAMILIES:
        folder=directory if family=='main'else directory/family;catalog=json.loads((folder/'catalog.json').read_text())['words'];entries=[]
        for word in catalog:
            count=len(word['glyphs'])if family=='main'else sum(len(p['glyphs'])for p in word['placements']);key=old_keys[index];i=images[key];x,y,_,_=placed[key]
            entries.append(dict(file=word['file'],atlas=[x,y,i['width'],i['height']],sourceCrop=[0,i['top'],i['width'],i['height']],inkFitsOriginalHorizontalTextBounds=True));index+=count
        metadata[family+'Placements']=entries
    if index!=1933:raise ValueError("Incomplete final prior placement metadata")
    proofs=[]
    for key,old in old_images.items():
        i=images[key];reconstructed=i['rgba']+bytes((old['height']-i['height'])*old['width']*4)
        if reconstructed!=old['rgba']:raise ValueError("Prior authored RGBA changed")
        x,y,_,_=placed[key];copied=b''.join(pixels[((y+j)*1024+x)*4:((y+j)*1024+x+i['width'])*4]for j in range(i['height']))
        if copied!=i['rgba']:raise ValueError("Packed authored pixels changed")
        proofs.append(dict(sourceSha256=key,oldRgbaSha256=_hash(old['rgba']),retainedRgbaSha256=_hash(i['rgba']),oldHeight=old['height'],height=i['height'],width=i['width'],reconstructedOldRgbaExact=True))
    metadata.update(optionsPlacements=options_placements,packing=dict(logicalWords=125,uniqueImages=92,dimensions=[1024,2048],gutterPixels=2,reviewedFixedPriority=True,allocationsNonoverlapping=True,originalGlyphGeometryPreserved=1933,mainTransparentTrailingCrop=trims,allPriorVisiblePixelsExact=83),preservedImageProofs=proofs,additionalCachedTextureBytesForOptions=0)
    return pixels,rows,metadata

def options_words(directory,old_pixels,old_rows,prior,hashes,identity):
    """Reviewed source loader; returns updated atlas, rows and all family placements."""
    from hud_font_environment import project_fonts
    from generate_remaster_pixels import source
    catalog_path=directory/'options/catalog.json';recipe_path=directory/'options/packing.json'
    if catalog_path.stat().st_size>1048576 or recipe_path.stat().st_size>32768:raise ValueError("Unbounded authored Options catalog")
    catalog_bytes=catalog_path.read_bytes();recipe=json.loads(recipe_path.read_text());_recipe(recipe)
    if _hash(catalog_bytes)!=recipe['catalogSha256']:raise ValueError("Reviewed Options catalog changed")
    words=json.loads(catalog_bytes)['words']
    with project_fonts():
        pixels,rows,metadata=repack_options_words(directory,words,old_pixels,old_rows,prior,recipe,lambda path,w,h:source(path,w,h,hashes),identity)
    metadata.update(catalogSha256=_hash(catalog_bytes),recipeSha256=_hash(recipe_path.read_bytes()))
    return pixels,rows,metadata
