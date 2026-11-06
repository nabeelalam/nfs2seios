"""Unpublished base transfer must never weaken public release or privacy checks."""
import hashlib,json,os,sys,tempfile,unittest,zipfile
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
import release_download_test as fixtures
from deliver_classic_base_draft import prepare,deliver,ASSET,REPOSITORY
from prepare_release_download import inspect,PREFIX,LICENSES,CLASSIC_LICENSES
class DraftTransferTest(unittest.TestCase):
    def fixture(self,folder):
        ipa,manifest,source=fixtures.DownloadTest().fixture(folder,classic=True)
        data=json.loads(manifest.read_text());data.update(personal_artwork_required=True,public_release_ready=False,package_kind="classic-public-base-needs-personal-artwork");manifest.write_text(json.dumps(data));return ipa,manifest,source
    def test_stripped_transfer_preserves_binary_and_default_public_refusal(self):
        with tempfile.TemporaryDirectory()as folder:
            ipa,manifest,source=self.fixture(folder);before=hashlib.sha256(ipa.read_bytes()).hexdigest();out=Path(folder)/"base.zip";receipt=prepare(ipa,manifest,source,out)
            with self.assertRaisesRegex(ValueError,"personal artwork"):inspect(ipa,manifest,source)
            with zipfile.ZipFile(out)as z:
                self.assertEqual(set(z.namelist()),{"BASE-ONLY.txt","NFSIISEGame-development-unsigned.ipa","NFSIISEGame-development-unsigned.json"});game=Path(folder)/"stripped.ipa";doc=game.with_suffix('.json');game.write_bytes(z.read('NFSIISEGame-development-unsigned.ipa'));doc.write_bytes(z.read('NFSIISEGame-development-unsigned.json'))
            inspect(game,doc,source,assets_separate=True,unpublished_classic_base=True)
            with zipfile.ZipFile(game)as a,zipfile.ZipFile(ipa)as b:
                self.assertEqual(set(a.namelist()),{PREFIX+n for n in LICENSES|CLASSIC_LICENSES|{'Info.plist','PkgInfo','NFSIISEGame'}});self.assertEqual(a.read(PREFIX+'NFSIISEGame'),b.read(PREFIX+'NFSIISEGame'))
            self.assertEqual(before,hashlib.sha256(ipa.read_bytes()).hexdigest());self.assertFalse(receipt['personalArtworkPresent']);self.assertFalse(receipt['originalSupportFilesPresent'])
            with self.assertRaisesRegex(ValueError,'overwritten'):prepare(ipa,manifest,source,out)
    def test_false_acceptance_or_wrong_source_rejected(self):
        with tempfile.TemporaryDirectory()as folder:
            ipa,manifest,source=self.fixture(folder);out=Path(folder)/'base.zip'
            with self.assertRaisesRegex(ValueError,'identity'):prepare(ipa,manifest,'b'*40,out)
            data=json.loads(manifest.read_text());data['public_release_ready']=True;manifest.write_text(json.dumps(data))
            with self.assertRaisesRegex(ValueError,'unfinished'):prepare(ipa,manifest,source,out)
            self.assertFalse(out.exists())
    def transfer(self,published=False,bad_digest=False,wrong_repo=False):
        with tempfile.TemporaryDirectory()as folder:
            folder=Path(folder);out=folder/'base.zip';out.write_bytes(b'bounded fixture');source='a'*40;receipt=folder/'receipt.json';calls=[];tag='classic-base-123-1'
            release={'id':7,'tag_name':tag,'target_commitish':source,'draft':not published,'published_at':None if not published else 'time','assets':[{'id':9}]}
            def request(*args):
                calls.append(args)
                if args==('repos/'+REPOSITORY,):return {'visibility':'private'if wrong_repo else'public'}
                if args[0]=='--method'and args[1]=='POST'and args[2].endswith('/releases'):
                    body=json.loads(Path(args[-1]).read_text());self.assertTrue(body['draft']);self.assertFalse(body['generate_release_notes']);return release
                if args[0]=='--method'and args[2].startswith('https://uploads.github.com/'):
                    return {'id':9,'name':ASSET,'state':'uploaded','size':out.stat().st_size,'digest':'sha256:'+('0'*64 if bad_digest else hashlib.sha256(out.read_bytes()).hexdigest())}
                return release
            env={'GITHUB_REPOSITORY':REPOSITORY,'GITHUB_SERVER_URL':'https://github.com','GITHUB_RUN_ID':'123','GITHUB_RUN_ATTEMPT':'1'}
            with patch.dict(os.environ,env):
                if published or bad_digest or wrong_repo:
                    with self.assertRaises(ValueError):deliver(out,source,receipt,request)
                else:
                    result=deliver(out,source,receipt,request);self.assertTrue(result['draft']);self.assertFalse(result['publiclyPublished']);self.assertTrue(result['serverDigestVerified'])
            self.assertTrue(all('PATCH'not in call for call in calls))
            if published or wrong_repo:self.assertFalse(any('https://uploads.github.com/'in str(call)for call in calls))
    def test_remote_delivery_stays_draft(self):self.transfer()
    def test_published_response_never_uploads(self):self.transfer(published=True)
    def test_bad_remote_digest_rejected(self):self.transfer(bad_digest=True)
    def test_private_repository_never_uploads(self):self.transfer(wrong_repo=True)
if __name__=='__main__':unittest.main()
