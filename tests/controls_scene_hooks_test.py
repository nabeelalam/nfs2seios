"""Public synthetic source proof for Controls original native426950 hooks."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from controls_scene_hooks import MARKER,NAMES,add_controls_scene_hooks,remove_controls_scene_hooks
from graphics_scene_hooks import add_graphics_scene_hooks
from audio_scene_hooks import add_audio_scene_hooks
HEADER='#include "FunctionIds.h"\n'
LEASE='\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction tx;\n\t} // checked copy transaction end\n'
def call(name):return '\tesp -= 4; '+name+'(); esp += 4; //call\n'
def source():
    # Original operation spellings/order/loop edges are independently transcribed
    # from the measured maintained Methods03 source; no game data/private paths.
    body='\tpush32(ebx);\n'+LEASE+'loc_42697D:\n'+LEASE+'loc_4269A7:\n'+LEASE+call('___CHP')+LEASE+call('_sub_4850B0')+LEASE+call('_sub_49755C')+call('_sub_497738')+'\tadd(esi, (int32_t)0x20);\n'+call('_sub_497D6C')+'\tcmp(esi, (int32_t)0x1E0);\n\tif (jnz())\n\t\tgoto loc_4269A7;\n\tadd(edi, (int32_t)0x20);\n\tcmp(edi, (int32_t)0x280);\n\tif (jnz())\n\t\tgoto loc_42697D;\n\tpop32(ebx);\n'
    return HEADER+'Fn(void) Game::_sub_426950()\n{\n'+body+'}\nFn(void) Game::_sub_426A60()\n{\n return;\n}\n'
def composed():return add_graphics_scene_hooks(add_audio_scene_hooks(source(),'Methods_03.cpp'),'Methods_03.cpp')
class ControlsSceneHooks(unittest.TestCase):
    def test_exact_reversal_lf_crlf_and_original_operations(self):
        for base in (source(),composed()):
            for text in (base,base.replace('\n','\r\n')):
                out=add_controls_scene_hooks(text,'Methods_03.cpp')
                self.assertEqual(remove_controls_scene_hooks(out).encode(),text.encode())
                self.assertEqual(re.findall(r'\besp -= 4; (?:_sub_|___)[A-Za-z0-9]+\(\); esp \+= 4; //call',out),re.findall(r'\besp -= 4; (?:_sub_|___)[A-Za-z0-9]+\(\); esp \+= 4; //call',text))
                self.assertEqual(out.count('GuestCopyTransaction'),text.count('GuestCopyTransaction'))
                for namespace in ('nfs_audio_scene_live','nfs_graphics_scene_live'):
                    self.assertEqual(out.count(namespace),text.count(namespace))
                self.assertEqual(out[out.index('Fn(void) Game::_sub_426A60'):],text[text.index('Fn(void) Game::_sub_426A60'):])
    def test_outer_scope_order_and_unwind_cancellation(self):
        out=add_controls_scene_hooks(composed(),'Methods_03.cpp')
        self.assertLess(out.index('nfs_controls_scene_live::begin'),out.index('nfs_graphics_scene_live::begin'))
        self.assertLess(out.index('nfs_graphics_scene_live::begin'),out.index('_sub_49755C();'))
        self.assertGreater(out.index('nfs_controls_scene_live::finish'),out.index('nfs_graphics_scene_live::finish'))
        self.assertGreater(out.index('nfs_controls_scene_live::finish'),out.index('pop32(ebx);'))
        self.assertIn('nfs_controls_scene_live::finish(nfs_controls_scene_intent);\n } catch(...) {\n nfs_controls_scene_live::cancel(nfs_controls_scene_intent);\n throw;',out)
        for forbidden in ('activeRequest()=','nextRequest()','enqueue_renderer','guest_store','privateObserve'):
            self.assertNotIn(forbidden,out)
    def test_missing_duplicate_and_reordered_anchors_decline(self):
        for token in ('#include "FunctionIds.h"','Game::_sub_426950','loc_42697D:','loc_4269A7:','___CHP();','_sub_4850B0();','_sub_49755C();','_sub_497738();','_sub_497D6C();','add(esi, (int32_t)0x20);','cmp(esi, (int32_t)0x1E0);','goto loc_4269A7;','add(edi, (int32_t)0x20);','cmp(edi, (int32_t)0x280);','goto loc_42697D;','pop32(ebx);'):
            with self.subTest(token=token),self.assertRaises(ValueError):add_controls_scene_hooks(source().replace(token,'changed-original',1),'Methods_03.cpp')
        with self.assertRaises(ValueError):add_controls_scene_hooks(source()+source(),'Methods_03.cpp')
        swapped=source().replace('_sub_49755C','temporary').replace('_sub_497738','_sub_49755C').replace('temporary','_sub_497738')
        with self.assertRaises(ValueError):add_controls_scene_hooks(swapped,'Methods_03.cpp')
    def test_exit_lease_mixed_newline_and_markers_decline(self):
        base=source()
        for changed in (base.replace('\tpop32(ebx);',' return;\n\tpop32(ebx);'), '// checked copy transaction begin\n'+base,base.replace('\tpop32(ebx);','// checked copy transaction begin\n\tpop32(ebx);'),base.replace('\tpush32(ebx);','// checked copy transaction end\n\tpush32(ebx);'),base.replace('\n','\r\n',1)):
            with self.assertRaises(ValueError):add_controls_scene_hooks(changed,'Methods_03.cpp')
        out=add_controls_scene_hooks(base,'Methods_03.cpp')
        with self.assertRaises(ValueError):add_controls_scene_hooks(out,'Methods_03.cpp')
        with self.assertRaises(ValueError):remove_controls_scene_hooks(out.replace(MARKER+'_END:finish',MARKER+'_END:broken'))
    def test_private_and_foreign_wrappers_preserved(self):
        base=composed().replace('Fn(void) Game::_sub_426950()\n{','Fn(void) Game::_sub_426950()\n{\n existing_private_observer_scope();')
        out=add_controls_scene_hooks(base,'Methods_03.cpp')
        self.assertEqual(remove_controls_scene_hooks(out),base)
        self.assertEqual(out.count('existing_private_observer_scope();'),1)
    def test_unrelated_unchanged(self):
        self.assertEqual(NAMES,('Methods_03.cpp',))
        self.assertEqual(add_controls_scene_hooks('unchanged','Methods_01.cpp'),'unchanged')
        self.assertEqual(remove_controls_scene_hooks('unchanged'),'unchanged')
if __name__=='__main__':unittest.main()
