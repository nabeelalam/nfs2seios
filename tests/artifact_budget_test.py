"""Upload size gates must reject absent or excessive package payloads."""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
import check_artifact_budget as budget

class ArtifactBudgetTest(unittest.TestCase):
    def test_aggregate_upload_boundary(self):
        old_root,old_limit=budget.ROOT,budget.MAX_BYTES
        try:
            with tempfile.TemporaryDirectory() as folder:
                budget.ROOT=Path(folder);budget.MAX_BYTES=16
                (budget.ROOT/".build").mkdir()
                with self.assertRaisesRegex(RuntimeError,"No validated"):budget.main()
                game=budget.ROOT/".build/NFSIISEGame-development-unsigned.ipa"
                game.write_bytes(b"fixture")
                with contextlib.redirect_stdout(io.StringIO()):budget.main()
                (budget.ROOT/".build/NFSIISEProbe-unsigned.ipa").write_bytes(b"fixture-too-large")
                with self.assertRaisesRegex(RuntimeError,"exceeds"):budget.main()
        finally:budget.ROOT,budget.MAX_BYTES=old_root,old_limit
if __name__=="__main__":unittest.main()
