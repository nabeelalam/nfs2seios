"""Public synthetic original-call fixtures for additive Controls text observations."""
# SPDX-License-Identifier: MIT
import re
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from controls_text_hooks import NAMES,MARKER,add_controls_text_hooks,remove_controls_text_hooks

HEADER='#include "FunctionIds.h"\n'
def call(symbol):return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call\n'
def fn(symbol,body=''):return 'Fn(void) Game::_sub_'+symbol+'()\n{\n'+body+'}\n'
LEASE='\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction tx;\n\t} // checked copy transaction end\n'
FOREIGN='\n// NFS_EXISTING_OWNER_BEGIN:kept\n existing_observation(*this);\n// NFS_EXISTING_OWNER_END:kept\n'
INPUT=fn('403FC0',call('414100')+FOREIGN+'\tguest_store<int32_t>(eax, int32_t(edx));\n')+fn('414000',call('414200')+'\teax = ecx; //mov\n')+fn('414200','\treturn;\n')
CENTER='\tedx = eax; //mov\n\tsar(edx, (int32_t)0x1F);\n\tsub(eax, edx);\n\tsar(eax, (int32_t)1);\n\tedx = si; //movsx\n\tsub(edx, eax);\n\tesi = edx; //mov\n'
def fixtures():
    normal='loc_41020E:\n'+LEASE+call('405B30')+FOREIGN+call('405B60')+'\teax = esi; //mov\n'+FOREIGN+call('40FA90')+'loc_410299:\n\treturn;\n'
    policy=LEASE+call('41B690')+FOREIGN+call('41B710')+'loc_4055D3:\n\teax = ecx; //mov\n'+call('482424')+FOREIGN+'\tsub(esi, eax);\nloc_4055DC:\n\treturn;\nloc_405646:\n\teax = ecx; //mov\n'+call('482424')+FOREIGN+CENTER+'loc_405662:\n\treturn;\n'
    store='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n'
    return {
        'Methods_01.cpp':HEADER+INPUT+fn('4101C0',normal)+fn('4054E0',LEASE)+fn('405580',policy)+fn('405CF0',LEASE)+fn('405B30',' ORIGINAL_RESET_UNCHANGED();\n')+fn('405B60',' ORIGINAL_SIDEBAR_UNCHANGED();\n'),
        'Methods_09.cpp':HEADER+fn('4824CC',LEASE)+fn('414400',' INPUT_UNCHANGED();\n'),
        'Methods_10.cpp':HEADER+fn('482DA0',LEASE)+fn('482BD4',call('414600')),
        'Methods_13.cpp':HEADER+fn('4A5830',LEASE+FOREIGN+store*4),
        'Methods_14.cpp':HEADER+fn('4AC934',LEASE)+fn('4AC9C8',LEASE)+fn('4B3EE0',LEASE+FOREIGN+store),
    }
def section(source,symbol):
    return re.search(r'Fn\(void\) Game::_sub_'+symbol+r'\(\)\r?\n\{.*?(?=\r?\nFn\(void\) Game::|\Z)',source,re.S).group()

class ControlsTextHooks(unittest.TestCase):
    def test_exact_byte_reversal_original_calls_and_stores_both_newlines(self):
        for name,base in fixtures().items():
            for source in (base,base.replace('\n','\r\n')):
                after=add_controls_text_hooks(source,name)
                self.assertEqual(remove_controls_text_hooks(after).encode(),source.encode())
                self.assertEqual(re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',after),re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',source))
                self.assertEqual(after.count('guest_store<'),source.count('guest_store<'))
                self.assertEqual(after.count('existing_observation'),source.count('existing_observation'))
                self.assertNotIn('nfs_controls_live',after)
                self.assertNotIn('activeRequest',after)
                self.assertNotIn('enqueue_renderer',after)
    def test_owner_policy_tail_and_real_register_measurement_order(self):
        source=fixtures()['Methods_01.cpp'];after=add_controls_text_hooks(source,'Methods_01.cpp')
        self.assertIn('PolicyScope nfs_controls_policy(*this,true)',after)
        self.assertIn('PolicyScope nfs_controls_policy(*this,false)',after)
        self.assertEqual(after.count('nfs_controls_text::DialogPolicyScope'),1)
        self.assertEqual(after.count('nfs_controls_text::TailScope'),1)
        normal=section(after,'4101C0')
        self.assertLess(normal.index('OwnerScope'),normal.index('loc_41020E:'))
        self.assertLess(normal.index('TailScope'),normal.index('_sub_40FA90()'))
        self.assertLess(normal.index('_sub_405B60()'),normal.index('TailScope'))
        policy=section(after,'405580')
        for name in ('resourceAlignment','dialogResourceAlignment','dialogResolvedResource'):
            self.assertEqual(policy.count('nfs_controls_text::'+name+'('),1)
        self.assertLess(policy.index('_sub_41B690()'),policy.index('dialogResourceAlignment'))
        self.assertLess(policy.index('_sub_41B710()'),policy.index('dialogResolvedResource'))
        right=policy[policy.index('loc_4055D3:'):policy.index('loc_4055DC:')]
        centre=policy[policy.index('loc_405646:'):policy.index('loc_405662:')]
        for region,kind,mutate in ((right,2,'sub(esi, eax)'),(centre,1,'edx = eax')):
            self.assertLess(region.index('_sub_482424()'),region.index('dialogMeasured'))
            self.assertLess(region.index('dialogMeasured'),region.index(mutate))
            self.assertIn('uint32_t(ecx),int32_t(eax),int32_t(esi),'+str(kind),region)
    def test_input_and_reset_sidebar_functions_unchanged(self):
        source=fixtures()['Methods_01.cpp'];after=add_controls_text_hooks(source,'Methods_01.cpp')
        for symbol in ('403FC0','414000','414200','405B30','405B60'):
            self.assertEqual(section(source,symbol),section(after,symbol))
        for name,symbol in (('Methods_09.cpp','414400'),('Methods_10.cpp','482BD4')):
            source=fixtures()[name];after=add_controls_text_hooks(source,name)
            self.assertEqual(section(source,symbol),section(after,symbol))
    def test_exact_word_store_scope_count(self):
        for name,count in (('Methods_13.cpp',4),('Methods_14.cpp',1)):
            source=fixtures()[name];after=add_controls_text_hooks(source,name)
            self.assertEqual(after.count('nfs_controls_text::WordCapture'),count)
            with self.assertRaises(ValueError):add_controls_text_hooks(source.replace('guest_store<int16_t>','other_store<int16_t>',1),name)
    def test_missing_duplicate_and_reordered_anchors_decline(self):
        source=fixtures()['Methods_01.cpp']
        tokens=('#include "FunctionIds.h"','Game::_sub_4101C0','Game::_sub_405580','Game::_sub_4054E0','Game::_sub_405CF0','loc_41020E:','loc_4055D3:','loc_405646:','_sub_41B690()','_sub_41B710()','_sub_405B30()','_sub_405B60()','_sub_40FA90()','sub(esi, eax);','sar(eax, (int32_t)1);')
        for token in tokens:
            with self.subTest(token=token),self.assertRaises(ValueError):add_controls_text_hooks(source.replace(token,token+'changed',1),'Methods_01.cpp')
        for extra in (HEADER,fn('405580'),fn('4101C0')):
            with self.assertRaises(ValueError):add_controls_text_hooks(source+extra,'Methods_01.cpp')
        right='loc_4055D3:\n\teax = ecx; //mov\n'+call('482424')
        with self.assertRaises(ValueError):add_controls_text_hooks(source.replace(right,right+call('482424')),'Methods_01.cpp')
        with self.assertRaises(ValueError):add_controls_text_hooks(source.replace('\tsub(esi, eax);\n','',1).replace('loc_4055D3:\n','loc_4055D3:\n\tsub(esi, eax);\n',1),'Methods_01.cpp')
    def test_checked_copy_leases_decline_all_observation_families(self):
        for name,base in fixtures().items():
            with self.assertRaises(ValueError):add_controls_text_hooks(base.replace(HEADER,HEADER+'// checked copy transaction begin\n',1),name)
        source=fixtures()['Methods_01.cpp']
        for anchor in (call('41B690'),call('41B710'),call('40FA90'),call('482424')):
            changed=source.replace(anchor,'// checked copy transaction begin\n'+anchor+'// checked copy transaction end\n',1)
            with self.assertRaises(ValueError):add_controls_text_hooks(changed,'Methods_01.cpp')
        for name in ('Methods_13.cpp','Methods_14.cpp'):
            source=fixtures()[name];store='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
            with self.assertRaises(ValueError):add_controls_text_hooks(source.replace(store,'// checked copy transaction begin\n'+store+'\n// checked copy transaction end',1),name)
    def test_duplicate_application_and_damaged_removal_decline(self):
        for name,source in fixtures().items():
            after=add_controls_text_hooks(source,name)
            with self.assertRaises(ValueError):add_controls_text_hooks(after,name)
            with self.assertRaises(ValueError):remove_controls_text_hooks(after.replace(MARKER+'_END:include',MARKER+'_END:wrong',1))
        self.assertEqual(remove_controls_text_hooks('unrelated'), 'unrelated')
        self.assertEqual(add_controls_text_hooks('unchanged','Methods_03.cpp'),'unchanged')
        self.assertEqual(len(NAMES),5)

if __name__=='__main__':unittest.main()
