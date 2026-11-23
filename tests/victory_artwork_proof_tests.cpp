// SPDX-License-Identifier: MIT
// Explicit private original-resource/reference-plane fixtures; no default CTest registration.
#include "hd/ClassicScreens/VictoryArtworkProof.h"
#include <fstream>
#include <cassert>
#include <iostream>
using namespace nfs_victory_art_proof;
std::vector<unsigned char>read(const std::string&p){std::ifstream f(p,std::ios::binary);assert(f);return std::vector<unsigned char>(std::istreambuf_iterator<char>(f),{});}
std::vector<uint16_t>plane(const std::string&p){auto b=read(p);assert(b.size()==640*480*2);std::vector<uint16_t>out(640*480);for(size_t i=0;i<out.size();++i)out[i]=uint16_t(b[2*i])|uint16_t(b[2*i+1])<<8;return out;}
int main(int argc,char**argv){assert(argc==2);std::string root=argv[1];auto raw=read(root+"/back.source");auto name=reinterpret_cast<const unsigned char*>("back");auto image=Image::create(name,raw);assert(image);auto observed=plane(root+"/back.observed");auto back=Scene::background(image,observed);assert(back&&back->image()==0&&!back->photo());auto changed=observed;changed[0]^=1;assert(!Scene::background(image,changed));assert(!Image::create(nullptr,raw));assert(!Image::create(reinterpret_cast<const unsigned char*>("nope"),raw));raw.back()^=1;assert(!Image::create(name,raw));raw.pop_back();assert(!Image::create(name,raw));assert(!Scene::winner(back,image,observed));assert(!Scene::background({},observed));unsigned photos=0;
for(const auto&s:nfs_victory_original::sources){if(!s.image)continue;auto bytes=read(root+"/"+s.name+".source");auto photo=Image::create(reinterpret_cast<const unsigned char*>(s.name),bytes);assert(photo);auto pixels=plane(root+"/"+s.name+".observed");auto scene=Scene::winner(back,photo,pixels);assert(scene&&scene->image()==s.image&&scene->photo()&&scene->pixels()==pixels);assert(!Scene::winner(scene,photo,pixels));assert(!Scene::winner({},photo,pixels));auto inside=pixels;inside[size_t(s.top)*640+s.left]^=1;assert(!Scene::winner(back,photo,inside));auto outside=pixels;outside[0]^=1;assert(!Scene::winner(back,photo,outside));bytes[16]^=1;assert(!Image::create(reinterpret_cast<const unsigned char*>(s.name),bytes));assert(!Image::create(name,photo->bytes()));assert(back->pixels()==observed);++photos;}
assert(photos==15);std::cout<<"Verified backdrop and all15 winner compositions; malformed sources, altered observed pixels, invalid composition order rejected.\n";
}
