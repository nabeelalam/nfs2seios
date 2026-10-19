"""Bounded, digest-pinned remaster sources; never accepts arbitrary image files."""
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]
FONT_FILES={
    'assets/hud/fonts/BebasNeue-Regular.ttf':'08e4623805102d819f58601e46e345648846075e363b2ceb23313c2d1c83ec73',
    'assets/hud/fonts/Michroma-Regular.ttf':'b62301163788bc5b7f8fcac0b74b184e34e1827e577b499ecb724da065098f87',
    'assets/hud/fonts/BebasNeue-OFL.txt':'72082f6cb4d04be2ecf7cc7d9e1e7d73787f0af8a5a278a47cade70c16b78341',
    'assets/hud/fonts/Michroma-OFL.txt':'33c39ce59a45bfb0dae3280e98bdb46be59d6d68efc62d4dc5fae90a26d2f393',
}

def reviewed_font_file(name,data):
    return name in FONT_FILES and hashlib.sha256(data).hexdigest()==FONT_FILES[name]

def reviewed_remaster_svg(name,data):
    if not re.fullmatch(r'assets/hud/(?:(?:timing|speed)-[0-9]|omega|pause/(?:continue|restart|racing-music|sound-effects|quit|quit-to-system))\.svg',name):return False
    if len(data)>65536 or any(x in data.lower() for x in (b'<!',b'<?',b'href',b'data:')):return False
    try:
        approved=json.loads((ROOT/'assets/hud/remaster-sources.json').read_text())['reviewedSvgSha256']
        if hashlib.sha256(data).hexdigest()!=approved.get(name):return False
        root=ET.fromstring(data);ns='{http://www.w3.org/2000/svg}'
        if root.tag!=ns+'svg' or set(root.attrib)!={'width','height','viewBox'}:return False
        if name.endswith('omega.svg'):size=(64,64)
        elif '/pause/' in name:size=(255,28) if name.endswith('quit-to-system.svg') else ((255,24) if name.endswith('sound-effects.svg') else (155,24))
        else:size=(12,26) if '/timing-' in name else (29,24)
        if root.attrib!={'width':str(size[0]),'height':str(size[1]),'viewBox':'0 0 %d %d'%size}:return False
        attrs={
            'defs':set(),'g':{'transform'},'linearGradient':{'id','x1','y1','x2','y2'},
            'stop':{'offset','stop-color'},'filter':{'id','x','y','width','height'},
            'feGaussianBlur':{'in','stdDeviation','result'},'feOffset':{'in','dy','result'},
            'feFlood':{'flood-color','flood-opacity'},'feComposite':{'in2','operator','result'},
            'feMerge':set(),'feMergeNode':{'in'},
            'text':{'x','y','font-family','font-size','text-anchor','transform','fill','stroke','stroke-width','paint-order','filter'},
            'path':{'d','fill'},'polygon':{'points','fill'},
        }
        for node in root.iter():
            if node is root:continue
            if not node.tag.startswith(ns):return False
            tag=node.tag[len(ns):]
            if tag not in attrs or set(node.attrib)-attrs[tag]:return False
            if (node.tail or '').strip():return False
            if (node.text or '').strip() and (tag!='text' or not re.fullmatch('[A-Z0-9 ]+',node.text)):return False
            for key,value in node.attrib.items():
                if key in ('fill','stroke','stop-color','flood-color'):
                    if not re.fullmatch(r'#[0-9a-fA-F]{3}(?:[0-9a-fA-F]{3})?|url\(#[a-zA-Z][a-zA-Z0-9_-]*\)',value):return False
                elif key=='filter':
                    if not re.fullmatch(r'url\(#[a-zA-Z][a-zA-Z0-9_-]*\)',value):return False
                elif key=='font-family':
                    if value not in ('Bebas Neue','Michroma'):return False
                elif key=='d':
                    if not re.fullmatch(r'[MmZzLlHhVvCcSsQqTtAaEe0-9+.,\s-]+',value):return False
                elif key=='points':
                    if not re.fullmatch(r'[-0-9eE+.,\s]+',value):return False
                elif key=='transform':
                    if not re.fullmatch(r'(?:(?:translate|scale|skewX)\([-0-9eE+.,\s]+\)\s*)+',value):return False
                elif key in ('id','in','in2','result'):
                    if not re.fullmatch('[A-Za-z][A-Za-z0-9_-]*',value):return False
                elif key in ('operator','paint-order','text-anchor'):
                    if value not in ('in','stroke fill','middle'):return False
                elif not re.fullmatch(r'-?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)%?',value):return False
        return True
    except (OSError,KeyError,ValueError,ET.ParseError):return False
