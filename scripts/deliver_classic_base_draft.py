"""Transfer an unfinished classic base through an unpublished, authenticated draft.
No Actions artifacts, personal artwork, original support files, or public release.
"""
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,hashlib,io,json,os,re,subprocess,tempfile,zipfile
from prepare_release_download import inspect,PREFIX,LICENSES,CLASSIC_LICENSES,LIMIT
REPOSITORY="nabeelalam/nfs2seios"
ASSET="NFSIISEGame-classic-base-NEEDS-PERSONAL-ARTWORK.zip"
ROOT=Path(__file__).resolve().parents[1]
def sha(data):return hashlib.sha256(data).hexdigest()
def prepare(ipa,manifest,source,output):
    ipa,manifest,output=map(Path,(ipa,manifest,output))
    metadata=inspect(ipa,manifest,source,unpublished_classic_base=True)
    if output.exists():raise ValueError("Existing draft transfer is never overwritten")
    payload=io.BytesIO();keep=LICENSES|CLASSIC_LICENSES|{"Info.plist","PkgInfo","NFSIISEGame"}
    with zipfile.ZipFile(ipa)as original,zipfile.ZipFile(payload,"w",zipfile.ZIP_DEFLATED)as stripped:
        for info in original.infolist():
            if info.filename[len(PREFIX):]in keep:stripped.writestr(info,original.read(info))
    metadata["support_sha256"]={name:value for name,value in metadata["support_sha256"].items()if name in LICENSES|CLASSIC_LICENSES}
    metadata["ipa_sha256"]=sha(payload.getvalue());output.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="classic-base-",dir=output.parent)as folder:
        folder=Path(folder);game=folder/"game.ipa";doc=folder/"game.json";game.write_bytes(payload.getvalue());doc.write_text(json.dumps(metadata,indent=2)+"\n")
        inspect(game,doc,source,assets_separate=True,unpublished_classic_base=True)
        with zipfile.ZipFile(output,"x",zipfile.ZIP_DEFLATED)as archive:
            archive.write(game,"NFSIISEGame-development-unsigned.ipa");archive.write(doc,"NFSIISEGame-development-unsigned.json")
            archive.writestr("BASE-ONLY.txt","Unfinished unsigned ARM64 base. Separately held personal artwork and original game files are required. This is not the finished app and has no device acceptance. Do not publish this draft.\n")
    if output.stat().st_size>LIMIT:raise ValueError("Draft base exceeds the unchanged64MiB bound")
    return {"source":source,"sha256":sha(output.read_bytes()),"bytes":output.stat().st_size,"publicReleaseReady":False,"personalArtworkPresent":False,"originalSupportFilesPresent":False,"actionsArtifactUpload":False}
def gh(*args):
    env=dict(os.environ,GH_HOST="github.com")
    result=subprocess.run(["gh","api",*args],cwd=ROOT,env=env,capture_output=True)
    if result.returncode:raise RuntimeError("GitHub draft-transfer operation failed (exit%d); credentials were not printed"%result.returncode)
    return json.loads(result.stdout)
def draft_check(release,tag,source):
    if release.get("draft")is not True or release.get("published_at")is not None or release.get("tag_name")!=tag or release.get("target_commitish")!=source or type(release.get("id"))is not int or release["id"]<=0:
        raise ValueError("Refusing a published or mismatched base transfer")
def deliver(output,source,receipt,request=gh):
    if os.environ.get("GITHUB_REPOSITORY")!=REPOSITORY or os.environ.get("GITHUB_SERVER_URL")!="https://github.com":raise ValueError("Wrong repository or host for base delivery")
    run=os.environ.get("GITHUB_RUN_ID","");attempt=os.environ.get("GITHUB_RUN_ATTEMPT","")
    if not re.fullmatch(r"[1-9][0-9]{0,19}",run)or not re.fullmatch(r"[1-9][0-9]{0,3}",attempt):raise ValueError("Invalid workflow identity")
    if request("repos/"+REPOSITORY).get("visibility")!="public":raise ValueError("Base build must remain in the public repository")
    tag="classic-base-"+run+"-"+attempt;data={"tag_name":tag,"target_commitish":source,"name":"Unsigned classic base - personal artwork required","draft":True,"prerelease":True,"make_latest":"false","generate_release_notes":False,"body":"Unpublished transfer of the checked ARM64 base only. No personal artwork or original support files. Not a finished app, signed app or device acceptance. Keep this release a draft; delete the temporary transfer after verified local download."}
    body=receipt.with_suffix(".request.json");body.write_text(json.dumps(data)+"\n")
    release=request("--method","POST","repos/"+REPOSITORY+"/releases","--input",str(body));draft_check(release,tag,source);identity=release["id"]
    summary={"repository":REPOSITORY,"tag":tag,"releaseId":identity,"source":source,"draft":True,"publiclyPublished":False,"actionsArtifactUpload":False};receipt.write_text(json.dumps(summary,indent=2)+"\n")
    asset=request("--method","POST","https://uploads.github.com/repos/"+REPOSITORY+"/releases/"+str(identity)+"/assets?name="+ASSET,"-H","Content-Type: application/zip","--input",str(output))
    expected=sha(output.read_bytes())
    if asset.get("name")!=ASSET or asset.get("state")!="uploaded"or asset.get("size")!=output.stat().st_size or asset.get("digest")!="sha256:"+expected:raise ValueError("Draft asset size or server digest mismatch")
    final=request("repos/"+REPOSITORY+"/releases/"+str(identity));draft_check(final,tag,source)
    if len(final.get("assets",[]))!=1 or final["assets"][0].get("id")!=asset.get("id"):raise ValueError("Unexpected assets in draft transfer")
    if request("repos/"+REPOSITORY).get("visibility")!="public":raise ValueError("Repository visibility changed during transfer")
    summary.update(assetId=asset["id"],sha256=expected,bytes=output.stat().st_size,serverDigestVerified=True);receipt.write_text(json.dumps(summary,indent=2)+"\n");return summary
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("--source",required=True);args=parser.parse_args()
    if not re.fullmatch(r"[0-9a-f]{40}",args.source):raise ValueError("Invalid source")
    directory=ROOT/".build";output=directory/ASSET;prepare(directory/"NFSIISEGame-development-unsigned.ipa",directory/"NFSIISEGame-development-unsigned.json",args.source,output)
    print(json.dumps(deliver(output,args.source,directory/"classic-base-draft.json"),sort_keys=True))
if __name__=="__main__":main()
