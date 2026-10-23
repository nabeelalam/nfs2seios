"""Audio finite body hooks preserve every original call and lease boundary."""
# SPDX-License-Identifier: MIT
import sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'scripts'))
from audio_dialog_word_hooks import add_audio_dialog_word_hooks,remove_audio_dialog_word_hooks
SOURCE='#include "FunctionIds.h"\nFn(void) Game::_sub_405580()\n{\n\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction tx;\n\t} // checked copy transaction end\n\tesp -= 4; _sub_41B710(); esp += 4; //call\n\tecx = eax; //mov\n\tesp -= 4; _sub_482424(); esp += 4; //call\n// NFS_AUDIO_SIDEBAR_MEASURE_END:measured-right\n\tsub(esi, eax);\n// NFS_AUDIO_SIDEBAR_MEASURE_END:measured-center\n\tedx = eax; //mov\n}\n'
class AudioDialogWordHooks(unittest.TestCase):
 def test_exact_original_byte_reversal_both_newlines(self):
  for source in (SOURCE,SOURCE.replace('\n','\r\n')):
   after=add_audio_dialog_word_hooks(source,'Methods_01.cpp');self.assertEqual(remove_audio_dialog_word_hooks(after).encode(),source.encode());self.assertEqual(after.count('nfs_audio_text::DialogPolicyScope'),1);self.assertLess(after.index('nfs_audio_text::DialogPolicyScope'),after.index('checked copy transaction begin'));self.assertGreater(after.index('nfs_audio_text::dialogResolvedResource'),after.index('checked copy transaction end'));self.assertLess(after.index('nfs_audio_text::dialogMeasured'),after.index('sub(esi, eax)'));self.assertIn('uint32_t(ecx),int32_t(eax),int32_t(esi),2',after);self.assertIn('uint32_t(ecx),int32_t(eax),int32_t(esi),1',after)
 def test_anchors_missing_duplicate_reordered_fail_closed(self):
  for token in ('_sub_41B710','NFS_AUDIO_SIDEBAR_MEASURE_END:measured-right','NFS_AUDIO_SIDEBAR_MEASURE_END:measured-center','Fn(void) Game::_sub_405580'):
   with self.assertRaises(ValueError):add_audio_dialog_word_hooks(SOURCE.replace(token,token+'Changed'),'Methods_01.cpp')
  with self.assertRaises(ValueError):add_audio_dialog_word_hooks(SOURCE+SOURCE,'Methods_01.cpp')
  with self.assertRaises(ValueError):add_audio_dialog_word_hooks(add_audio_dialog_word_hooks(SOURCE,'Methods_01.cpp'),'Methods_01.cpp')
 def test_changed_guest_lease_scope_declines(self):
  source=SOURCE.replace('\t} // checked copy transaction end\n','',1).replace('\tecx = eax; //mov','\t} // checked copy transaction end\n\tecx = eax; //mov',1)
  with self.assertRaises(ValueError):add_audio_dialog_word_hooks(source,'Methods_01.cpp')
 def test_other_methods_untouched(self):
  self.assertEqual(add_audio_dialog_word_hooks(SOURCE,'Methods_03.cpp'),SOURCE)
if __name__=='__main__':unittest.main()
