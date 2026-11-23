# SPDX-License-Identifier: MIT
from pathlib import Path
import struct,zlib,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from extend_intro_artwork import extend
photo=b'\xff'*(4*3*4);words=b'abcdef';metadata=struct.pack('<II4f',4,3,1,2,3,4)+struct.pack('<II',1,1)*6
payload=metadata+photo+words;original=b'NFSINTRO'+struct.pack('<II',1,len(payload))+zlib.compress(payload)
wide=b'\xff'*(16*9*4);result=extend(original,wide,16,9);decoded=zlib.decompress(result[16:])
assert result[:8]==b'NFSINTRO' and struct.unpack_from('<I',result,8)[0]==2
assert decoded[:72]==metadata and decoded[80:80+len(photo)+len(words)]==photo+words and decoded[-len(wide):]==wide
for args in [(original+b'junk',wide,16,9),(original[:-1],wide,16,9),(original,wide[:-1],16,9),(original,wide,4,3),(original,wide,2**32,9),(original,wide[:-1]+b'\x00',16,9),(result,wide,16,9)]:
 try:extend(*args)
 except ValueError:pass
 else:raise AssertionError('Malformed pack accepted')
print('PASS wide intro packing: original planes exact, version2 layout, malformed/trailing/truncated/nonopaque rejection')
