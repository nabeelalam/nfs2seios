"""Bounded, digest-pinned remaster sources; never accepts arbitrary image files."""
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]
FONT_FILES={
    'assets/hud/fonts/Oxanium-OFL.txt':'fe17c0f2581d71b4e1ea7e636e7f4877c29223e11bb1dd1a871e8c3f2a86336b',
    'assets/hud/fonts/Oxanium-Variable.ttf':'2ce01d946e1e1ffc8d7eecfffbda8623bedd63eaf811a20488c4b69af45babb0',
    'assets/hud/fonts/BebasNeue-Regular.ttf':'08e4623805102d819f58601e46e345648846075e363b2ceb23313c2d1c83ec73',
    'assets/hud/fonts/Michroma-Regular.ttf':'b62301163788bc5b7f8fcac0b74b184e34e1827e577b499ecb724da065098f87',
    'assets/hud/fonts/BebasNeue-OFL.txt':'72082f6cb4d04be2ecf7cc7d9e1e7d73787f0af8a5a278a47cade70c16b78341',
    'assets/hud/fonts/Michroma-OFL.txt':'33c39ce59a45bfb0dae3280e98bdb46be59d6d68efc62d4dc5fae90a26d2f393',
}
FRONTEND_WORDS = {
    'game-setup': ('game setup', 95), 'location': ('location', 66),
    'player-one': ('player one', 85), 'player-two': ('player two', 86),
    'opponents': ('opponents', 87), 'race': ('RACE', 75),
    'options': ('options', 62), 'exit': ('exit', 32),
}
LOCATION_WORDS = {
    'track': ('track', 33),
    'colon': (':', 6),
    'north-country': ('North Country', 88),
    'laps': ('laps', 27),
    'lap-two': ('2', 10),
    'track-info': ('track info...', 70),
    'track-records': ('track records...', 92),
    'personal-statistics': ('personal statistics...', 124),
    'done': ('DONE', 32),
    'cancel': ('CANCEL', 46),
    'pacific-spirit': ('Pacific Spirit', 78),
    'mediterraneo': ('Mediterraneo', 84),
    'mystic-peaks': ('Mystic Peaks', 82),
    'proving-grounds': ('Proving Grounds', 102),
    'outback': ('Outback', 53),
    'last-resort': ('Last Resort', 71),
    'lap-4': ('4', 10),
    'lap-8': ('8', 10),
}

SETUP_WORDS = {
    'game-type': ('game type', 66),
    'race-type': ('race type', 59),
    'style': ('style', 32),
    'catch-up': ('catch up', 54),
    'backwards': ('backwards', 67),
    'mirrored': ('mirrored', 55),
    'colon': (':', 6),
    'one-player': ('one player', 66),
    'split-screen': ('split screen', 72),
    'modem': ('modem', 48),
    'serial-link': ('serial link', 60),
    'network': ('network', 53),
    'single-race': ('single race', 68),
    'tournament': ('tournament', 74),
    'knockout': ('knockout', 59),
    'wild': ('wild', 27),
    'arcade': ('arcade', 43),
    'simulation': ('simulation', 66),
    'off': ('off', 20),
    'on': ('on', 18),
    'done': ('DONE', 32),
    'cancel': ('CANCEL', 46),
}
SETUP_SHARED = {'colon', 'done', 'cancel'}
PLAYER_WORDS = {'car': ('car', 21), 'transmission': ('transmission', 80), 'color': ('color', 33), 'graph': ('graph', 37), 'colon': (':', 6), 'manual': ('manual', 47), 'automatic': ('automatic', 64), 'off': ('off', 20), 'on': ('on', 18), 'done': ('DONE', 32), 'cancel': ('CANCEL', 46), 'player-one-heading': ('PLAYER ONE', 71), 'player-two-heading': ('PLAYER TWO', 73), 'ford-indigo': ('Ford Indigo', 70), 'mustang-mach-iii': ('Mustang Mach III', 105), 'jaguar-xj220': ('Jaguar XJ220', 86), 'lotus-gt1': ('Lotus GT1', 63), 'lotus-esprit-v8': ('Lotus Esprit V8', 93), 'nazca-c2': ('Nazca C2', 58), 'italdesign-cala': ('Italdesign Calà', 91), 'isdera-112i': ('Isdera 112i', 68), 'mclaren-f1': ('McLaren F1', 72), 'ferrari-f50': ('Ferrari F50', 68), 'ferrari-355-f1': ('Ferrari 355 F1', 86), 'ford-gt90': ('Ford GT90', 65), 'class-a': ('class A', 44), 'class-b': ('class B', 44), 'class-c': ('class C', 44), 'acceleration': ('acceleration', 77), 'top-speed': ('top speed', 63), 'braking': ('braking', 48), 'handling': ('handling', 54), 'settings': ('settings...', 62), 'showcase': ('showcase...', 72)}
PLAYER_SHARED = {'colon': 'location', 'done': 'location', 'cancel': 'location', 'off': 'setup', 'on': 'setup'}



OPPONENTS_WORDS = {'car': ('car', 21), 'number': ('number', 49), 'skill-level': ('skill level', 59), 'traffic': ('traffic', 40), 'colon': (':', 6), 'done': ('DONE', 32), 'cancel': ('CANCEL', 46), 'class-a': ('class A', 44), 'class-b': ('class B', 44), 'class-c': ('class C', 44), 'no-opponents': ('no opponents', 86), 'ford-indigo': ('Ford Indigo', 70), 'mustang-mach-iii': ('Mustang Mach III', 105), 'jaguar-xj220': ('Jaguar XJ220', 86), 'lotus-gt1': ('Lotus GT1', 63), 'lotus-esprit-v8': ('Lotus Esprit V8', 93), 'nazca-c2': ('Nazca C2', 58), 'italdesign-cala': ('Italdesign Cal\xe0', 91), 'isdera-112i': ('Isdera 112i', 68), 'mclaren-f1': ('McLaren F1', 72), 'ferrari-f50': ('Ferrari F50', 68), 'ferrari-355-f1': ('Ferrari 355 F1', 86), 'ford-gt90': ('Ford GT90', 65), 'full-grid': ('full grid', 49), 'none': ('none', 33), 'one': ('one', 25), 'beginner': ('beginner', 56), 'advanced': ('advanced', 60), 'off': ('off', 20), 'on': ('on', 18)}
OPPONENTS_SHARED = {'car': 'player', 'colon': 'location', 'done': 'location', 'cancel': 'location', 'class-a': 'player', 'class-b': 'player', 'class-c': 'player', 'ford-indigo': 'player', 'mustang-mach-iii': 'player', 'jaguar-xj220': 'player', 'lotus-gt1': 'player', 'lotus-esprit-v8': 'player', 'nazca-c2': 'player', 'italdesign-cala': 'player', 'isdera-112i': 'player', 'mclaren-f1': 'player', 'ferrari-f50': 'player', 'ferrari-355-f1': 'player', 'ford-gt90': 'player', 'off': 'setup', 'on': 'setup'}

OPTIONS_WORDS = {'speedometer': ('speedometer',83), 'mph': ('mph',29), 'kmh': ('kmh',29), 'personal-stats': ('personal stats',88), 'audio': ('audio...',46), 'controls': ('controls...',63), 'load-save': ('load/save...',72), 'graphics': ('graphics...',64), 'credits': ('credits...',54)}

def reviewed_font_file(name,data):
    return name in FONT_FILES and hashlib.sha256(data).hexdigest()==FONT_FILES[name]

def reviewed_remaster_svg(name,data):
    if name in ('assets/hud/warning/wrong-way.svg','assets/hud/warning/exclamation.svg'):
        from generate_warning_word_pixels import reviewed_warning_word_source
        return reviewed_warning_word_source(name,data)
    if name == 'assets/hud/pause/volume-bar.svg':
        from generate_volume_bar_pixels import reviewed_volume_bar_source
        return reviewed_volume_bar_source(name,data)
    if name in ('assets/hud/pause/header-title.svg','assets/hud/pause/header-caution.svg'):
        from generate_pause_header_pixels import reviewed_pause_header_source
        return reviewed_pause_header_source(name,data)
    options_match=re.fullmatch(r'assets/hud/frontend/options/([a-z0-9-]+)\.svg',name)
    options_art=options_match is not None and options_match[1] in OPTIONS_WORDS
    opponents_match=re.fullmatch(r'assets/hud/frontend/opponents/([a-z0-9-]+)\.svg',name)
    opponents_art=opponents_match is not None and opponents_match[1] in OPPONENTS_WORDS and opponents_match[1] not in OPPONENTS_SHARED
    player_match=re.fullmatch(r'assets/hud/frontend/player/([a-z0-9-]+)\.svg',name)
    player_art=player_match is not None and player_match[1] in PLAYER_WORDS and player_match[1] not in PLAYER_SHARED
    if not options_art and not opponents_art and not player_art and not re.fullmatch(r'assets/hud/(?:(?:timing|speed)-[0-9]|small-[A-Z]|record-(?:[1-9]|dot|colon)|punctuation/(?:clock|slash)|start/sphere|omega|pause/(?:continue|restart|racing-music|sound-effects|quit|quit-to-system)|gauge/(?:lower|middle|right)|gear/(?:N|R|[1-6]|face)|frontend/(?:game-setup|location|player-one|player-two|opponents|race|options|exit|location/(?:track|colon|north-country|laps|lap-two|track-info|track-records|personal-statistics|done|cancel|pacific-spirit|mediterraneo|mystic-peaks|proving-grounds|outback|last-resort|lap-4|lap-8)|setup/(?:game-type|race-type|style|catch-up|backwards|mirrored|one-player|split-screen|modem|serial-link|network|single-race|tournament|knockout|wild|arcade|simulation|off|on)))\.svg',name):return False
    if len(data)>65536 or any(x in data.lower() for x in (b'<!',b'<?',b'href',b'data:')):return False
    try:
        approved=json.loads((ROOT/'assets/hud/remaster-sources.json').read_text())['reviewedSvgSha256']
        if hashlib.sha256(data).hexdigest()!=approved.get(name):return False
        root=ET.fromstring(data);ns='{http://www.w3.org/2000/svg}'
        if root.tag!=ns+'svg' or set(root.attrib)!={'width','height','viewBox'}:return False
        origin=(0,0)
        if '/gauge/' in name:
            x,y,w,h={'lower':(10,32,43,26),'middle':(32,8,73,24),'right':(105,7,71,12)}[Path(name).stem]
            origin=(x,y);size=(w,h)
        elif '/frontend/options/' in name:size=(OPTIONS_WORDS[Path(name).stem][1],51)
        elif '/frontend/opponents/' in name:size=(OPPONENTS_WORDS[Path(name).stem][1],51)
        elif '/frontend/player/' in name:size=(PLAYER_WORDS[Path(name).stem][1],51)
        elif '/frontend/setup/' in name:size=(SETUP_WORDS[Path(name).stem][1],51)
        elif '/frontend/location/' in name:size=(LOCATION_WORDS[Path(name).stem][1],51)
        elif '/frontend/' in name:size=(FRONTEND_WORDS[Path(name).stem][1],51)
        elif '/start/' in name:size=(64,64)
        elif '/punctuation/' in name:size=(33,26) if name.endswith('/clock.svg') else (9,28)
        elif '/gear/' in name:size=(32,32) if name.endswith('/face.svg') else (14,24)
        elif '/small-' in name:size=(10 if name.endswith('small-W.svg') else 4 if name.endswith('small-I.svg') else 8,8)
        elif '/record-' in name:size=(6 if name.endswith('record-1.svg') else 4 if name.endswith(('record-dot.svg','record-colon.svg')) else 8,8)
        elif name.endswith('omega.svg'):size=(64,64)
        elif '/pause/' in name:size=(255,28) if name.endswith('quit-to-system.svg') else ((255,24) if name.endswith('sound-effects.svg') else (155,24))
        else:size=(12,26) if '/timing-' in name else (29,24)
        if root.attrib!={'width':str(size[0]),'height':str(size[1]),'viewBox':'%d %d %d %d'%(*origin,*size)}:return False
        attrs={
            'defs':set(),'g':{'transform'},'linearGradient':{'id','x1','y1','x2','y2','gradientUnits'},
            'radialGradient':{'id','cx','cy','r'},'ellipse':{'cx','cy','rx','ry','fill'},
            'stop':{'offset','stop-color','stop-opacity'},'filter':{'id','x','y','width','height'},
            'feGaussianBlur':{'in','stdDeviation','result'},'feOffset':{'in','dy','result'},
            'feFlood':{'flood-color','flood-opacity'},'feComposite':{'in2','operator','result'},
            'feMerge':set(),'feMergeNode':{'in'},
            'text':{'x','y','font-family','font-weight','font-size','text-anchor','transform','fill','stroke','stroke-width','paint-order','filter'},
            'path':{'d','fill','fill-rule','stroke','stroke-width','stroke-linejoin','stroke-linecap','stroke-opacity'},'polygon':{'points','fill'},
            'circle':{'cx','cy','r','fill','stroke','stroke-width','stroke-opacity'},
        }
        for node in root.iter():
            if node is root:continue
            if not node.tag.startswith(ns):return False
            tag=node.tag[len(ns):]
            if tag not in attrs or set(node.attrib)-attrs[tag]:return False
            if (node.tail or '').strip():return False
            if (node.text or '').strip():
                if tag!='text':return False
                if '/frontend/options/' in name:
                    if node.text!=OPTIONS_WORDS[Path(name).stem][0]:return False
                elif '/frontend/opponents/' in name:
                    if node.text!=OPPONENTS_WORDS[Path(name).stem][0]:return False
                elif '/frontend/player/' in name:
                    if node.text!=PLAYER_WORDS[Path(name).stem][0]:return False
                elif '/frontend/setup/' in name:
                    if node.text!=SETUP_WORDS[Path(name).stem][0]:return False
                elif '/frontend/location/' in name:
                    if node.text!=LOCATION_WORDS[Path(name).stem][0]:return False
                elif '/frontend/' in name:
                    if node.text!=FRONTEND_WORDS[Path(name).stem][0]:return False
                elif not re.fullmatch('[A-Z0-9 .:]+',node.text):return False
            for key,value in node.attrib.items():
                if key in ('fill','stroke','stop-color','flood-color'):
                    rgb=re.fullmatch(r'rgb\(([0-9]+(?:\.[0-9]+)?),([0-9]+(?:\.[0-9]+)?),([0-9]+(?:\.[0-9]+)?)\)',value)
                    if rgb:
                        if any(float(c)>255 for c in rgb.groups()):return False
                    elif value!='none' and not re.fullmatch(r'#[0-9a-fA-F]{3}(?:[0-9a-fA-F]{3})?|url\(#[a-zA-Z][a-zA-Z0-9_-]*\)',value):return False
                elif key=='stop-opacity':
                    if not re.fullmatch(r'(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)',value) or not 0<=float(value)<=1:return False
                elif key=='gradientUnits':
                    if value!='userSpaceOnUse':return False
                elif key=='fill-rule':
                    if value!='evenodd':return False
                elif key in ('stroke-linejoin','stroke-linecap'):
                    if value!='round':return False
                elif key=='filter':
                    if not re.fullmatch(r'url\(#[a-zA-Z][a-zA-Z0-9_-]*\)',value):return False
                elif key=='font-family':
                    if value not in ('Bebas Neue','Michroma','Oxanium'):return False
                elif key=='font-weight':
                    if value!='500':return False
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
