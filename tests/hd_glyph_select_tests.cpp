// SPDX-License-Identifier: MIT
#include "HdGlyphSelect.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#define check(v) do{if(!(v)){std::fprintf(stderr,"FAIL glyph selector line %d\n",__LINE__);std::abort();}}while(0)
int main(){
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
