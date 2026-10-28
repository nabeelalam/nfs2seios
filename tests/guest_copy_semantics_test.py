from pathlib import Path
import argparse,subprocess,sys,shutil,os
parser=argparse.ArgumentParser(description='Compare exact original and batched integer/FPU state')
parser.add_argument('--thin-lto',action='store_true',help='Also exercise the bounded ThinLTO compiler/linker path')
args=parser.parse_args()
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from batch_guest_copies import batch_copies as transform
root=Path(__file__).resolve().parents[1]
# Bind the mixed semantic translation to the exact engine selected by CI.
# No fallback to a stale legacy preparation when an explicit choice is missing.
requested_engine=Path(os.environ.get('NFS_CI_ENGINE_SOURCE','.build/fullmapped'))
engine=(requested_engine if requested_engine.is_absolute() else root/requested_engine).resolve()
try:engine.relative_to((root/'.build').resolve())
except ValueError:raise ValueError('Selected CI engine must be inside the project build directory') from None
if not engine.is_dir() or not (engine/'Game.h').is_file():
    raise FileNotFoundError('Selected CI engine has not been prepared (Game.h missing)')

(root/'.build').mkdir(exist_ok=True)
index=1
while (root/f'.build/copy-semantics-{index:03}').exists():index+=1
out=root/f'.build/copy-semantics-{index:03}';out.mkdir()
body=r'''Fn(void) Game::_sub_FFFF()
{
    eax = to32i(esi);
    edx = to16i(esi+4);
    add(eax,edx);
    adc(eax,to32i(esi+8));
    guest_store<int32_t>(edi,eax);
    fld(to32f(esi+16));
    fmul(to32f(esi+20));
    fadd(to64f(esi+24));
    { const auto mapped_address = edi+8; auto mapped_value = guest_load<double>(mapped_address); fstp(mapped_value); guest_store<double>(mapped_address, mapped_value); };
    fld1();
    { const auto mapped_address = edi+4; auto mapped_value = guest_load<int32_t>(mapped_address); fistp(mapped_value); guest_store<int32_t>(mapped_address, mapped_value); };
    test(eax,(int32_t)1);
    if (jz())
        goto loc_even;
    eax = to32i(esi+32);
    ebx = to32i(esi+36);
    xor_(eax,ebx);
    guest_store<int32_t>(edi+16,eax);
    edx = to32i(esi+40);
loc_even:
    eax = to8i(esi+12);
    ebx = to16i(esi+6);
    ecx = to32i(esi+44);
    edx = to64i(esi+48);
    guest_store<int16_t>(edi+18,bx);
    fld1();
    fldz();
    fcompp();
    { const auto mapped_address = edi+20; auto mapped_value = guest_load<int16_t>(mapped_address); fnstsw(mapped_value); guest_store<int16_t>(mapped_address, mapped_value); };
    { const auto mapped_address = edi+24; auto mapped_value = guest_load<int32_t>(mapped_address); bsr(mapped_value,to32i(esi+40)); guest_store<int32_t>(mapped_address, mapped_value); };
}
'''
after,blocks=transform(body);assert len(blocks)>=3
source=r'''
#include "Application.h"
#include "EngineMemory.h"
#include "GuestCopyTransaction.h"
#include "DATA.h"
#include "BSS.h"
#include <array>
#include <cstring>
#include <cstdio>
#include <stdexcept>
BssLayout _bss alignas(4){};DataLayout _data alignas(4){};
struct Before:Application{void operation();};struct After:Application{void operation();};
'''+body.replace('Game::_sub_FFFF()','Before::operation()')+'\n'+after.replace('Game::_sub_FFFF()','After::operation()')+r'''
struct State {
    std::array<int32_t,11> registers;std::array<unsigned char,128> memory;std::array<unsigned char,64> fpu;
    bool operator==(const State &other)const{return registers==other.registers && memory==other.memory && fpu==other.fpu;}
};
int main(){
    nfs::EngineMemory memory(8*1024*1024);Before before;After after;auto *bytes=static_cast<unsigned char *>(memory.allocate(128));uint32_t base=memory.encode(bytes),random=0x785f2913;size_t cases=0;
    auto next=[&]{random^=random<<13;random^=random>>17;random^=random<<5;return random;};
    for(unsigned sourceOffset:{0u,1u})for(unsigned destinationOffset:{0u,4u,17u,64u})for(unsigned n=0;n<5000;++n){
        std::array<unsigned char,128> initial;for(auto &v:initial)v=static_cast<unsigned char>(next());
        float x=float(int(next()%2001)-1000)/16.f,y=float(int(next()%2001)-1000)/32.f;double z=double(int(next()%2001)-1000)/64.;
        std::memcpy(initial.data()+sourceOffset+16,&x,4);std::memcpy(initial.data()+sourceOffset+20,&y,4);std::memcpy(initial.data()+sourceOffset+24,&z,8);
        auto run=[&](auto &game){
            std::memcpy(bytes,initial.data(),128);game.eax=0;game.ebx=0;game.ecx=0;game.edx=0;game.esi=int32_t(base+sourceOffset);game.edi=int32_t(base+destinationOffset);game.flags.setWord(0x202);game.fpu.init();for(auto &v:game.fpu.regs)v=0;
            game.operation();State state;state.registers={game.eax,game.ebx,game.ecx,game.edx,game.esi,game.edi,int32_t(game.flags.getWord()),int32_t(game.fpu.control.word),int32_t(game.fpu.status.flags),int32_t(game.fpu.status.top),0};
            std::memcpy(state.memory.data(),bytes,128);std::memcpy(state.fpu.data(),game.fpu.regs,64);return state;
        };
        if(!(run(before)==run(after)))throw std::runtime_error("Mixed integer/FPU/control-flow state mismatch");++cases;
    }
    memory.release(bytes);std::printf("PASS: %zu mixed integer/FPU/branch cases with overlapping unaligned operands and exact CPU/FPU/memory state\n",cases);
}
'''
path=out/'mixed_semantics.cpp';assert not path.exists();path.write_text(source)
command=[shutil.which('clang++') or shutil.which('c++') or 'clang++','-std=c++14','-O3','-g','-pthread','-fwrapv','-fno-strict-aliasing','-Isrc','-I'+str(engine),str(path),'src/EngineMemory.cpp','src/GameTrace.cpp','-o',str(out/'mixed-semantics')]
if args.thin_lto:
    if not shutil.which('clang++'):raise RuntimeError('ThinLTO validation requires clang++')
    backend='-Wl,-mllvm,-threads=2' if sys.platform=='darwin' else '-Wl,-plugin-opt,jobs=2'
    command[1:1]=['-flto=thin',backend]
with (out/'mixed-build.log').open('w') as log:subprocess.run(command,check=True,cwd=root,stdout=log,stderr=subprocess.STDOUT,timeout=60)
with (out/'mixed-test.log').open('w') as log:subprocess.run([str(out/'mixed-semantics')],check=True,cwd=root,stdout=log,stderr=subprocess.STDOUT,timeout=30)
print((out/'mixed-test.log').read_text())
