# SPDX-License-Identifier: MIT
"""Three source-only Apple assembly compilations; log evidence, never build an app."""
import hashlib,json,os,pathlib,re,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
VARIANTS=("old","candidate","unified")
# Frozen pre-promotion baseline: fail closed if canonical inputs drift.
PINNED_INPUTS={
    'src/GuestCopyTransaction.h': # SHA256 of public source bytes, not a credential.
        '72fb419cfed6acbcdfd0d1018820143c6081f1a1a9c0fc2dfd38811b30f2c76b',
    'src/GuestAccess.h': # SHA256 of public source bytes, not a credential.
        '9b96ebf525bae7c484d34b82248965a1a091aed5e4d6b3c1e367603c423c2413',
    'src/GuestRuntimeC.h': # SHA256 of public source bytes, not a credential.
        '7cd3eb3960dc15ef393c6e9feaff5bc5c96e7f07731ae395b7a221bc11f3c1e4',
    'src/WorkerControl.h': # SHA256 of public source bytes, not a credential.
        'b07d47a99feff9c1df1964807e5b920f4b8a278fb5ed60be9eb183bd932daa76',
    'tests/fixtures/stop_slot_address_cache/GuestCopyTransaction.h': # SHA256 of public source bytes, not a credential.
        '18f923b852e21c5869dc392964fa4dbd79d318269eb5fc67b7e61b6b657638ca',
    'tests/fixtures/unified_stop_tls/WorkerControl.h': # SHA256 of public source bytes, not a credential.
        'e2f75fce16fb9c737ccc14331be72f19fb99aa0446b9071c725d0b177bde0d77',
}
CLOSURE=("GuestCopyTransaction.h","GuestAccess.h","GuestRuntimeC.h","WorkerControl.h")
def materialize(root,folder):
    inputs={}
    for name,expected in PINNED_INPUTS.items():
        data=(root/name).read_bytes()
        if hashlib.sha256(data).hexdigest()!=expected:raise ValueError("Pinned probe input changed: "+name)
        inputs[name]=data
    result={}
    for variant in VARIANTS:
        target=folder/variant;target.mkdir()
        result[variant]={}
        for name in CLOSURE:
            source="src/"+name
            if variant=="candidate" and name=="GuestCopyTransaction.h":source="tests/fixtures/stop_slot_address_cache/GuestCopyTransaction.h"
            if variant=="unified" and name=="WorkerControl.h":source="tests/fixtures/unified_stop_tls/WorkerControl.h"
            (target/name).write_bytes(inputs[source]);result[variant][name]=hashlib.sha256(inputs[source]).hexdigest()
    return result

def command(clang,sdk,root,variant,output,closure):
    if variant not in VARIANTS:raise ValueError("Unknown variant")
    include=closure/variant
    return [clang,"-target","arm64-apple-ios16.0","-std=c++14","-O3","-fwrapv","-fno-strict-aliasing","-isysroot",sdk,"-ffile-prefix-map="+str(root)+"=/source","-fdebug-prefix-map="+str(root)+"=/source","-I"+str(include),"-ffile-prefix-map="+str(closure)+"=/probe","-S",str(root/"tests/apple_stop_slot_assembly_probe.cpp"),"-o",str(output)]
def inspect(text):
    selected={}
    for match in re.finditer(r"^(_nfs_probe_\w+):[^\n]*\n(.*?)(?=^\s*\.cfi_endproc)",text,re.M|re.S):
        body=match.group(2)
        selected[match.group(1)]={"tlv_address_references":len(re.findall(r"@TLVP",body)),"indirect_calls":len(re.findall(r"\bblr\b",body)),"live_atomic_loads":len(re.findall(r"\bldar(?:b|h)?\b",body)),"body":body}
    if set(selected)!={"_nfs_probe_accessor","_nfs_probe_transform","_nfs_probe_rep8"}:raise ValueError("Expected three probe functions")
    return {"functions":selected,"whole_translation_unit":{"tlv_references":len(re.findall(r"@TLVP",text)),"live_atomic_loads":len(re.findall(r"\bldar(?:b|h)?\b",text)),"has_cleanup":bool(re.search(r"mutex.*unlock|unique_lock.*D[012]",text)),"has_worker_stop_exception":"WorkerStopped" in text,"control_slot_symbols":text.count("current_worker_control"),"critical_depth_symbols":text.count("owned_critical_depth"),"deferral_symbols":text.count("worker_stop_deferrals"),"unified_slot_symbols":text.count("worker_stop_thread_state"),"conditional_branches":len(re.findall(r"\b(?:cbz|cbnz|tbz|tbnz|b\.eq|b\.ne)\b",text))}}
def main():
    if sys.platform!="darwin":raise SystemExit("Apple assembly execution requires Darwin; no fallback compiler")
    version=subprocess.check_output(["xcodebuild","-version"],text=True)
    if not version.startswith("Xcode 16.4\n"):raise SystemExit("Expected pinned Xcode16.4")
    clang=subprocess.check_output(["xcrun","--sdk","iphoneos","--find","clang++"],text=True).strip()
    sdk=subprocess.check_output(["xcrun","--sdk","iphoneos","--show-sdk-path"],text=True).strip()
    evidence={"scope":"Tiny source-only pre-ThinLTO ARM64 iOS codegen, not app/FPS/device proof","source_sha":os.environ.get("GITHUB_SHA","local"),"compiler":subprocess.check_output([clang,"--version"],text=True).splitlines()[0],"variants":{}}
    with tempfile.TemporaryDirectory(prefix="nfs-stop-slot-") as folder:
        closures=materialize(ROOT,pathlib.Path(folder))
        for variant in VARIANTS:
            output=pathlib.Path(folder)/(variant+".s")
            subprocess.run(command(clang,sdk,ROOT,variant,output,pathlib.Path(folder)),check=True,timeout=180)
            text=output.read_text();report=inspect(text)
            if not report["whole_translation_unit"]["live_atomic_loads"] or not report["whole_translation_unit"]["tlv_references"]:raise ValueError("Missing live atomic/TLS evidence")
            unit=report["whole_translation_unit"]
            slot_evidence=unit["unified_slot_symbols"] if variant=="unified" else all([unit["control_slot_symbols"],unit["critical_depth_symbols"],unit["deferral_symbols"]])
            if not all([unit["has_cleanup"],unit["has_worker_stop_exception"],slot_evidence,unit["conditional_branches"]]):raise ValueError("Missing stop/checkpoint/cleanup assembly evidence")
            report["header_closure_sha256"]=closures[variant]
            for name,entry in report["functions"].items():
                body=entry.pop("body");print("ASSEMBLY",variant,name)
                for line in body.splitlines()[:240]:print(line.replace(str(ROOT),"/source"))
                entry["printed_lines"]=min(240,len(body.splitlines()));entry["truncated"]=len(body.splitlines())>240
            evidence["variants"][variant]=report
    print(json.dumps(evidence,sort_keys=True,indent=2))
if __name__=="__main__":main()
