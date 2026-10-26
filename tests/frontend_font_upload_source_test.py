# SPDX-License-Identifier: MIT
"""Extract only the exact maintained aux-sync function for isolated GL tests."""
from pathlib import Path
import argparse
import unittest
ROOT=Path(__file__).resolve().parents[1]
def sync_source():
    text=(ROOT/'src/RendererHdAtlas.inc').read_text()
    start=text.index('static void nfs_hd_sync_uniform(void){')
    brace=text.index('{',start);depth=0
    for i in range(brace,len(text)):
        if text[i]=='{':depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0:return text[start:i+1]+'\n'
    raise ValueError('Aux sync function incomplete')
class SourceContract(unittest.TestCase):
    def test_runtime_failure_and_exact_rebind_consumer(self):
        text=(ROOT/'src/FrontendFontRuntime.inc').read_text()
        self.assertIn('nfs_hd_bound_aux_texture=0;glActiveTexture(GL_TEXTURE0)',text)
        self.assertLess(text.index('if(glGetError()!=GL_NO_ERROR)'),text.index('glGenTextures'))
        self.assertNotIn('while(glGetError',text)
        exact=sync_source();self.assertIn('nfs_hd_bound_aux_texture!=auxiliary',exact)
        self.assertIn('glBindTexture(GL_TEXTURE_2D,auxiliary)',exact)
        prepare=(ROOT/'scripts/prepare_renderer.py').read_text();self.assertIn("'FrontendFontCapacity.h'",prepare)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--prepare-sync-include');a,rest=parser.parse_known_args()
    if a.prepare_sync_include:
        p=Path(a.prepare_sync_include)
        text=sync_source()
        if not p.exists()or p.read_text()!=text:p.write_text(text)
    else:unittest.main(argv=[__file__]+rest)
