"""Publication boundary rejection checks; synthetic values only."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from publication_audit import content_findings, SAFE_EMAIL

class PublicationAuditTest(unittest.TestCase):
    def test_owned_files_and_forced_staging(self):
        for name in ("game/owned/file.dat","GAMEDATA/car.fsh",".build/run.log","secret.p12",".env.production"):
            self.assertIn("forbidden_file",content_findings(name,b"fixture"))
        self.assertIn("binary_file",content_findings("evidence/test.json",b"x\0y"))
        self.assertIn("symlink",content_findings("src/test.h",b"target","120000"))

    def test_sensitive_values(self):
        directory=b"C:"+b"\\"+b"Users"+b"\\"+b"example-user"+b"\\work"
        self.assertIn("personal_directory",content_findings("README.md",directory))
        for prefix in (b"ghp_",b"github_pat_"):
            self.assertIn("github_token",content_findings("src/test.h",prefix+b"A"*80))
        key=b"-----BEGIN "+b"PRIVATE KEY-----"
        self.assertIn("private_key",content_findings("src/test.h",key))

    def test_public_text_and_identity(self):
        self.assertEqual(content_findings("src/test.cpp",b'int main(){return 0;}'),[])
        self.assertTrue(SAFE_EMAIL.fullmatch("example@users.noreply.github.com"))
        self.assertFalse(SAFE_EMAIL.fullmatch("private@example.invalid"))

if __name__=="__main__":unittest.main()
