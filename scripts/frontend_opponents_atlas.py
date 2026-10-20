"""Repack authored frontend words with fixed reviewed priority and unchanged geometry."""
# SPDX-License-Identifier: MIT
import hashlib
import math
import re
from frontend_player_atlas import _subtract


EXPECTED_WORDS=[('car.svg', 'car', 'label', 21, 'player'), ('number.svg', 'number', 'label', 49, None), ('skill-level.svg', 'skill level', 'label', 59, None), ('traffic.svg', 'traffic', 'label', 40, None), ('colon.svg', ':', 'colon', 6, 'location'), ('done.svg', 'DONE', 'button', 32, 'location'), ('cancel.svg', 'CANCEL', 'button', 46, 'location'), ('class-a.svg', 'class A', 'value', 44, 'player'), ('class-b.svg', 'class B', 'value', 44, 'player'), ('class-c.svg', 'class C', 'value', 44, 'player'), ('no-opponents.svg', 'no opponents', 'value', 86, None), ('ford-indigo.svg', 'Ford Indigo', 'value', 70, 'player'), ('mustang-mach-iii.svg', 'Mustang Mach III', 'value', 105, 'player'), ('jaguar-xj220.svg', 'Jaguar XJ220', 'value', 86, 'player'), ('lotus-gt1.svg', 'Lotus GT1', 'value', 63, 'player'), ('lotus-esprit-v8.svg', 'Lotus Esprit V8', 'value', 93, 'player'), ('nazca-c2.svg', 'Nazca C2', 'value', 58, 'player'), ('italdesign-cala.svg', 'Italdesign Cal\xe0', 'value', 91, 'player'), ('isdera-112i.svg', 'Isdera 112i', 'value', 68, 'player'), ('mclaren-f1.svg', 'McLaren F1', 'value', 72, 'player'), ('ferrari-f50.svg', 'Ferrari F50', 'value', 68, 'player'), ('ferrari-355-f1.svg', 'Ferrari 355 F1', 'value', 86, 'player'), ('ford-gt90.svg', 'Ford GT90', 'value', 65, 'player'), ('full-grid.svg', 'full grid', 'value', 49, None), ('none.svg', 'none', 'value', 33, None), ('one.svg', 'one', 'value', 25, None), ('beginner.svg', 'beginner', 'value', 56, None), ('advanced.svg', 'advanced', 'value', 60, None), ('off.svg', 'off', 'value', 20, 'setup'), ('on.svg', 'on', 'value', 18, 'setup')]


def validate_words(words):
    if not isinstance(words,list) or len(words)!=30:raise ValueError('Unexpected reviewed word inventory')
    total=0
    for word,expected in zip(words,EXPECTED_WORDS):
        if tuple(word[k] for k in ('file','text','kind','width','sharedFamily'))!=expected:
            raise ValueError('Unreviewed word, width, role or shared source')
        caps=word['kind']=='button'
        proportions=('Bebas Neue',25,.75,21) if caps else ('Oxanium',24,.54,20)
        if tuple(word[k] for k in ('font','size','condensation','baseline'))!=proportions:
            raise ValueError('Unreviewed font proportions')
        if not 1<=len(word['placements'])<=32:raise ValueError('Unbounded original placements')
        for p in word['placements']:
            if not all(isinstance(p[k],(int,float)) and math.isfinite(p[k]) for k in ('x','y','right','width')):
                raise ValueError('Nonfinite original word geometry')
            if not (0<=p['x']<p['right']<=640 and p['y'] in (144,145,170,171,196,197,222,223,265,266,319,349,379,409) and p['width']==p['right']-p['x']):
                raise ValueError('Invalid original word bounds')
            align='center' if caps else 'left' if word['kind']=='value' and p['x']>=365 else 'right'
            if p['alignment']!=align:raise ValueError('Unreviewed alignment')
            if len(p['glyphs'])!=len(word['text']):raise ValueError('Incomplete original glyph sequence')
            at=p['x']
            for char,g in zip(word['text'],p['glyphs']):
                code=ord(char);page=int(code==224);cell=121 if page else code-4
                if g['sourcePage']!=page or g['cell']!=cell or g['drawText']!=word['text']:
                    raise ValueError('Unrelated glyph or source page')
                if not all(isinstance(g[k],(int,float)) and math.isfinite(g[k]) for k in ('x','y','w','h','left','right')):
                    raise ValueError('Nonfinite original glyph')
                if not(g['x']==at and g['y']==p['y'] and 0<g['w']<=32 and g['h']==51):
                    raise ValueError('Invalid original glyph geometry')
                if abs(g['left']-(at-p['x'])/p['width'])>1e-12 or abs(g['right']-(at+g['w']-p['x'])/p['width'])>1e-12:
                    raise ValueError('Invalid compatibility partition')
                at+=g['w'];total+=1
            if at!=p['right']:raise ValueError('Incomplete word extent')
    if total!=666:raise ValueError('Unexpected reviewed glyph coverage')

def repack_words(directory, opponents_directory, words, old_pixels,
                 old_rows, priority, rasterize, identity):
    import json
    validate_words(words)
    parsed=[]
    for row in old_rows:
        fields=row.strip('{}').split(',')
        if len(fields)!=14:raise ValueError('Unexpected original word row')
        values=list(map(float,fields[:-1]))
        token=int(re.fullmatch(r'UINT64_C\(0x([0-9a-f]+)\)',fields[-1])[1],16)
        if not all(math.isfinite(v) for v in values):raise ValueError('Nonfinite original word row')
        parsed.append((values,token))
    if len(parsed)!=1267 or len(old_pixels)!=8388608 or len(words)!=30:
        raise ValueError('Unexpected reviewed frontend coverage')
    images={};old_keys=[];index=0
    for family in ('main','location','setup','player'):
        folder=directory if family=='main' else directory/family
        catalog=json.loads((folder/'catalog.json').read_text())['words']
        for word in catalog:
            shared=word.get('sharedFamily') or ('location' if word.get('shared') else family)
            source=directory/word['file'] if family=='main' else directory/shared/word['file']
            key=hashlib.sha256(source.read_bytes()).hexdigest()
            count=len(word['glyphs']) if family=='main' else sum(len(p['glyphs']) for p in word['placements'])
            rows=parsed[index:index+count]
            if len(rows)!=count or not count:raise ValueError('Incomplete prior word rows')
            v=rows[0][0];x,y,w,h,top=v[7]*4,v[8]*8,v[9]*4,v[12]*4,v[11]*4
            if not all(n==int(n) for n in (x,y,w,h,top)):raise ValueError('Nonintegral source rectangle')
            x,y,w,h,top=map(int,(x,y,w,h,top))
            if not (0<=x<x+w<=1024 and 0<=y<y+h<=2048 and w==word['width']*4):raise ValueError('Invalid prior source rectangle')
            if any((r[0][7],r[0][8],r[0][9],r[0][11],r[0][12])!=(v[7],v[8],v[9],v[11],v[12]) for r in rows):raise ValueError('Prior word source changed')
            rgba=b''.join(old_pixels[((y+line)*1024+x)*4:((y+line)*1024+x+w)*4] for line in range(h))
            image=dict(width=w,height=h,top=top,rgba=rgba)
            if key in images and images[key]!=image:raise ValueError('Shared source pixels disagree')
            images[key]=image;old_keys.extend([key]*count);index+=count
    if index!=1267 or len(images)!=74:raise ValueError('Unexpected prior authored image inventory')
    word_keys={}
    for word in words:
        name=word['file']
        if not re.fullmatch(r'[a-z0-9-]+\.svg',name):raise ValueError('Invalid authored word path')
        family=word['sharedFamily']
        if family not in (None,'location','setup','player'):raise ValueError('Unknown shared family')
        path=directory/family/name if family else opponents_directory/name
        data=path.read_bytes();key=hashlib.sha256(data).hexdigest();width=word['width']*4
        if key not in images:
            rgba=rasterize(path,width,204)
            if len(rgba)!=width*204*4:raise ValueError('Incorrect authored raster size')
            ys=[y for y in range(204) for x in range(width) if rgba[(y*width+x)*4+3]]
            if not ys:raise ValueError('Empty authored word')
            top,bottom=max(0,min(ys)-2),min(204,max(ys)+3)
            if bottom>104:raise ValueError('Authored word crosses next row')
            images[key]=dict(width=width,height=bottom-top,top=top,rgba=rgba[top*width*4:bottom*width*4])
        if images[key]['width']!=width:raise ValueError('Shared width differs')
        word_keys[word['file']]=key
    if len(images)!=83 or len(priority)!=83 or set(priority)!=set(images):raise ValueError('Reviewed packing priority differs')
    placed={};free=[(0,0,1024,2048)]
    for key in priority:
        item=images[key];w,h=item['width']+2,item['height']+2;candidates=[]
        for x,y,fw,fh in free:
            if w<=fw and h<=fh:
                a,z=fw-w,fh-h;candidates.append(((min(a,z),max(a,z),y,x),x,y))
        if not candidates:raise ValueError('Reviewed artwork no longer fits')
        _,x,y=min(candidates);rectangle=(x,y,w,h)
        if not (0<=x<x+w<=1024 and 0<=y<y+h<=2048) or any(not(x+w<=a or a+c<=x or y+h<=z or z+d<=y) for a,z,c,d in placed.values()):raise ValueError('Overlapping authored allocations')
        placed[key]=rectangle;free=_subtract(free,rectangle)
    pixels=bytearray(8388608)
    for key,item in images.items():
        x,y,_,_=placed[key];w=item['width']
        for line in range(item['height']):
            at=((y+line)*1024+x)*4;pixels[at:at+w*4]=item['rgba'][line*w*4:(line+1)*w*4]
    output=[]
    for (values,token),key in zip(parsed,old_keys):
        v=values[:];x,y,_,_=placed[key];v[7:9]=[x/4,y/8];output.append((v,token))
    for word in words:
        key=word_keys[word['file']];item=images[key];x,y,_,_=placed[key];width=word['width']
        ink_x=[xx for yy in range(item['height']) for xx in range(item['width']) if item['rgba'][(yy*item['width']+xx)*4+3]]
        for p in word['placements']:
            target=p['right']-width if p['alignment']=='right' else p['x'] if p['alignment']=='left' else p['x']+(p['width']-width)/2
            candidates=[(abs(round(target*4)/4+s/4-target),round(target*4)/4+s/4) for s in range(-32,33) if p['x']<round(target*4)/4+s/4+(min(ink_x)+.5)/4 and round(target*4)/4+s/4+(max(ink_x)+.5)/4<p['right']]
            if not candidates:raise ValueError('Authored ink exceeds original bounds')
            left=min(candidates)[1]
            for g in p['glyphs']:
                v=[g[k] for k in ('cell','x','y','w','h','left','right')]+[x/4,y/8,width,left,item['top']/4,item['height']/4]
                if not all(math.isfinite(n) for n in v):raise ValueError('Nonfinite output glyph')
                output.append((v,identity(g['drawText'])))
    if len(output)!=1933:raise ValueError('Unexpected final glyph coverage')
    rows=['{'+','.join(format(v,'.12g') for v in values)+',UINT64_C(0x'+format(token,'016x')+')}' for values,token in output]
    return pixels,rows,placed

def opponents_words(directory, old_pixels, old_rows, hashes, identity):
    """Read reviewed authored sources; preserve prior positions and ink metrics."""
    import json
    from hud_font_environment import project_fonts
    from generate_remaster_pixels import source
    catalog_path=directory/'opponents/catalog.json'
    recipe_path=directory/'opponents/packing.json'
    if catalog_path.stat().st_size>1048576 or recipe_path.stat().st_size>16384:
        raise ValueError('Unbounded authored source catalog')
    words=json.loads(catalog_path.read_text())['words']
    validate_words(words)
    recipe=json.loads(recipe_path.read_text())
    if set(recipe)!={'gutterPixels','dimensions','imageSha256Priority','heuristic','atlasSha256'} or recipe['gutterPixels']!=2 or recipe['dimensions']!=[1024,2048] or recipe['heuristic']!='short':
        raise ValueError('Unexpected reviewed packing recipe')
    priority=recipe['imageSha256Priority']
    if not isinstance(priority,list) or len(priority)!=83 or len(set(priority))!=83 or any(not isinstance(v,str) or not re.fullmatch('[0-9a-f]{64}',v) for v in priority):
        raise ValueError('Invalid reviewed image priority')
    with project_fonts():
        pixels,rows,placed=repack_words(directory,directory/'opponents',words,old_pixels,old_rows,priority,
                                       lambda path,w,h:source(path,w,h,hashes),identity)
    if hashlib.sha256(pixels).hexdigest()!=recipe['atlasSha256']:
        raise ValueError('Reviewed packed artwork changed')
    output={};index=0
    for family in ('main','location','setup','player','opponents'):
        if family=='opponents':catalog=words
        else:catalog=json.loads((directory/('catalog.json' if family=='main' else family+'/catalog.json')).read_text())['words']
        entries=[]
        for word in catalog:
            shared=word.get('sharedFamily') or ('location' if word.get('shared') else family)
            path=directory/word['file'] if family=='main' else directory/shared/word['file']
            key=hashlib.sha256(path.read_bytes()).hexdigest();x,y,_,_=placed[key]
            first=list(map(float,rows[index].strip('{}').split(',')[:-1]))
            width,height,top=int(first[9]*4),int(first[12]*4),int(first[11]*4)
            if width!=word['width']*4:raise ValueError('Packed metadata width differs')
            entries.append(dict(file=word['file'],atlas=[x,y,width,height],sourceCrop=[0,top,width,height],inkFitsOriginalHorizontalTextBounds=True))
            index+=len(word['glyphs']) if family=='main' else sum(len(p['glyphs']) for p in word['placements'])
        output[family+'Placements']=entries
    if index!=1933:raise ValueError('Incomplete generated placement metadata')
    output.update(catalogSha256=hashlib.sha256(catalog_path.read_bytes()).hexdigest(),recipeSha256=hashlib.sha256(recipe_path.read_bytes()).hexdigest(),
                  packing=dict(logicalWords=112,uniqueImages=83,dimensions=[1024,2048],mainCropChanged=False,
                               oldGlyphGeometryAndInkMetricsPreserved=1267,gutterPixels=2,reviewedFixedPriority=True,
                               allocationsNonoverlapping=True,allPriorSourceRectanglesExact=74))
    return pixels,rows,output
