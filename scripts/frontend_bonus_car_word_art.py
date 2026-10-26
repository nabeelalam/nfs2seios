"""Three finite bonus-value images appended after the unchanged legacy atlas."""
# SPDX-License-Identifier: MIT
import hashlib
import json
import math
import re

WIDTH, BASE_HEIGHT, HEIGHT = 1024, 2048, 2120
BASE_BYTES, RGBA_BYTES = WIDTH * BASE_HEIGHT * 4, WIDTH * HEIGHT * 4
SOURCE_SHA = "b70f216494fb34a4c7d52a491136f7806b8f5c67ad08151b3721c80af1f7f140"
BASE_SHA = "3cfbcbe7c3b9f0330b629b9bbfdc75dc6a5ecb25c3a64c8a063d8e798c2340a4"
LAYOUT_SHA = "d03f5b97ba4a69f40986395c77da48325a7f12d9242e08c002f2c2f34853ebb7"
GEOMETRY_SHA = "696f65f84a1a0df18dbbf425dbd10cded26ce191cc326a87c223219d3799b5d8"
NAMES = ("Bomber BFS", "Tombstone", "FZR 2000")
CONTEXTS = ("93a5cbd30dad057f", "7b8fa054abe43f28", "92e3d1d6c6ede731")
ATLAS = ((2,2050,312,68), (318,2050,280,68), (602,2050,240,68))
BEGIN = ",\n/* NFS_BONUS_CAR_WORDS_BEGIN */\n// Fixed-word provenance: assets/hud/frontend/BONUS_CAR_WORD_ART.md.\n"
END = "\n/* NFS_BONUS_CAR_WORDS_END */"


def _pairs(values):
    out = {}
    for key, value in values:
        if key in out:
            raise ValueError("Duplicate bonus text-art key")
        out[key] = value
    return out


def load(directory, geometry_path):
    path = directory / "bonus-car-word-art.json"
    raw = path.read_bytes()
    if len(raw) > 524288 or hashlib.sha256(raw).hexdigest() != SOURCE_SHA:
        raise ValueError("Bonus fixed-word source changed")
    d = json.loads(raw, object_pairs_hook=_pairs)
    keys = {"schema","scope","typeface","style","sourceGeometrySha256","legacyRasterSha256","legacyLayoutSha256","words"}
    if (set(d) != keys or type(d["schema"]) is not int or d["schema"] != 1
            or d["scope"] != "Three fixed bonus car-value graphics only; no font software, reusable alphabet or original pixels"
            or d["typeface"] != "Haettenschweiler" or d["style"] != [24,.88,.5]
            or d["sourceGeometrySha256"] != GEOMETRY_SHA or d["legacyRasterSha256"] != BASE_SHA
            or d["legacyLayoutSha256"] != LAYOUT_SHA or type(d["words"]) is not list or len(d["words"]) != 3):
        raise ValueError("Bonus fixed-word schema/style changed")
    geometry = geometry_path.read_bytes()
    if hashlib.sha256(geometry).hexdigest() != GEOMETRY_SHA:
        raise ValueError("Bonus source-cell geometry changed")
    text = geometry.decode().split("nfs_front_cells[]={",1)[1].split("};",1)[0]
    cells = [tuple(map(int,x)) for x in re.findall(r"\{(\d+),(\d+),(\d+),(\d+),(\d+)\}",text)]
    small = {code:(i+1,w) for i,(_,_,w,code,large) in enumerate(cells) if not large}
    result = []
    for word, name, context, atlas in zip(d["words"],NAMES,CONTEXTS,ATLAS):
        if (set(word) != {"text","context","sourceCells","atlas","sourceCrop","rgbaSha256","alphaRuns"}
                or word["text"] != name or word["context"] != context or word["atlas"] != list(atlas)
                or word["sourceCrop"] != [0,10,atlas[2],atlas[3]]
                or word["sourceCells"] != [list(small[ord(c)]) for c in name]
                or sum(x[1] for x in word["sourceCells"]) * 4 != atlas[2]):
            raise ValueError("Bonus word/source-cell identity changed")
        runs = word["alphaRuns"]
        count = atlas[2]*atlas[3]
        if type(runs) is not list or not runs or len(runs)%2 or len(runs)>count*2:
            raise ValueError("Bonus alpha run inventory changed")
        alpha = bytearray()
        for n,v in zip(runs[::2],runs[1::2]):
            if type(n) is not int or type(v) is not int or not 1<=n<=65535 or not 0<=v<=255 or n>count-len(alpha):
                raise ValueError("Bonus alpha run overflow/value")
            alpha.extend(bytes((v,))*n)
        if len(alpha)!=count or not any(alpha):
            raise ValueError("Bonus alpha image length/ink changed")
        rgba = bytearray(b"\xff"*(count*4));rgba[3::4]=alpha
        if hashlib.sha256(rgba).hexdigest()!=word["rgbaSha256"]:
            raise ValueError("Bonus authored image digest changed")
        result.append((word,bytes(rgba)))
    return result


def strip_layout(layout):
    if BEGIN not in layout and END not in layout:
        return layout
    if layout.count(BEGIN)!=1 or layout.count(END)!=1:
        raise ValueError("Ambiguous bonus layout extension")
    a,b=layout.index(BEGIN),layout.index(END)
    if b<=a or layout[b+len(END):]!="\n};\n":
        raise ValueError("Bonus layout boundary changed")
    return layout[:a]+layout[b+len(END):]


def _rows(images):
    rows=[]
    for word,_ in images:
        ax,ay,aw,ah=word["atlas"];width=aw/4
        for shadow in (0,1):
            edge=0
            for cell,advance in word["sourceCells"]:
                values=[cell,365+shadow+edge,84+shadow,advance,51,edge/width,(edge+advance)/width,
                        ax/4,ay/8,width,365+shadow,2.5,ah/4]
                rows.append("{"+",".join(format(v,".12g") for v in values)
                            +",UINT64_C(0x"+word["context"]+"),1,1}")
                edge+=advance
            if edge!=width:
                raise ValueError("Incomplete bonus source partition")
    if len(rows)!=54:
        raise ValueError("Unexpected bonus source row count")
    return rows


def apply(pixels, layout, directory, geometry_path):
    if (len(pixels)!=BASE_BYTES or hashlib.sha256(pixels).hexdigest()!=BASE_SHA
            or hashlib.sha256(layout.encode()).hexdigest()!=LAYOUT_SHA
            or BEGIN in layout or not layout.endswith("\n};\n")):
        raise ValueError("Legacy atlas/layout differs; explicit review required")
    images=load(directory,geometry_path)
    # Every old accepted UV endpoint is below row2046; preserve the entire
    # old2048-row image anyway, plus its clamp-edge row in the new guard band.
    for first,_ in re.findall(r"\{([^{}]+),UINT64_C\(0x[0-9a-f]+\)([^{}]*)\}",layout):
        g=list(map(float,first.split(",")))
        if not all(math.isfinite(v) for v in g) or g[8]*8+g[12]*4>2045.000001:
            raise ValueError("Legacy sampling footprint changed")
    result=bytearray(pixels)+bytearray(RGBA_BYTES-BASE_BYTES)
    result[BASE_BYTES:BASE_BYTES+WIDTH*4]=pixels[-WIDTH*4:]
    for word,rgba in images:
        ax,ay,w,h=word["atlas"]
        # Two-pixel side/bottom guards; one inner top guard and the immutable
        # legacy clamp-edge copy are outside every newly sampled footprint.
        for y in range(-1,h+2):
            sy=min(h-1,max(0,y))
            for x in range(-2,w+2):
                sx=min(w-1,max(0,x));at=((ay+y)*WIDTH+ax+x)*4
                result[at:at+4]=rgba[(sy*w+sx)*4:(sy*w+sx+1)*4]
    if result[:BASE_BYTES]!=pixels:
        raise ValueError("Legacy atlas bytes changed")
    rows=_rows(images)
    table=layout[:-4]+BEGIN+",\n".join(rows)+END+"\n};\n"
    if strip_layout(table)!=layout:
        raise ValueError("Legacy integer rows changed")
    metadata=dict(sourceSha256=SOURCE_SHA,legacyRasterSha256=BASE_SHA,legacyLayoutSha256=LAYOUT_SHA,
                  sourceGeometrySha256=GEOMETRY_SHA,dimensions=[WIDTH,HEIGHT],legacyDimensions=[WIDTH,BASE_HEIGHT],
                  addedRGBABytes=RGBA_BYTES-BASE_BYTES,newRows=len(rows),
                  words=[{k:v for k,v in w.items() if k!="alphaRuns"} for w,_ in images],
                  oldPixelsAndRowsUnchanged=True,style=[24,.88,.5],noOriginalPixels=True)
    return result,table,metadata


def verify(pixels, table, metadata, directory, geometry_path):
    legacy=strip_layout(table)
    expected,layout,proof=apply(pixels[:BASE_BYTES],legacy,directory,geometry_path)
    if bytes(expected)!=bytes(pixels) or layout!=table or proof!=metadata:
        raise ValueError("Bonus pixels/layout/provenance changed")
    return legacy
