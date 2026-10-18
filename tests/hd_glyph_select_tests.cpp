// SPDX-License-Identifier: MIT
#include "HdGlyphSelect.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
#define check(v) do{if(!(v)){std::fprintf(stderr,"FAIL glyph selector line %d\n",__LINE__);std::abort();}}while(0)
static void gold_checks(){
 const float bounds[10][4]={{43.330078125f,47.314453125f,72.71484375f,70.72265625f},{72.216796875f,47.314453125f,100.60546875f,70.72265625f},{101.103515625f,47.314453125f,130.48828125f,70.72265625f},{129.990234375f,47.314453125f,159.375f,70.72265625f},{159.873046875f,47.314453125f,189.2578125f,70.72265625f},{188.759765625f,47.314453125f,218.14453125f,70.72265625f},{217.646484375f,47.314453125f,246.03515625f,70.72265625f},{73.212890625f,71.220703125f,101.6015625f,94.62890625f},{102.099609375f,71.220703125f,131.484375f,94.62890625f},{130.986328125f,71.220703125f,160.37109375f,94.62890625f}};unsigned count=0;
 for(unsigned d=0;d<10;++d){const float *b=bounds[d];
   const float corners[4][2]={{b[0],b[1]},{b[2],b[1]},{b[0],b[3]},{b[2],b[3]}};
   for(unsigned a=0;a<4;++a)for(unsigned j=0;j<4;++j)for(unsigned c=0;c<4;++c){
     float uv[6]={corners[a][0],corners[a][1],corners[j][0],corners[j][1],corners[c][0],corners[c][1]};
     const int expected=(a!=j && j!=c && a!=c)?int(d+2):0;
     if(nfs_hd_digit_triangle(uv)!=expected)std::abort();++count;
   }
   float uv[6]={b[0],b[1],b[2],b[1],b[2],b[3]};
   for(unsigned i=0;i<6;++i){const float saved=uv[i];
     for(float change:{.1f,-.1f}){uv[i]=saved+change;if(nfs_hd_digit_triangle(uv))std::abort();++count;}
     for(float invalid:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){
       uv[i]=invalid;if(nfs_hd_digit_triangle(uv))std::abort();++count;
     }
     uv[i]=saved;
   }
 }
 std::printf("PASS: %u complete/reordered/degenerate/clipped/nonfinite gold selection cases\n",count);
}

int main(){
    gold_checks();
    for(unsigned digit=0;digit<10;++digit){
        const float shrink=255.0f/256.0f;
        float left=(48.5f+12*digit)*shrink,right=(60.0f+12*digit)*shrink;
        float top=21.5f*shrink,bottom=47*shrink;
        float first[]={left,top,right,top,left,bottom};
        float second[]={right,top,right,bottom,left,bottom};
        check(nfs_hd_digit_triangle(first));check(nfs_hd_digit_triangle(second));
        first[4]=right;first[5]=top;check(!nfs_hd_digit_triangle(first));
        first[4]=left;first[5]=47;check(!nfs_hd_digit_triangle(first));
        first[5]=bottom;first[0]=NAN;check(!nfs_hd_digit_triangle(first));
        second[0]=left+1;check(!nfs_hd_digit_triangle(second));
    }
    float constant[]={55,30,55,30,55,30};check(!nfs_hd_digit_triangle(constant));
    float cross[]={49,22,63,22,49,46};check(!nfs_hd_digit_triangle(cross));
    float clipped[]={49,22,59,22,49,30};check(!nfs_hd_digit_triangle(clipped));
    float outside[]={20,22,30,22,20,46};check(!nfs_hd_digit_triangle(outside));
    // Exact observed original digit 9, which the old integer-cell test rejected.
    float observed9[]={155.888672f,21.416016f,167.34375f,21.416016f,155.888672f,46.816406f};
    check(nfs_hd_digit_triangle(observed9));
    std::puts("PASS: ten biased original numeral cells, both triangle halves, captured digit9, constant UV, band boundary, cross-cell, clipped and NaN rejection");
}
