# SPDX-License-Identifier: MIT
"""Source-only foundation gates; C++ behavioral fixtures require a later build."""
from pathlib import Path
import re
import unittest
ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "src/hd/LoadSaveSourcePolicy.h"
class Contracts(unittest.TestCase):
    def test_exact_dedicated_scope(self):
        s=HEADER.read_text()
        self.assertIn("FormSymbol=0x4d39a8,HandlerSymbol=0x40938c,FormType=9",s)
        self.assertIn("depth==1&&!nested&&!retired&&!unwound",s)
        self.assertNotIn("OptionsScene",s)
        self.assertNotIn("GraphicsDialog",s)
    def test_finite_static_word_geometry(self):
        s=HEADER.read_text()
        self.assertIn('"save tournament","load tournament","delete tournament","save replay","view replay","delete replay","DONE","LOAD/SAVE"',s)
        self.assertIn("{144,170,196,222,248,274,317,105}",s)
        self.assertIn("0x4d3908,0x4d391c,0x4d3930,0x4d3944,0x4d3958,0x4d396c,0x4d3980,0x4d3994",s)
        self.assertIn("s.resourcePolicy!=4",s)
        self.assertIn("a.selected>=7",s)
        self.assertIn("c.x!=426-advance+int(shadow)",s)
    def test_no_actual_profile_or_live_issuer(self):
        s=HEADER.read_text()
        self.assertIn("supportedOriginalProfile(const Profile&)noexcept{return false;}",s)
        for forbidden in ("GuestCopyTransaction", "nfs_host_address", "Game&", "enqueue_owned_renderer", "fopen(", "SDL_"):
            self.assertNotIn(forbidden,s)
    def test_replay_checks_are_not_only_packet_labels(self):
        s=HEADER.read_text()
        for required in ("fontBytes==p.fontBytes", "w.before!=replay[index]", "w.computed!=computed", "w.after!=computed", "store!=use.stores.size()", "replay==packet->original()", "packet->matches(", "writes==packet->writes()", "x%2?byte&15:byte>>4"):
            self.assertIn(required,s)
    def test_behavioral_fixture_includes_rejection_boundaries(self):
        s=(ROOT/"tests/loadsave_source_policy_tests.cpp").read_text()
        for required in ("std::swap(f.evidence.strings[0]", "o.nested=true", "o.retired=true", "o.unwound=true", "owner.context", "profile.fontBytes[0]", "profile.glyphs[0].pitch", "profile.palettes[4]", "glyphs[0].stores.pop_back", "f.post[0]^=1", "!ls::supportedOriginalProfile"):
            self.assertIn(required,s)
        self.assertGreaterEqual(s.count("fault([&]"),50)
    def test_not_registered_into_current_renderer(self):
        # This stage intentionally cannot change compiled production behavior.
        for name in ("scripts/prepare_renderer.py", "CMakeLists.txt"):
            self.assertNotIn("LoadSaveSourcePolicy",(ROOT/name).read_text())
if __name__ == "__main__": unittest.main()
