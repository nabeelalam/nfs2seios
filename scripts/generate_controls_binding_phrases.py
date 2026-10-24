"""Author complete Controls binding phrases privately; never export font cells.

Reads only external FFN layout metrics and a locally installed display font.
Original bitmap pixels and proprietary font files are never copied to output.
"""
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,hashlib,json
from generate_controls_setup_words import find_font,alpha_runs
from inspect_frontend_font import inspect

KEY_NAMES=tuple(sorted(set(chr(i).upper()for i in range(33,127))|set(('space','enter','insert','delete','home','end','page up','page down','up','down','left','right','backspace','tab','center'))))
assert len(KEY_NAMES)==83

def specifications(size,advances):
    gap,third=(6,66)if size==16 else(10,80)
    def width(text):
        values=[advances.get(ord(c),0)for c in text]
        if any(x<=0 for x in values):raise ValueError('Unsupported source advance')
        return sum(values)
    def row(kind,name,device,number,tokens,positions):
        widths=[width(t)for t in tokens]
        return dict(kind=kind,name=name,device=device,number=number,sourceSize=size,tokens=tokens,positions=positions,advances=widths,logicalWidth=positions[-1]+widths[-1])
    for name in KEY_NAMES:yield row(0,name,0,0,['keyboard',name],[0,third])
    for device in (1,2):
        for axis in range(16):yield row(1,str(axis),device,axis,['joystick',str(device),'axis',str(axis)],[0,width('joystick')+gap,third,third+width('axis')+gap])
    for device in (1,2):
        for button in range(32):yield row(2,chr(65+button),device,button,['joystick',str(device),'button',chr(65+button)],[0,width('joystick')+gap,third,third+width('button')+gap])
    yield row(3,'________',0,0,['________'],[0])

def generate(output,font_path,metrics_directory):
    from PIL import Image,ImageDraw,ImageFilter,ImageFont
    font=ImageFont.truetype(str(font_path),192)
    if font.getname()[0].lower().replace(' ','')!='haettenschweiler':raise ValueError('Expected Haettenschweiler')
    cache={};rows=[];metrics={};output.mkdir(parents=True,exist_ok=False)
    def shape(text,size,advance):
        key=(text,size,advance)
        if key in cache:return cache[key]
        canvas=Image.new('L',(2400,512));ImageDraw.Draw(canvas).text((16,200),text,font=font,fill=255,anchor='ls');box=canvas.getbbox()
        if not box:raise ValueError('Empty complete-phrase component')
        ink=canvas.crop(box);pad=Image.new('L',(ink.width+32,ink.height+32));pad.paste(ink,(16,16));ink=pad.resize((round(pad.width*.88),pad.height),Image.Resampling.LANCZOS).filter(ImageFilter.MinFilter(9));ink=ink.crop(ink.getbbox());scale=size/18/2
        ink=ink.resize((max(1,round(ink.width*scale)),max(1,round(ink.height*scale))),Image.Resampling.LANCZOS)
        if ink.width>advance*4-4:ink=ink.resize((advance*4-4,ink.height),Image.Resampling.LANCZOS)
        cache[key]=ink;return ink
    for size in (16,18):
        info=inspect((metrics_directory/('swiss%d.ffn'%size)).read_bytes());metrics[str(size)]=info['metricsSha256'];advances={g['code']:g['advance']for g in info['glyphs']}
        for row in specifications(size,advances):
            image=len(rows)+1;width=row['logicalWidth']*4;alpha=Image.new('L',(width,128))
            for text,x,advance in zip(row['tokens'],row['positions'],row['advances']):
                ink=shape(text,size,advance);alpha.paste(ink,(x*4+2,96-ink.height))
            if not alpha.getbbox()or alpha.getbbox()[0]<=0 or alpha.getbbox()[1]<=0 or alpha.getbbox()[2]>=width or alpha.getbbox()[3]>=128:raise ValueError('Missing phrase padding')
            rgba=Image.new('RGBA',alpha.size,(255,255,255,255));rgba.putalpha(alpha);digest=hashlib.sha256(rgba.tobytes()).hexdigest();row.update(image=image,width=width,height=128,rgbaSha256=digest,alphaRuns=alpha_runs(alpha));rows.append(row)
            if row['name']in('A','page up')or row['kind']==1 and row['device']==1 and row['number']==0 or row['kind']==2 and row['device']==2 and row['number']==2:rgba.save(output/('phrase-%03d.png'%image))
    assert len(rows)==360
    manifest=dict(schema=1,scope='Finite complete Controls binding phrase artwork',typeface='Haettenschweiler',wholePhraseImagesOnly=True,reusableFontAlphabet=False,originalBitmapPixelsExported=False,sourceMetricsSha256=metrics,rasterScale=4,baseline=24,rows=rows)
    path=output/'binding-phrases.json';path.write_text(json.dumps(manifest,separators=(',',':'))+'\n');return dict(images=len(rows),manifestBytes=path.stat().st_size,manifestSha256=hashlib.sha256(path.read_bytes()).hexdigest(),individualGlyphImagesExported=False)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True,type=Path);p.add_argument('--font',type=Path);p.add_argument('--original-font-directory',required=True,type=Path);a=p.parse_args();print(json.dumps(generate(a.output,find_font(a.font),a.original_font_directory)))
