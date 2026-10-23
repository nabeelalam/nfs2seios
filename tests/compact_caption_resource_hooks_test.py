"""Fixed Options/Setup resource observations preserve every original call byte."""
# SPDX-License-Identifier: MIT
import re
import sys
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
sys.path.insert(0,str(ROOT/'tests'))
from options_scene_hooks import add_options_scene_hooks
from setup_scene_hooks import add_setup_scene_hooks
from options_scene_hooks_test import original as before_options
from setup_scene_hooks_test import original as before_setup

CALL="\tesp -= 4; _sub_41B710(); esp += 4; //call"

class CompactCaptionResourceHooks(unittest.TestCase):
    def test_each_fixed_scope_covers_the_actual_resource_return_before_measurement(self):
        for family,transform,before in (("options",add_options_scene_hooks,before_options),
                                        ("setup",add_setup_scene_hooks,before_setup)):
            original=before('Methods_01.cpp');result=transform(original,'Methods_01.cpp')
            section=result[result.index('Fn(void) Game::_sub_405580()'):]
            section=section.split('\nFn(void) Game::',1)[0]
            tokens=['nfs_'+family+'_caption::ResourceScope',CALL,
                    'nfs_'+family+'_caption::resolvedResource(*this,uint32_t(eax));',
                    'nfs_'+family+'_caption::measured']
            offsets=[section.index(token) for token in tokens]
            with self.subTest(family=family):
                self.assertEqual(offsets,sorted(offsets))
                self.assertEqual(section.count(CALL),1)
                self.assertEqual(result.count('nfs_'+family+'_caption::ResourceScope'),1)
                marker='NFS_'+family.upper()+'_SCENE'
                stripped=re.sub(r'\n// '+marker+r'_BEGIN:([^\n]+)\n.*?// '+marker+r'_END:\1\n','',result,flags=re.S)
                self.assertEqual(stripped.encode(),original.encode())

    def test_missing_duplicate_and_changed_resolution_decline(self):
        for family,transform,before in (("options",add_options_scene_hooks,before_options),
                                        ("setup",add_setup_scene_hooks,before_setup)):
            original=before('Methods_01.cpp')
            self.assertEqual(original.count(CALL),1)
            for replacement in ('',CALL+CALL,CALL.replace('41B710','41B711')):
                with self.subTest(family=family,replacement=replacement),self.assertRaises(ValueError):
                    transform(original.replace(CALL,replacement,1),'Methods_01.cpp')

    def test_main_origin_remains_player_only_and_enum_policies_are_fixed(self):
        core=(ROOT/'src/hd/CachedMenuCaptionGameHooks.inc').read_text()
        proof=(ROOT/'src/hd/CompactCaptionProof.h').read_text()
        initial=core[core.index(' InitialCallerScope(Game&game'):core.index('struct TailScope')]
        self.assertIn('!playerProofPolicy()',initial)
        self.assertNotIn('ordinaryProofPolicy()',initial)
        resource=core[core.index('struct ResourceScope'):core.index('// Inserted only immediately after actual41B710')]
        self.assertIn('!ordinaryProofPolicy()',resource)
        self.assertIn('e.policy=proofPolicy()',resource)
        self.assertIn('Setup=0x4d2790',proof)
        self.assertIn('Options=0x4d3a88',proof)

if __name__=='__main__':unittest.main()
