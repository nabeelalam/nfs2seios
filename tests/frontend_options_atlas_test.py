# SPDX-License-Identifier: MIT
"""Pure authored-atlas composition checks; no original game files or renderer."""
import copy
import hashlib
import json
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from frontend_options_atlas import repack_options_words,validate_words,_recipe,FAMILIES
from frontend_player_atlas import _subtract
from frontend_pixel_limits import FRONTEND_PIXEL_MAX_PARTS
from hud_font_environment import project_fonts
from rasterize_hud_svg import rasterize
DIR=ROOT/'assets/hud/frontend'
def sha(b):return hashlib.sha256(b).hexdigest()
def identity(text):
    h=14695981039346656037
    for c in text.encode('latin1'):h=((h^c)*1099511628211)&0xffffffffffffffff
    return h

def decode_current(*, full=False):
    header=(ROOT/'src/FrontendFontPixels.inc').read_text();names=re.findall(r'#include "(FrontendFontPixelsPart[0-9]+\.inc)"',header)
    if not 1<=len(names)<=FRONTEND_PIXEL_MAX_PARTS:raise AssertionError('Bounded current generated part inventory')
    encoded=bytearray()
    for name in names:
        body=(ROOT/'src'/name).read_text().split('\n',1)[1]
        encoded.extend(int(n)for n in re.findall(r'\b[0-9]+\b',body))
    if len(encoded)%6:raise AssertionError('RLE unit count')
    pixels=bytearray()
    for at in range(0,len(encoded),6):
        n=encoded[at]|(encoded[at+1]<<8)
        if not n or len(pixels)+4*n>8683520:raise AssertionError('Bounded generated raster')
        pixels.extend(encoded[at+2:at+6]*n)
    if len(pixels)!=8683520:raise AssertionError('Full generated raster')
    # Existing legacy compositor tests intentionally verify the unchanged prefix.
    return pixels if full else pixels[:8388608]

def fixture():
    """Reconstruct historical packing input before the fixed text overrides.

    Re-render only overridden Main/Player/shared SVG sources with pinned fonts;
    retain every independent old recipe digest and geometry assertion. Separate
    fixed-art tests verify the later overrides and alias inventory.
    """
    current=decode_current();meta=json.loads((DIR/'generated.json').read_text());assert sha(current)==meta['legacyRasterSha256']
    from frontend_player_word_art import WORDS
    overridden = {alias for word in WORDS for alias in word[-1]}
    from frontend_player_heading_art import WORDS as HEADINGS
    overridden.update(alias for word in HEADINGS for alias in word[-1])
    from frontend_remaining_word_art import WORDS as REMAINING
    overridden.update(alias for word in REMAINING for alias in word[-1])
    historical_main={}
    with project_fonts():
        for word in json.loads((DIR/'catalog.json').read_text())['words']:
            historical_main[word['file']]=rasterize((DIR/word['file']).read_bytes(),word['width']*4,204)
    assert len(historical_main)==8
    images={};groups=[];rows=[]
    for line in (ROOT/'src/FrontendWordLayout.inc').read_text().splitlines():
        if line.startswith('{'):rows.append(line.rstrip(','))
    rows=rows[:1933];parsed=[]
    for row in rows:
        f=row.strip('{}').split(',');parsed.append((list(map(float,f[:13])),f[13]))
    index=0
    for family in FAMILIES:
        catalog=json.loads((DIR/('catalog.json'if family=='main'else family+'/catalog.json')).read_text())['words']
        for word,place in zip(catalog,meta[family+'Placements']):
            shared=word.get('sharedFamily')or('location'if word.get('shared')else family);path=DIR/word['file']if family=='main'else DIR/shared/word['file'];key=sha(path.read_bytes())
            x,y,w,h=place['atlas'];raw=b''.join(current[((y+j)*1024+x)*4:((y+j)*1024+x+w)*4]for j in range(h));top=place['sourceCrop'][1]
            if family=='main':
                raw=historical_main[word['file']];h=204
                assert len(raw)==w*h*4
            elif family+'/'+word['file'] in overridden:
                with project_fonts():
                    original = rasterize(path.read_bytes(), w, 204)
                raw = original[top*w*4:(top+h)*w*4]
                assert len(raw)==w*h*4
            item=dict(width=w,height=h,top=top,rgba=raw)
            if key in images:assert images[key]==item
            images[key]=item
            count=len(word['glyphs'])if family=='main'else sum(len(p['glyphs'])for p in word['placements']);groups.append((family,word,key,index,count));index+=count
    assert index==1933 and len(images)==83
    recipe=json.loads((DIR/'opponents/packing.json').read_text());free=[(0,0,1024,2048)];placed={}
    for key in recipe['imageSha256Priority']:
        item=images[key];w,h=item['width']+2,item['height']+2;c=[]
        for x,y,fw,fh in free:
            if w<=fw and h<=fh:c.append(((min(fw-w,fh-h),max(fw-w,fh-h),y,x),x,y))
        _,x,y=min(c);placed[key]=(x,y,w,h);free=_subtract(free,(x,y,w,h))
    pixels=bytearray(8388608);prior={f+'Placements':[]for f in FAMILIES}
    for key,item in images.items():
        x,y,_,_=placed[key];w=item['width']
        for line in range(item['height']):
            at=((y+line)*1024+x)*4;pixels[at:at+w*4]=item['rgba'][line*w*4:(line+1)*w*4]
    for family,word,key,start,count in groups:
        i=images[key];x,y,_,_=placed[key];prior[family+'Placements'].append(dict(file=word['file'],atlas=[x,y,i['width'],i['height']],sourceCrop=[0,i['top'],i['width'],i['height']],inkFitsOriginalHorizontalTextBounds=True))
        for n in range(start,start+count):
            v,token=parsed[n];v[7:9]=[x/4,y/8]
            if family=='main':v[12]=51
            rows[n]='{'+','.join(format(z,'.12g')for z in v)+','+token+'}'
    options=json.loads((DIR/'options/catalog.json').read_text())['words'];new_recipe=json.loads((DIR/'options/packing.json').read_text())
    assert sha(pixels)==new_recipe['previousAtlasSha256']
    assert sha(('\n'.join(rows)+'\n').encode())==new_recipe['previousRowsSha256']
    new_rasters={}
    with project_fonts():
        for w in options:
            if not w['sharedFamily']:
                path=DIR/'options'/w['file'];new_rasters[w['file']]=rasterize(path.read_bytes(),w['width']*4,204)
    return pixels,rows,prior,options,new_recipe,new_rasters,images,groups

class Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.pixels,cls.rows,cls.prior,cls.words,cls.recipe,cls.rasters,cls.images,cls.groups=fixture()
    def run_repack(self,words=None,recipe=None,pixels=None,rows=None,prior=None,raster=None):
        return repack_options_words(DIR,self.words if words is None else words,self.pixels if pixels is None else pixels,self.rows if rows is None else rows,self.prior if prior is None else prior,self.recipe if recipe is None else recipe,raster or(lambda p,w,h:self.rasters[p.name]),identity)
    def test_exact_old_pixels_geometry_and_both_family_metadata(self):
        pixels,rows,meta=self.run_repack();self.assertEqual(sha(pixels),self.recipe['atlasSha256']);self.assertEqual(len(pixels),8388608);self.assertEqual(len(rows),2121)
        self.assertEqual(meta['additionalCachedTextureBytesForOptions'],0);self.assertEqual(meta['packing']['uniqueImages'],92)
        self.assertEqual(len(meta['preservedImageProofs']),83);self.assertTrue(all(p['reconstructedOldRgbaExact']for p in meta['preservedImageProofs']))
        for family,word,key,start,count in self.groups:
            entry=next(p for p in meta[family+'Placements']if p['file']==word['file']);x,y,w,h=entry['atlas'];raw=b''.join(pixels[((y+j)*1024+x)*4:((y+j)*1024+x+w)*4]for j in range(h));old=self.images[key]
            self.assertEqual(raw+bytes((old['height']-h)*w*4),old['rgba'])
            for n in range(start,start+count):
                before=self.rows[n].strip('{}').split(',');after=rows[n].strip('{}').split(',')
                self.assertEqual(before[:7],after[:7]);self.assertEqual(before[9:12],after[9:12]);self.assertEqual(before[13],after[13])
                if family=='main':self.assertEqual(float(after[12]),h/4)
                else:self.assertEqual(before[12],after[12])
        self.assertEqual(sum(len(w['placements'])for w in self.words),28)
        self.assertEqual(len(meta['optionsPlacements']),13)
        for before,w in zip(meta['optionsPlacements'],self.words):
            if w['sharedFamily']:
                entry=next(p for p in meta[w['sharedFamily']+'Placements']if p['file']==w['file']);self.assertEqual(before['atlas'],entry['atlas'])
    def test_options_complete_draw_text_not_combined_ellipsis(self):
        _,rows,_=self.run_repack();tokens=[r.strip('{}').split(',')[-1]for r in rows[1933:]]
        for prefix in ('audio','controls','load/save','graphics','credits'):
            self.assertIn('UINT64_C(0x'+format(identity(prefix),'016x')+')',tokens)
            self.assertNotIn('UINT64_C(0x'+format(identity(prefix+'...'),'016x')+')',tokens)
        self.assertIn('UINT64_C(0x'+format(identity('...'),'016x')+')',tokens)
    def test_catalog_mutations_reject(self):
        cases=[]
        def case(mut):
            words=copy.deepcopy(self.words);mut(words);cases.append(words)
        case(lambda w:w.pop());case(lambda w:w.__setitem__(1,copy.deepcopy(w[0])))
        for key,value in [('file','../speedometer.svg'),('text','Speedometer'),('font','other'),('kind','value'),('width',82),('condensation',.55),('sharedFamily','options')]:case(lambda w,k=key,v=value:w[0].__setitem__(k,v))
        for key,value in [('x',261),('width',83),('right',348),('alignment','center'),('shadow',False),('authoredLeft',float('nan'))]:case(lambda w,k=key,v=value:w[0]['placements'][0].__setitem__(k,v))
        for key,value in [('cell',110),('x',264),('y',144),('w',8),('h',50),('left',.1),('right',.5),('drawText','speedometer...'),('sourcePage',1)]:case(lambda w,k=key,v=value:w[0]['placements'][0]['glyphs'][0].__setitem__(k,v))
        for words in cases:
            with self.subTest(words=words[0].get('file')):
                with self.assertRaises(ValueError):validate_words(words)
    def test_recipe_mutations_reject(self):
        for key,value in [('dimensions',[2048,2048]),('gutterPixels',0),('heuristic','short'),('previousAtlasSha256','bad'),('previousRowsSha256','bad')]:
            bad=copy.deepcopy(self.recipe);bad[key]=value
            with self.subTest(key=key):
                with self.assertRaises(ValueError):_recipe(bad)
        bad=copy.deepcopy(self.recipe);bad['imageSha256Priority'][1]=bad['imageSha256Priority'][0]
        with self.assertRaises(ValueError):_recipe(bad)
        bad=copy.deepcopy(self.recipe);bad['mainTrailingCrop'].pop(next(iter(bad['mainTrailingCrop'])))
        with self.assertRaises(ValueError):_recipe(bad)
    def test_prior_original_plane_and_geometry_mutations_reject(self):
        pixels=bytearray(self.pixels);pixels[140]^=1
        with self.assertRaises(ValueError):self.run_repack(pixels=pixels)
        rows=self.rows[:];rows[0]=rows[0].replace(',93,126,',',94,126,')
        self.assertNotEqual(rows,self.rows)
        with self.assertRaises(ValueError):self.run_repack(rows=rows)
        with self.assertRaises(ValueError):self.run_repack(rows=self.rows[:-1])
        prior=copy.deepcopy(self.prior);prior['mainPlacements'][0]['atlas'][0]+=1
        with self.assertRaises(ValueError):self.run_repack(prior=prior)
    def test_crop_pixels_recipe_and_output_digest_mutations_reject(self):
        bad=copy.deepcopy(self.recipe);first=next(iter(bad['mainTrailingCrop']));bad['mainTrailingCrop'][first]['height']-=1
        with self.assertRaises(ValueError):self.run_repack(recipe=bad)
        bad=copy.deepcopy(self.recipe);bad['mainTrailingCrop'][first]['rgbaSha256']='0'*64
        with self.assertRaises(ValueError):self.run_repack(recipe=bad)
        bad=copy.deepcopy(self.recipe);bad['atlasSha256']='0'*64
        with self.assertRaises(ValueError):self.run_repack(recipe=bad)
        with self.assertRaises(ValueError):self.run_repack(raster=lambda p,w,h:bytes(w*204*4))
        with self.assertRaises(ValueError):self.run_repack(raster=lambda p,w,h:b'wrong')
if __name__=='__main__':unittest.main()
