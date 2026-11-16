// SPDX-License-Identifier: MIT
// Include the loader to exercise its exact bounded inflater independently of
// the intentionally empty production content pin. No authored fixture selected.
#include "../src/hd/ClassPictureArtworkLoader.cpp"
#include <cassert>
#include <string>
static bool refused(const std::vector<unsigned char>&file,size_t bytes){try{(void)inflateExact(file,bytes);return false;}catch(...){return true;}}
int main(){std::vector<unsigned char>plain(32,7),file(16+compressBound(plain.size()),0);uLongf n=file.size()-16;assert(compress2(file.data()+16,&n,plain.data(),plain.size(),9)==Z_OK);file.resize(16+n);assert(inflateExact(file,32)==plain);assert(refused(file,31));assert(refused(file,33));auto bad=file;bad.back()^=128;assert(refused(bad,32));auto trailing=file;trailing.push_back(0);assert(refused(trailing,32));auto cut=file;cut.pop_back();assert(refused(cut,32));assert(refused(file,9u*1024u*1024u+1));char error[128]{};assert(!nfs_class_picture_art_load_file("missing",error,sizeof error));assert(std::string(error).find("pin")!=std::string::npos);}
