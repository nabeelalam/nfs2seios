# SPDX-License-Identifier: MIT
"""Two source-only Apple assembly compilations; log evidence, never build an app."""
import hashlib,json,os,pathlib,re,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
def command(clang,sdk,root,variant,output):
    include=root/("src" if variant=="old" else "tests/fixtures/stop_slot_address_cache")
    return [clang,"-target","arm64-apple-ios16.0","-std=c++14","-O3","-fwrapv","-fno-strict-aliasing","-isysroot",sdk,"-ffile-prefix-map="+str(root)+"=/source","-fdebug-prefix-map="+str(root)+"=/source","-I"+str(include),"-I"+str(root/"src"),"-S",str(root/"tests/apple_stop_slot_assembly_probe.cpp"),"-o",str(output)]
def inspect(text):
    selected={}
    for match in re.finditer(r"^(_nfs_probe_\w+):[^\n]*\n(.*?)(?=^\s*\.cfi_endproc)",text,re.M|re.S):
        body=match.group(2)
        selected[match.group(1)]={"tlv_address_references":len(re.findall(r"@TLVP",body)),"indirect_calls":len(re.findall(r"\bblr\b",body)),"live_atomic_loads":len(re.findall(r"\bldar(?:b|h)?\b",body)),"body":body}
    if set(selected)!={"_nfs_probe_accessor","_nfs_probe_transform","_nfs_probe_rep8"}:raise ValueError("Expected three probe functions")
    return {"functions":selected,"whole_translation_unit":{"tlv_references":len(re.findall(r"@TLVP",text)),"live_atomic_loads":len(re.findall(r"\bldar(?:b|h)?\b",text)),"has_cleanup":bool(re.search(r"mutex.*unlock|unique_lock.*D[012]",text)),"has_worker_stop_exception":"WorkerStopped" in text,"control_slot_symbols":text.count("current_worker_control"),"critical_depth_symbols":text.count("owned_critical_depth"),"deferral_symbols":text.count("worker_stop_deferrals"),"conditional_branches":len(re.findall(r"\b(?:cbz|cbnz|tbz|tbnz|b\.eq|b\.ne)\b",text))}}
def main():
    if sys.platform!="darwin":raise SystemExit("Apple assembly execution requires Darwin; no fallback compiler")
    version=subprocess.check_output(["xcodebuild","-version"],text=True)
    if not version.startswith("Xcode 16.4\n"):raise SystemExit("Expected pinned Xcode16.4")
    clang=subprocess.check_output(["xcrun","--sdk","iphoneos","--find","clang++"],text=True).strip()
    sdk=subprocess.check_output(["xcrun","--sdk","iphoneos","--show-sdk-path"],text=True).strip()
    evidence={"scope":"Tiny source-only pre-ThinLTO ARM64 iOS codegen, not app/FPS/device proof","source_sha":os.environ.get("GITHUB_SHA","local"),"compiler":subprocess.check_output([clang,"--version"],text=True).splitlines()[0],"variants":{}}
    with tempfile.TemporaryDirectory(prefix="nfs-stop-slot-") as folder:
        for variant in ["old","candidate"]:
            output=pathlib.Path(folder)/(variant+".s")
            subprocess.run(command(clang,sdk,ROOT,variant,output),check=True,timeout=180)
            text=output.read_text();report=inspect(text)
            if not report["whole_translation_unit"]["live_atomic_loads"] or not report["whole_translation_unit"]["tlv_references"]:raise ValueError("Missing live atomic/TLS evidence")
            unit=report["whole_translation_unit"]
            if not all([unit["has_cleanup"],unit["has_worker_stop_exception"],unit["control_slot_symbols"],unit["critical_depth_symbols"],unit["deferral_symbols"],unit["conditional_branches"]]):raise ValueError("Missing stop/checkpoint/cleanup assembly evidence")
            header=ROOT/("src/GuestCopyTransaction.h" if variant=="old" else "tests/fixtures/stop_slot_address_cache/GuestCopyTransaction.h")
            report["header_sha256"]=hashlib.sha256(header.read_bytes()).hexdigest()
            for name,entry in report["functions"].items():
                body=entry.pop("body");print("ASSEMBLY",variant,name)
                for line in body.splitlines()[:240]:print(line.replace(str(ROOT),"/source"))
                entry["printed_lines"]=min(240,len(body.splitlines()));entry["truncated"]=len(body.splitlines())>240
            evidence["variants"][variant]=report
    print(json.dumps(evidence,sort_keys=True,indent=2))
if __name__=="__main__":main()
