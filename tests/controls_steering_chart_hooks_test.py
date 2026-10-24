"""Synthetic exact-anchor chart hooks; no game/build/helper execution."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
sys.path.insert(0,str(Path(__file__).resolve().parent))
from controls_steering_text_hooks_test import fixtures as text_fixtures
from controls_steering_text_hooks import add_controls_steering_text_hooks
from controls_text_hooks_test import fn,section,LEASE
from controls_steering_chart_hooks import (MARKER,NAMES,STORE,CODES,
    add_controls_steering_chart_hooks,remove_controls_steering_chart_hooks)

ROW=('\tebp = eax; //mov\n\tesi = edx; //mov\n\tedi = ebx; //mov\n'
     'loc_483292:\n'+LEASE+
     '\tebx = guest_transaction.load<int32_t>(dword_5636EC); //mov\n'
     '\tif (jb())\n\t\tgoto loc_483300;\n'
     '\tif (jnb())\n\t\tgoto loc_4832FD;\n'
     ' ORIGINAL_INDEXED_ARITHMETIC_UNCHANGED();\n'
     'loc_4832FD:\n'+STORE+'loc_483300:\n'
     '\tesi = static_cast<int32_t>(guest_address(esi+2)); //lea\n'
     '\tdec(edi);\n\tif (jnz())\n\t\tgoto loc_483292;\n')
UNSUPPORTED=('loc_48307A:\n\tebx = to32i(edi); //mov\n'
             ' ORIGINAL_RGBA_ARITHMETIC_UNCHANGED();\n'+STORE)

def fixtures(prepared=True):
    result=text_fixtures()
    result['Methods_01.cpp']+=fn('410330',' ORIGINAL_CHART_SELECTION_UNCHANGED();\n')
    result['Methods_10.cpp']+=fn('483288',ROW)+fn('483070',UNSUPPORTED)+fn('499999',STORE)
    return {name:add_controls_steering_text_hooks(result[name],name) if prepared else result[name] for name in NAMES}

class Tests(unittest.TestCase):
    def test_exact_reversal_lf_crlf_and_prior_text_coexistence(self):
        for name,original in fixtures().items():
            for nl in ('\n','\r\n'):
                source=original.replace('\n',nl)
                after=add_controls_steering_chart_hooks(source,name)
                self.assertEqual(remove_controls_steering_chart_hooks(after).encode(),source.encode())
                self.assertEqual(after.count('guest_store<'),source.count('guest_store<'))
                self.assertEqual(after.count('nfs_controls_steering_text::'),source.count('nfs_controls_steering_text::'))
                self.assertEqual(re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',after),re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',source))
                self.assertNotIn('enqueue',after)
                with self.assertRaises(ValueError):add_controls_steering_chart_hooks(after,name)

    def test_include_and_scope_interfaces(self):
        for name,source in fixtures().items():
            after=add_controls_steering_chart_hooks(source,name)
            self.assertLess(after.index('// NFS_CONTROLS_STEERING_TEXT_END:include'),after.index('// '+MARKER+'_BEGIN:include'))
            if name=='Methods_01.cpp':
                self.assertIn(CODES['entry-410330'],section(after,'410330'))
                self.assertIn(CODES['entry-405B60'],section(after,'405B60'))
                for symbol in ('4101C0','403970','403FC0','40B2C0'):
                    self.assertEqual(section(source,symbol),section(after,symbol))
            else:
                row=section(after,'483288')
                self.assertIn(CODES['entry-483288'],row)
                self.assertIn(CODES['entry-483070'],section(after,'483070'))
                self.assertEqual(after.count('::WordCapture'),1)
                self.assertLess(row.index('loc_4832FD:'),row.index('::WordCapture'))
                self.assertLess(row.index('::WordCapture'),row.index(STORE))
                self.assertLess(row.index(STORE),row.index('// '+MARKER+'_BEGIN:word-close'))
                self.assertLess(row.index('// '+MARKER+'_END:word-close'),row.index('loc_483300:'))
                self.assertEqual(section(source,'499999'),section(after,'499999'))
                self.assertEqual(section(source,'482DA0'),section(after,'482DA0'))

    def test_complete_prior_text_collector_required(self):
        for name,source in fixtures(prepared=False).items():
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source,name)
        for name,source in fixtures().items():
            for token in ('#include "hd/ControlsSteeringTextGameHooks.inc"','NFS_CONTROLS_STEERING_TEXT_END:include'):
                with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source.replace(token,'changed',1),name)
            tag='entry-4101C0' if name=='Methods_01.cpp' else 'entry-482DA0'
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source.replace('NFS_CONTROLS_STEERING_TEXT_END:'+tag,'changed',1),name)

    def test_partial_prior_owner_and_removed_text_finish_refuse(self):
        for name,source in fixtures().items():
            body=' nfs_controls_steering_text::OwnerScope' if name=='Methods_01.cpp' else ' nfs_controls_steering_text::beforeDescriptorRetire'
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source.replace(body,' changed_prior_scope',1),name)
        source=fixtures()['Methods_01.cpp']
        missing=re.sub(r'\n// NFS_CONTROLS_STEERING_TEXT_BEGIN:finish-before-chart\n.*?// NFS_CONTROLS_STEERING_TEXT_END:finish-before-chart\n','',source,flags=re.S)
        with self.assertRaises(ValueError):add_controls_steering_chart_hooks(missing,'Methods_01.cpp')

    def test_changed_indexed_anchors_and_ambiguous_labels_refuse(self):
        source=fixtures()['Methods_10.cpp']
        for token in ('Game::_sub_483288','loc_483292:','dword_5636EC','loc_4832FD:','loc_483300:',STORE,'dec(edi);','goto loc_483292;','loc_48307A:','ebx = to32i(edi);'):
            with self.subTest(token=token),self.assertRaises(ValueError):
                add_controls_steering_chart_hooks(source.replace(token,'changed',1),'Methods_10.cpp')
        for label in ('loc_4832FD:','loc_483300:'):
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source.replace(label,label+'\n'+label,1),'Methods_10.cpp')
        for symbol in ('483288','483070'):
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source+fn(symbol),'Methods_10.cpp')
        changed=source.replace('loc_4832FD:\n'+STORE+'loc_483300:\n','loc_4832FD:\nloc_483300:\n'+STORE,1)
        with self.assertRaises(ValueError):add_controls_steering_chart_hooks(changed,'Methods_10.cpp')

    def test_missing_nested_and_unbalanced_functions_refuse(self):
        source=fixtures()['Methods_01.cpp']
        for symbol in ('410330','405B60'):
            body=section(source,symbol)
            for changed in ('',body.replace('{\n','{\nFn(void) Game::_sub_111111()\n{\n}\n',1),body.rsplit('}',1)[0]):
                with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source.replace(body,changed,1),'Methods_01.cpp')
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source+fn(symbol),'Methods_01.cpp')

    def test_checked_copy_leases_and_mixed_newlines_refuse(self):
        for name,source in fixtures().items():
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks('// checked copy transaction begin\n'+source,name)
            with self.assertRaises(ValueError):add_controls_steering_chart_hooks(source.replace('\n','\r\n',1),name)
        source=fixtures()['Methods_10.cpp'];changed=source.replace('loc_4832FD:\n'+STORE,'loc_4832FD:\n// checked copy transaction begin\n'+STORE+'// checked copy transaction end\n',1)
        with self.assertRaises(ValueError):add_controls_steering_chart_hooks(changed,'Methods_10.cpp')

    def test_partial_malformed_nested_and_changed_hook_removal_refuses(self):
        for name,source in fixtures().items():
            after=add_controls_steering_chart_hooks(source,name)
            for altered in (after.replace(MARKER+'_END:include','damaged',1),
                            after.replace(CODES['include'],CODES['include']+'\n changed();',1),
                            after+'\n// '+MARKER+'_BEGIN:partial\n',
                            after.replace('// '+MARKER+'_BEGIN:include','// '+MARKER+'_BEGIN:include\n// '+MARKER+'_BEGIN:nested',1)):
                with self.assertRaises(ValueError):remove_controls_steering_chart_hooks(altered)
            block='\n// '+MARKER+'_BEGIN:include\n'+CODES['include']+'\n// '+MARKER+'_END:include\n'
            with self.assertRaises(ValueError):remove_controls_steering_chart_hooks(after.replace(block,'',1))
            with self.assertRaises(ValueError):remove_controls_steering_chart_hooks(after+block)
            if name=='Methods_10.cpp':
                with self.assertRaises(ValueError):remove_controls_steering_chart_hooks(after.replace('loc_4832FD:\n','loc_changed:\n',1))
                with self.assertRaises(ValueError):remove_controls_steering_chart_hooks(after.replace(STORE,'changed_store();\n',1))

    def test_unsupported_units_unchanged_and_unhooked_removal_identity(self):
        for source in ('untouched','\r\nuntouched\r\n'):
            self.assertEqual(add_controls_steering_chart_hooks(source,'Methods_02.cpp'),source)
            self.assertEqual(remove_controls_steering_chart_hooks(source),source)

if __name__=='__main__':unittest.main()
