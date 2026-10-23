"""Validate caption reuse without changing atlas sampling or original geometry."""
import copy
import json
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from hud_asset_review import ROOT
from generate_frontend_pixels import text_identity
from frontend_right_labels import right_label_rows, extend_layout, base_layout

class RightLabels(unittest.TestCase):
    def setUp(self):
        self.catalog=json.loads((ROOT/'assets/hud/frontend/right-labels/catalog.json').read_text())
        self.placements=json.loads((ROOT/'assets/hud/frontend/generated.json').read_text())['setupPlacements']
    def rows(self,catalog=None,placements=None):
        return right_label_rows(self.catalog if catalog is None else catalog,self.placements if placements is None else placements,text_identity)
    def test_width_height_and_original_shadow(self):
        rows=self.rows();self.assertEqual(len(rows),42)
        for start,count,width,atlas_width in [(0,10,49,66),(20,11,50,68)]:
            values=[[float(v) for v in row[1:row.index(',UINT64')].split(',')] for row in rows[start:start+count*2]]
            main,shadow=values[:count],values[count:]
            self.assertEqual(sum(v[3] for v in main),width)
            for a,b in zip(main,shadow):
                self.assertEqual(b[1:3],[a[1]+1,a[2]+1]);self.assertEqual(a[9],atlas_width)
                self.assertEqual(a[4],37)
            scale=float(rows[start].rsplit(',',2)[1]);self.assertAlmostEqual(atlas_width*scale,width)
            self.assertAlmostEqual(float(rows[start].rsplit(',',1)[1][:-1])*51,37)
    def test_existing_layout_roundtrip(self):
        layout=(ROOT/'src/FrontendWordLayout.inc').read_text()
        self.assertEqual(extend_layout(base_layout(layout),self.rows()),layout)
    def test_unknown_duplicate_and_missing_words(self):
        for mutate in [lambda c:c['words'][0].update(text='other'),lambda c:c['words'].append(c['words'][0]),lambda c:c['words'].pop(),lambda c:c['words'].__setitem__(1,copy.deepcopy(c['words'][0]))]:
            c=copy.deepcopy(self.catalog);mutate(c)
            with self.assertRaises(ValueError):self.rows(c)
    def test_gapped_duplicate_nonfinite_and_incomplete_glyphs(self):
        for field,value in [('x',1),('width',0),('width',float('nan')),('width',float('inf')),('cell',0),('cell',True)]:
            c=copy.deepcopy(self.catalog);c['words'][0]['glyphs'][0][field]=value
            with self.assertRaises(ValueError):self.rows(c)
        c=copy.deepcopy(self.catalog);c['words'][0]['glyphs'].pop()
        with self.assertRaises(ValueError):self.rows(c)
    def test_missing_duplicate_and_invalid_packing(self):
        for kind in ['missing','duplicate','bounds','crop']:
            p=copy.deepcopy(self.placements);item=next(x for x in p if x['file']=='one-player.svg')
            if kind=='missing':p.remove(item)
            elif kind=='duplicate':p.append(copy.deepcopy(item))
            elif kind=='bounds':item['atlas'][0]=1024
            else:item['sourceCrop'][2]+=1
            with self.assertRaises(ValueError):self.rows(placements=p)
    def test_extension_rejects_unexpected_boundaries(self):
        for layout in ['', 'uint64_t word;\n};', 'uint64_t word;float outputScaleX,outputScaleY;\n};\n']:
            with self.assertRaises(ValueError):extend_layout(layout,self.rows())
        with self.assertRaises(ValueError):base_layout('not an extended table')

    def test_layout_only_rejects_stale_inputs_and_preserves_pixels(self):
        import tempfile
        from unittest.mock import patch
        from generate_frontend_pixels import generate_frontend_layout
        metadata=json.loads((ROOT/'assets/hud/frontend/generated.json').read_text())
        relatives=set(metadata['sourceHashes']) | {'src/'+name for name in metadata['headerHashes']}
        relatives.update('assets/hud/frontend/'+name for name in ['generated.json','catalog.json',
            'location/catalog.json','setup/catalog.json','player/catalog.json','opponents/catalog.json',
            'opponents/packing.json','options/catalog.json','options/packing.json','right-labels/catalog.json',
            'main-word-art.json','player-word-art-0.json','player-word-art-1.json'])
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            for relative in relatives:
                target=root/relative;target.parent.mkdir(parents=True,exist_ok=True)
                target.write_bytes((ROOT/relative).read_bytes())
            pins={relative:(root/relative).stat().st_mtime_ns for relative in relatives}
            with patch('generate_frontend_pixels.ROOT',root):
                generate_frontend_layout(check=True);generate_frontend_layout();generate_frontend_layout()
                self.assertEqual(pins,{relative:(root/relative).stat().st_mtime_ns for relative in relatives})
                for relative in [next(iter(metadata['sourceHashes'])),
                                 'assets/hud/frontend/opponents/packing.json',
                                 'assets/hud/frontend/options/catalog.json','assets/hud/frontend/options/packing.json',
                                 'assets/hud/frontend/main-word-art.json',
                                 'assets/hud/frontend/player-word-art-0.json','assets/hud/frontend/player-word-art-1.json',
                                 'src/FrontendFontPixelsPart0.inc']:
                    target=root/relative;old=target.read_bytes();target.write_bytes(old+b' ')
                    with self.assertRaises(ValueError):generate_frontend_layout()
                    target.write_bytes(old)
                target=root/'assets/hud/frontend/generated.json'
                changed=copy.deepcopy(metadata)
                next(p for p in changed['setupPlacements'] if p['file']=='one-player.svg')['atlas'][0]+=4
                target.write_text(json.dumps(changed))
                with self.assertRaises(ValueError):generate_frontend_layout()

if __name__=='__main__':unittest.main()
