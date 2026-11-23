// SPDX-License-Identifier: MIT
#include "../src/hd/PlayerGraphSourceProof.h"
#include <cstdio>
#include <cstdlib>
#include <type_traits>
namespace gs=nfs_player_graph_source;
struct InputCanvas {unsigned bits=0;std::array<uint32_t,14>words{};std::array<uint32_t,4>header{};std::array<uint32_t,384>x{};std::array<uint32_t,160>y{};std::vector<uint8_t>pixels;};
struct Row {std::array<uint32_t,4>args{};uint32_t callback=0,format=0,converter=0,forward=0,palette=0,pattern=0;bool sourceSpanKnown=false,wrappedIndexed32=false;std::array<uint32_t,256>paletteBefore{},paletteAfter{};std::vector<uint8_t>sourceBefore,sourceAfter,before,after;};
struct Draw {std::array<uint8_t,4>name{};std::array<uint32_t,4>args{};std::array<uint32_t,9>tableBefore{},tableAfter{};std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<Row>rows;};
struct Selection {uint32_t selectedCar=0,cachedCarIndex=0,graphEnabled=0;std::array<uint32_t,4>graphValues{};};
struct Photo {InputCanvas before,after;uint32_t rawColor=0,packedColor=0,fillColor=0;bool colorObserved=false,fillObserved=false,consumedObserved=false;std::array<uint32_t,4>fillArgs{};std::array<uint32_t,9>fillTableBefore{},fillTableAfter{};std::vector<Row>fillRows;std::vector<Draw>draws;Selection consumed;};
struct Input {uintptr_t game=0,thread=0;uint64_t epoch=0;uint32_t form=0,formGuest=0,archive=0,car=0;std::array<Photo,2>photos;};
static_assert(!std::is_default_constructible<gs::Packet>::value,"Only the qualified factory can mint a Packet");
static_assert(!std::is_copy_constructible<gs::Packet>::value,"Packets retain immutable owned evidence");
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
int main(){
 std::array<unsigned char,32>sha{};CHECK(nfs_texture_sha256("abc",3,sha.data()));CHECK(sha==gs::digest("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
 for(const auto&id:gs::catalog){CHECK(id.paletteCount==256);CHECK(id.paletteOffset+16+id.paletteCount*4==id.span);CHECK((id.header[0]>>8)==id.paletteOffset);CHECK(id.header[1]==(id.width|(id.height<<16)));}
 Input c;gs::Failure failure=gs::Failure::None;CHECK(!gs::verify(c,&failure));CHECK(failure==gs::Failure::Replay);
 c.game=1;c.thread=2;c.epoch=3;c.form=c.formGuest=0x4d3050;c.archive=1000;CHECK(!gs::verify(c,&failure));CHECK(failure==gs::Failure::Replay);
 c.form=c.formGuest=0x4d3100;c.car=11;CHECK(!gs::verify(c,&failure));
 // No synthetic replacement for the original catalog or positive asset bytes.
 // Positive qualification and source/palette mutations require the private
 // actual observations and are intentionally delegated to the runtime caller.
 std::printf("PASS qualified graph source compile/catalog/negative checks: %u; actual-asset positive pending\n",checks);
}
