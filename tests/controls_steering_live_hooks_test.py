# SPDX-License-Identifier: MIT
"""Synthetic composition and exact original full-frame boundaries."""
from pathlib import Path
import sys,re,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from controls_steering_live_hooks import NAMES,MARKER,add_controls_steering_live_hooks as add,remove_controls_steering_live_hooks as remove,call,block
from controls_steering_sidebar_hooks_test import fixtures
from controls_steering_sidebar_hooks import add_controls_steering_sidebar_hooks
from controls_text_hooks_test import section

def bases():
 result={n:add_controls_steering_sidebar_hooks(s,n)for n,s in fixtures().items()if n in NAMES}
 source=result['Methods_01.cpp'];part=section(source,'4101C0');before=part
 reset='\tesp -= 4; _sub_428A20(); esp += 4; //call\n\tesp -= 4; _sub_40EB20(); esp += 4; //call\n\tedx = 0x400; //mov\n\teax = to32i(esi+4); //mov\n\txor_(ecx, ecx);\n\txor_(ebx, ebx);\n'
 side='\teax = to32i(dword_4D2314+2); //mov\n\tsar(eax, (int32_t)0x10);\n\teax = to32i(dword_4D231C+eax*4); //mov\n\teax = to32i(eax+0x20); //mov\n\tedx = 0x1000; //mov\n\txor_(ecx, ecx);\n\txor_(ebx, ebx);\n'
 part=part.replace(call('405B30'),reset+call('405B30'),1).replace(call('405B60'),side+call('405B60'),1)
 result['Methods_01.cpp']=source.replace(before,part,1);return result
class SteeringLiveSource(unittest.TestCase):
 def test_full_composition_and_reversal_both_endings(self):
  for name,s in bases().items():
   for source in (s,s.replace('\n','\r\n')):
    with self.subTest(name=name,crlf='\r'in source):
     out=add(source,name);self.assertEqual(remove(out),source)
     for token in ('guest_store<','_sub_','nfs_controls_setup_','nfs_controls_sidebar_','nfs_controls_steering_text::','nfs_controls_steering_chart_live::'):
      self.assertEqual(out.count(token),source.count(token))
     self.assertNotIn('enqueue_renderer',out)
 def test_complete_original_frame_order(self):
  s=bases()['Methods_01.cpp'];out=add(s,'Methods_01.cpp');part=section(out,'4101C0')
  for a,b in [('steering_text::OwnerScope','steering_live::OwnerScope'),('steering_live::beginTail','steering_text::beginTail'),('steering_text::finishTail','steering_live::finishTail'),('steering_live::finishTail','_sub_410330()'),('_sub_410330()','steering_live::finishChart'),('steering_live::finishChart','_sub_405C70()')]:self.assertLess(part.index(a),part.index(b))
  for tag,sym in [('reset','405B30'),('sidebar','405B60')]:self.assertIn(block(tag+'-open')+call(sym)+block(tag+'-close'),part)
  self.assertEqual(section(out,'403970'),section(s,'403970'))
  self.assertEqual(section(out,'410330'),section(s,'410330'))
 def test_mutated_original_calls_order_and_arguments_fail(self):
  s=bases()['Methods_01.cpp']
  for before,after in [(call('410330'),call('410331')),('\tedx = 0x400; //mov\n','\tedx = 0x800; //mov\n'),('\tedx = 0x1000; //mov\n','\tedx = 0x800; //mov\n'),('loc_41020E:','loc_changed:')]:
   with self.assertRaises(ValueError):add(s.replace(section(s,'4101C0'),section(s,'4101C0').replace(before,after,1),1),'Methods_01.cpp')
  out=add(s,'Methods_01.cpp')
  with self.assertRaises(ValueError):remove(out.replace(block('reset-open')+call('405B30'),block('reset-open')+'changed();',1))
 def test_missing_dependencies_partial_changed_duplicate_hooks_fail(self):
  s=bases()['Methods_01.cpp']
  for header in ('ControlsSteeringTextGameHooks.inc','ControlsSteeringChartGameHooks.inc'):
   with self.assertRaises(ValueError):add(s.replace(header,'Wrong.h',1),'Methods_01.cpp')
  out=add(s,'Methods_01.cpp')
  with self.assertRaises(ValueError):add(out,'Methods_01.cpp')
  for changed in (out.replace(block('owner'),'',1),out+block('owner'),out.replace('nfs_controls_steering_live::finishChart','bad::finishChart',1)):
   with self.assertRaises(ValueError):remove(changed)
 def test_transactions_mixed_newlines_and_unsupported(self):
  s=bases()['Methods_01.cpp'];s=s.replace('#include "FunctionIds.h"\n','#include "FunctionIds.h"\n// checked copy transaction begin\n',1)
  with self.assertRaises(ValueError):add(s,'Methods_01.cpp')
  s=bases()['Methods_01.cpp'];s=s.replace('\n','\r\n',1)
  with self.assertRaises(ValueError):add(s,'Methods_01.cpp')
  self.assertEqual(add('unrelated','Methods_02.cpp'),'unrelated');self.assertEqual(remove('unrelated'),'unrelated')
if __name__=='__main__':unittest.main()
