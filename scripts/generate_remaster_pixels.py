"""Generate the reviewed font/logo artwork without reading original game data."""
import argparse
import hashlib
import json
from pathlib import Path
from hud_asset_review import ROOT,reviewed_remaster_svg
from hud_font_environment import project_fonts
from rasterize_hud_svg import rasterize

ASSETS=ROOT/'assets/hud'

def array(name,data,decimal=False):
    render=str if decimal else lambda x:'0x%02x'%x
    return 'static const unsigned char '+name+'[]={\n'+',\n'.join(','.join(render(v) for v in data[i:i+24]) for i in range(0,len(data),24))+'\n};\n'

def rle(data):
    encoded=bytearray();i=0
    while i<len(data):
        color=data[i:i+4];count=1
        while count<65535 and i+(count+1)*4<=len(data) and data[i+count*4:i+(count+1)*4]==color:count+=1
        encoded+=bytes([count&255,count>>8])+color;i+=count*4
    return encoded

def source(path,w,h,sources):
    data=path.read_bytes();name=path.relative_to(ROOT).as_posix()
    if not reviewed_remaster_svg(name,data):raise ValueError('Unreviewed HUD vector')
    sources[name]=hashlib.sha256(data).hexdigest()
    return rasterize(data,w,h)

def generate(kind):
    hashes={}
    with project_fonts():
        if kind in ('timing','speed'):
            catalog=json.loads((ASSETS/(kind+'-font.json')).read_text())
            size=(48,104) if kind=='timing' else (116,96)
            rgba=b''.join(source(ASSETS/(kind+'-'+str(i)+'.svg'),*size,hashes) for i in range(10))
            expected=10*size[0]*size[1]*4
            if len(rgba)!=expected:raise ValueError('Invalid numeral raster size')
            title='BebasNeue' if kind=='timing' else 'Michroma'
            text='// Renderer code: MIT. Font artwork: see assets/hud/fonts/'+title+'-OFL.txt.\n'
            if kind=='timing':
                fingerprint=catalog['originalGpuFingerprint']
                text+=array('nfs_timing_original_digest',bytes.fromhex(fingerprint['sha256']))
            text+=array('nfs_'+kind+'_raster_digest',hashlib.sha256(rgba).digest())
            if kind=='timing':text+='static const uint64_t nfs_timing_sample_expected=UINT64_C(0x'+fingerprint['sampleFnv64']+');\n'
            encoded=rle(rgba);text+=array('nfs_'+kind+'_pixels_rle',encoded)
        elif kind=='logo':
            rgba=bytearray(source(ASSETS/'omega.svg',256,256,hashes))
            for i in range(0,len(rgba),4):rgba[i:i+3]=bytes([249,247,240])
            encoded=rle(rgba);template=json.loads((ROOT/'scripts/hud_templates/LogoPixels.json').read_text())['template']
            text=template.replace('@PIXEL_ARRAY@',array('nfs_logo_pixels_rle',encoded).rstrip())
        elif kind=='pause':
            catalog=json.loads((ASSETS/'pause/catalog.json').read_text());rgba=bytearray(704*384*4)
            for entry in catalog['entries']:
                w,h=entry['raster'];pixels=source(ASSETS/'pause'/entry['file'],w,h,hashes)
                placement=catalog['placements'][entry['name']]
                ax,ay,cw,ch=placement['atlasInkBounds'];left,top,right,bottom=placement['originalRasterInkBounds']
                if not(320<=ax and 640<=ay and ax+cw<=1024 and ay+ch<=1024 and right-left==cw and bottom-top==ch):raise ValueError('Pause atlas bounds invalid')
                for y in range(h):
                    for x in range(w):
                        if pixels[(y*w+x)*4+3] and not(left<=x<right and top<=y<bottom):raise ValueError('Pause ink no longer fits reviewed crop')
                for y in range(ch):
                    at=((ay-640+y)*704+ax-320)*4
                    rgba[at:at+cw*4]=pixels[((top+y)*w+left)*4:((top+y)*w+right)*4]
            encoded=rle(rgba);template=json.loads((ROOT/'scripts/hud_templates/PauseFontPixels.json').read_text())['template']
            text=template.replace('@PIXEL_ARRAY@',array('nfs_pause_pixels_rle',encoded,decimal=True).rstrip())
        else:raise ValueError('Unknown remaster family')
    manifest=dict(generatedFrom='Reviewed existing-font/brand SVG artwork',originalGamePixelsIncluded=False,
        sourceHashes=hashes,rasterSha256=hashlib.sha256(rgba).hexdigest(),rasterBytes=len(rgba),rleBytes=len(encoded),
        headerSha256=hashlib.sha256(text.encode()).hexdigest())
    return text,manifest

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    for kind,name in [('timing','TimingFontPixels.inc'),('speed','SpeedFontPixels.inc'),('pause','PauseFontPixels.inc'),('logo','LogoPixels.inc')]:
        text,meta=generate(kind);header=ROOT/'src'/name;receipt=ASSETS/(kind+'-generated.json')
        if args.check:
            if header.read_text()!=text or json.loads(receipt.read_text())!=meta:raise ValueError('Generated remaster artwork differs: '+kind)
        else:header.write_text(text);receipt.write_text(json.dumps(meta,indent=2)+'\n')
    from generate_surface_pixels import generate_surface_pixels
    generate_surface_pixels(args.check)
    from generate_label_pixels import generate_labels
    generate_labels(args.check)
    from generate_punctuation_pixels import generate_punctuation
    generate_punctuation(args.check)
    print('Verified remaster pixels from pinned project fonts and reviewed vectors' if args.check else 'Generated reviewed remaster pixels; no original assets read')
if __name__=='__main__':main()
