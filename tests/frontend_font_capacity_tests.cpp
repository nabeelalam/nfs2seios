// SPDX-License-Identifier: MIT
#include "FrontendFontCapacity.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
#define CHECK(v) do{if(!(v))std::abort();}while(0)
int main(){
 for(unsigned limit:{0u,1023u,2047u})CHECK(nfs_front_atlas_upload_height(limit,2120)==0);
 for(unsigned limit:{2048u,2100u,2119u})CHECK(nfs_front_atlas_upload_height(limit,2120)==2048);
 for(unsigned limit:{2120u,2304u,4096u})CHECK(nfs_front_atlas_upload_height(limit,2120)==2120);
 for(unsigned full:{0u,2047u,2305u})CHECK(nfs_front_atlas_upload_height(4096,full)==0);
 CHECK(nfs_front_atlas_row_fits(0,255.75f,1,.5f,2048,2120));
 CHECK(!nfs_front_atlas_row_fits(0,255.75f,1,.51f,2048,2120));
 CHECK(nfs_front_atlas_row_fits(.5f,256.25f,78,17,2120,2120));
 CHECK(!nfs_front_atlas_row_fits(.5f,256.25f,78,17,2048,2120));
 CHECK(nfs_front_atlas_row_fits(0,264.5f,256,1,2120,2120));
 CHECK(!nfs_front_atlas_row_fits(0,264.5f,256,1.01f,2120,2120));
 for(unsigned uploaded:{0u,2047u,2119u,2121u})CHECK(!nfs_front_atlas_row_fits(0,0,1,1,uploaded,2120));
 for(unsigned field=0;field<4;++field)for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){
  std::array<float,4>v{{0,0,1,1}};v[field]=bad;CHECK(!nfs_front_atlas_row_fits(v[0],v[1],v[2],v[3],2120,2120));}
 for(unsigned actual:{2048u,2120u})for(unsigned quarter=0;quarter<=2045*4;++quarter){
  float oldTow=quarter/32.0f;float mapped=nfs_front_atlas_tow(oldTow,actual);
  CHECK(std::fabs(mapped*(actual/256.0f)-oldTow*8)<.002f);
  if(actual==2048)CHECK(mapped==oldTow);
 }
 std::puts("PASS capacity boundaries/NaN/Infinity, legacy2048 fallback and old physical-pixel coordinate equivalence at both upload heights");
}
