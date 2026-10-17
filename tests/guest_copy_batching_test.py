import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import unittest
from batch_guest_copies import eligible,batch_copies
MEM='eax = to32i(esi);\nebx = to32i(esi+4);\nedx = to32i(esi+8);\nguest_store<int32_t>(edi,eax);\n'
class PreparationTests(unittest.TestCase):
    def test_two_checked_copies_share_one_scope(self):
        result,blocks=batch_copies('eax = to32i(esi);\nguest_store<int32_t>(edi,eax);\n');self.assertEqual(len(blocks),1);self.assertEqual(blocks[0]['checkedCopies'],2);self.assertIn('guest_transaction.load<int32_t>',result);self.assertIn('guest_transaction.store<int32_t>',result)
    def test_single_copy_stays_unbatched(self):
        result,blocks=batch_copies('eax = to32i(esi);\n');self.assertFalse(blocks);self.assertNotIn('GuestCopyTransaction',result)
    def test_four_checked_copies_share_one_scope(self):
        result,blocks=batch_copies(MEM);self.assertEqual(len(blocks),1);self.assertEqual(blocks[0]['checkedCopies'],4);self.assertIn('guest_transaction.store<int32_t>',result)
    def test_external_call_is_never_inside_transaction(self):
        result,blocks=batch_copies(MEM+'esp -= 4; _sub_123456(); esp += 4; //call\n'+MEM)
        self.assertEqual(len(blocks),2);self.assertIn('} // checked copy transaction end\nesp -= 4; _sub_',result)
    def test_unbraced_control_does_not_absorb_following_statements(self):
        result,blocks=batch_copies('if (jz())\n'+MEM);self.assertEqual(len(blocks),1);self.assertEqual(blocks[0]['checkedCopies'],3);self.assertIn('if (jz())\neax = to32i(esi);\n{ // checked copy transaction begin',result)
    def test_labels_and_declarations_are_barriers(self):
        result,blocks=batch_copies(MEM+'loc_123456:\n'+MEM+'nfs::CodecOperationGuard guard{uint32_t(eax)};\n'+MEM)
        self.assertEqual(len(blocks),3);self.assertIn('} // checked copy transaction end\nloc_',result)
    def test_copy_and_line_bounds(self):
        _,blocks=batch_copies(MEM*40);self.assertTrue(blocks);self.assertTrue(all(b['checkedCopies']<=32 and b['lines']<=64 for b in blocks))
    def test_nested_copy_helpers_and_unsafe_statements_rejected(self):
        for line in ['push32(eax);','pop32(eax);','movsd();','eax = malloc32(4);','eax = data[index];','eax = object->field;','{ const auto mapped_address = edi; auto mapped_value=guest_load<int32_t>(edi); pop32(mapped_value); guest_store<int32_t>(edi,mapped_value); };']:
            with self.subTest(line=line):self.assertFalse(eligible(line))
    def test_one_line_operand_local_is_rewritten(self):
        line='{ const auto mapped_address = edi; auto mapped_value = guest_load<float>(mapped_address); fstp(mapped_value); guest_store<float>(mapped_address, mapped_value); };\n'
        result,blocks=batch_copies(line*2);self.assertEqual(len(blocks),1);self.assertNotIn('guest_load<',result);self.assertNotIn('guest_store<',result)
    def test_spaced_helper_is_rewritten_and_block_comments_are_barriers(self):
        result,blocks=batch_copies(MEM.replace('to32i(', 'to32i ('));self.assertEqual(len(blocks),1);self.assertNotIn('to32i (',result)
        _,blocks=batch_copies('/* disabled\n'+MEM+'*/\n');self.assertFalse(blocks)
    def test_hidden_declarations_and_raw_native_dereferences_rejected(self):
        for line in ['eax = 0; std::mutex hidden;', 'eax = *native;', 'eax = *(native);', 'eax = (callback)();', 'eax = (*callback)();', '{ const auto mapped_address = edi; std::mutex hidden; };']:
            with self.subTest(line=line):self.assertFalse(eligible(line))
if __name__=='__main__':unittest.main()
