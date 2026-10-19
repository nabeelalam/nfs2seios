"""Use only the two pinned project fonts, without changing global font settings."""
from contextlib import contextmanager
import os
from pathlib import Path
import subprocess
import tempfile
from xml.sax.saxutils import escape
from hud_asset_review import ROOT, FONT_FILES, reviewed_font_file

@contextmanager
def project_fonts():
    for name in FONT_FILES:
        if not reviewed_font_file(name,(ROOT/name).read_bytes()):raise ValueError('Pinned HUD font or license differs')
    old=os.environ.get('FONTCONFIG_FILE')
    with tempfile.TemporaryDirectory(prefix='nfs-hud-fonts-') as folder:
        p=Path(folder);config=p/'fonts.conf'
        config.write_text('<fontconfig><dir>'+escape(str(ROOT/'assets/hud/fonts'))+'</dir><cachedir>'+escape(str(p/'cache'))+'</cachedir></fontconfig>')
        os.environ['FONTCONFIG_FILE']=str(config)
        try:
            for family,name in [('Bebas Neue','BebasNeue-Regular.ttf'),('Michroma','Michroma-Regular.ttf')]:
                actual=subprocess.check_output(['fc-match','-f','%{file}',family],text=True).strip()
                if Path(actual).resolve()!=(ROOT/'assets/hud/fonts'/name).resolve():raise ValueError('HUD font substitution rejected')
            yield
        finally:
            if old is None:os.environ.pop('FONTCONFIG_FILE',None)
            else:os.environ['FONTCONFIG_FILE']=old
