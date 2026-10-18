// SPDX-License-Identifier: MIT
#include "HdGlyphSelect.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
static void check(bool v){if(!v)std::abort();}
int main(){
    for(unsigned digit=0;digit<10;++digit){
        float left=48.31f+12*digit,right=59.76f+12*digit;
        float first[]={left,21.42f,right,21.42f,left,46.82f};
        float second[]={right,21.42f,right,46.82f,left,46.82f};
        check(nfs_hd_digit_triangle(first));check(nfs_hd_digit_triangle(second));
        first[4]=right;first[5]=21.42f;check(!nfs_hd_digit_triangle(first));
        first[4]=left;first[5]=47;check(!nfs_hd_digit_triangle(first));
        first[5]=46.82f;first[0]=NAN;check(!nfs_hd_digit_triangle(first));
    }
    float constant[]={55,30,55,30,55,30};check(!nfs_hd_digit_triangle(constant));
    float cross[]={49,22,63,22,49,46};check(!nfs_hd_digit_triangle(cross));
    float clipped[]={49,22,59,22,49,30};check(!nfs_hd_digit_triangle(clipped));
    float outside[]={20,22,30,22,20,46};check(!nfs_hd_digit_triangle(outside));
    std::puts("PASS: ten full numeral cells, both triangle halves, constant UV, band boundary, cross-cell, clipped and NaN rejection");
}
