"""Pack explicitly supplied private shared-footer pixels and trusted source bindings.

No assets or source identities are embedded in this tool. Outputs belong in the
private build directory and are never suitable for public source publication.
"""
from pathlib import Path
import argparse, hashlib, json, struct, zlib


def digest(value):
    raw=bytes.fromhex(value)
    if len(raw)!=32 or not any(raw):
        raise ValueError("Expected a nonzero SHA256")
    return raw


def array(raw):
    return "{{"+",".join(str(x) for x in raw)+"}}"


def build(manifest_path, output):
    manifest_path=Path(manifest_path).resolve()
    manifest=json.loads(manifest_path.read_text(encoding="utf-8"))
    rows=manifest["images"]; routes=manifest["bindings"]
    if not 1<=len(rows)<=98 or not 1<=len(routes)<=98:
        raise ValueError("Footer catalog cardinality exceeds native contract")
    chunks=[];pins=[];seen=set();decoded=0;offset=16
    for row in rows:
        width,height=row["width"],row["height"]
        if not isinstance(width,int) or not isinstance(height,int) or not (0<width<=4096 and 0<height<=4096 and width*3==height*4):
            raise ValueError("Invalid footer dimensions")
        size=width*height*4;decoded+=size
        if size>64*1024**2 or decoded>256*1024**2:
            raise ValueError("Decoded footer memory budget exceeded")
        path=(manifest_path.parent/row["compressed"]).resolve()
        if not path.is_relative_to(manifest_path.parent):
            raise ValueError("Footer input must remain beside its private manifest")
        chunk=path.read_bytes()
        if not 0<len(chunk)<=32*1024**2:
            raise ValueError("Compressed chunk budget exceeded")
        # Bounded inflate also rejects concatenated streams/trailing data.
        decoder=zlib.decompressobj();raw=decoder.decompress(chunk,size+1)
        if len(raw)!=size or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise ValueError("Invalid exact footer pixel stream")
        sha=hashlib.sha256(raw).digest()
        if sha!=digest(row["rgbaSha256"]) or sha in seen:
            raise ValueError("Footer pixel hash mismatch or duplicate canvas")
        seen.add(sha)
        if any(max(raw[k:k+3])>raw[k+3] for k in range(0,size,4)):
            raise ValueError("Footer is not premultiplied RGBA")
        pins.append("    {%d,%d,%d,%d,%s,%s}"%(offset,len(chunk),width,height,array(hashlib.sha256(chunk).digest()),array(sha)))
        chunks.append(chunk);offset+=len(chunk)
    if offset>256*1024**2:raise ValueError("Footer file budget exceeded")
    bindings=[];seen.clear()
    for route in routes:
        archive=digest(route["archiveSha256"]);entry=digest(route["entrySha256"]);index=route["footerIndex"]
        if not isinstance(index,int) or not 0<=index<len(rows) or (archive,entry) in seen:
            raise ValueError("Invalid or duplicate source routing")
        seen.add((archive,entry));bindings.append("        {%s,%s,%d}"%(array(archive),array(entry),index))
    data=b"NFSFTR01"+struct.pack("<II",1,len(rows))+b"".join(chunks)
    header="""// PRIVATE GENERATED INPUT: do not publish assets or original identities.
#pragma once
#include "hd/SlideshowFooterResource.h"
namespace nfs_slideshow_footer_private {
static constexpr uint64_t fileBytes=%d;
static const nfs_slideshow_footer::ResourcePin images[]={
%s
};
inline std::vector<nfs_slideshow_footer::Binding> bindings(){return {
%s
};}
}
"""%(len(data),",\n".join(pins),",\n".join(bindings))
    output=Path(output);output.mkdir(parents=True,exist_ok=True)
    (output/"slideshow-footer.ownedz").write_bytes(data)
    (output/"SlideshowFooterData.h").write_text(header,encoding="utf-8")
    receipt={"images":len(rows),"bindings":len(routes),"fileBytes":len(data),"decodedBytes":decoded,"packSha256":hashlib.sha256(data).hexdigest(),"appIntegrated":False,"physicalIpadVerified":False}
    (output/"footer-pack-receipt.json").write_text(json.dumps(receipt,indent=2)+"\n",encoding="utf-8")
    return receipt


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest");parser.add_argument("output")
    args=parser.parse_args();print(json.dumps(build(args.manifest,args.output)))
