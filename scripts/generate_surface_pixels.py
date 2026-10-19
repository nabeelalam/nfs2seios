"""Generate the reviewed panel-compatible gauge and gear artwork from vectors.

No game files or captured texture pages are read. The templates contain bounded
decoder/sampler code; only newly drawn artwork supplies their pixel arrays.
"""
import argparse
import hashlib
import json
from hud_asset_review import ROOT
from hud_font_environment import project_fonts
from generate_remaster_pixels import array, rle, source

ASSETS=ROOT/'assets/hud'

def generate_surface(kind):
    hashes={};encoded_bytes=0
    template_name={'gauge':'GaugeMaskPixels','gear':'GearPixels'}[kind]
    text=json.loads((ROOT/'scripts/hud_templates'/(template_name+'.json')).read_text())['template']
    with project_fonts():
        if kind=='gauge':
            rgba=bytearray(320*248*4)
            for name,w,h,ay in [('lower',172,104,0),('middle',292,96,104),('right',284,48,200)]:
                pixels=source(ASSETS/'gauge'/(name+'.svg'),w,h,hashes)
                for y in range(h):rgba[((ay+y)*320)*4:((ay+y)*320+w)*4]=pixels[y*w*4:(y+1)*w*4]
            encoded=rle(rgba);encoded_bytes=len(encoded)
            text=text.replace('@GAUGE_MASK@',array('nfs_gauge_mask_rle',encoded,decimal=True).rstrip())
        elif kind=='gear':
            rasters=[]
            for name in list('NR123456')+['face']:
                if name=='face':pixels=source(ASSETS/'gear/face.svg',128,128,hashes)
                else:
                    full=source(ASSETS/'gear'/(name+'.svg'),56,96,hashes)
                    # Store the common transparent-margin crop. Runtime code
                    # reconstructs the full cell and its optical offset.
                    for y in range(96):
                        for x in range(56):
                            if full[(y*56+x)*4+3] and not(4<=x<52 and 8<=y<88):raise ValueError('Gear ink exceeds reviewed crop')
                    pixels=b''.join(full[((y+8)*56+4)*4:((y+8)*56+52)*4] for y in range(80))
                rasters.append(pixels);encoded=rle(pixels);encoded_bytes+=len(encoded)
                text=text.replace('@GEAR_'+name+'@',array('nfs_gear_'+name+'_rle',encoded,decimal=True).rstrip())
            rgba=b''.join(rasters)
        else:raise ValueError('Unknown surface family')
    if '@' in text:raise ValueError('Unexpanded surface template')
    manifest=dict(generatedFrom='Reviewed newly drawn classic HUD vectors and pinned font',originalGamePixelsIncluded=False,
        sourceHashes=hashes,rasterSha256=hashlib.sha256(rgba).hexdigest(),rasterBytes=len(rgba),rleBytes=encoded_bytes,headerSha256=hashlib.sha256(text.encode()).hexdigest())
    return text,manifest

def generate_surface_pixels(check=False):
    for kind,name in [('gauge','GaugeMaskPixels.inc'),('gear','GearPixels.inc')]:
        text,manifest=generate_surface(kind);header=ROOT/'src'/name;receipt=ASSETS/(kind+'-generated.json')
        if check:
            if header.read_text()!=text or json.loads(receipt.read_text())!=manifest:raise ValueError('Generated surface differs: '+kind)
        else:header.write_bytes(text.encode());receipt.write_bytes((json.dumps(manifest,indent=2)+'\n').encode())
    print('Verified reproducible gauge/gear vectors; no original game pixels read' if check else 'Generated reviewed gauge/gear artwork')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--check',action='store_true');generate_surface_pixels(p.parse_args().check)
