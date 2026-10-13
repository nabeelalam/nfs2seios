// SPDX-License-Identifier: MIT
// Inlined into the statically translated Application class. The original
// 8/16/32-bit instructions mask counts to five bits; a zero count changes no
// operand or flags. Use unsigned wide intermediates to avoid C++ shift UB.
template<typename T1,typename T2> FnInl(void) shr(T1 &value,const T2 raw_count){
    static_assert(sizeof(T1)<=4,"Original shift operand width");
    const unsigned count=uint32_t(raw_count)&31,bits=sizeof(T1)*8;
    if(!count)return;
    const uint64_t original=UNSIGNED(value);
    if(count==1)flags.of=getMSB(value);
    flags.cf=count<=bits?((original>>(count-1))&1):0;
    UNSIGNED_REF(value)=count<bits?(original>>count):0;
    set_ZF_SF_flags(value);
}
template<typename T1,typename T2> FnInl(void) sar(T1 &value,const T2 raw_count){
    static_assert(sizeof(T1)<=4,"Original shift operand width");
    const unsigned count=uint32_t(raw_count)&31,bits=sizeof(T1)*8;
    if(!count)return;
    const uint64_t original=UNSIGNED(value),mask=(UINT64_C(1)<<bits)-1;
    const bool negative=(original>>(bits-1))!=0;
    if(count==1)flags.of=0;
    flags.cf=count>=bits?negative:((original>>(count-1))&1);
    uint64_t result=count>=bits?(negative?mask:0):(original>>count);
    if(negative && count<bits)result|=mask<<(bits-count);
    UNSIGNED_REF(value)=result;
    set_ZF_SF_flags(value);
}
template<typename T1,typename T2> FnInl(void) shl(T1 &value,const T2 raw_count){
    static_assert(sizeof(T1)<=4,"Original shift operand width");
    const unsigned count=uint32_t(raw_count)&31,bits=sizeof(T1)*8;
    if(!count)return;
    const uint64_t original=UNSIGNED(value);
    flags.cf=count<=bits?((original>>(bits-count))&1):0;
    UNSIGNED_REF(value)=count<bits?(original<<count):0;
    if(count==1)flags.of=(flags.cf!=getMSB(value));
    set_ZF_SF_flags(value);
}
template<typename T1,typename T2> FnInl(void) rol(T1 &value,const T2 raw_count){
    static_assert(sizeof(T1)<=4,"Original rotate operand width");
    const unsigned bits=sizeof(T1)*8,count=(uint32_t(raw_count)&31)%bits;
    if(!count)return;
    const uint64_t original=UNSIGNED(value);
    UNSIGNED_REF(value)=(original<<count)|(original>>(bits-count));
} // Preserve the fork's unused rotate-flag behavior.
template<typename T1,typename T2> FnInl(void) ror(T1 &value,const T2 raw_count){
    static_assert(sizeof(T1)<=4,"Original rotate operand width");
    const unsigned bits=sizeof(T1)*8,count=(uint32_t(raw_count)&31)%bits;
    if(!count)return;
    const uint64_t original=UNSIGNED(value);
    UNSIGNED_REF(value)=(original>>count)|(original<<(bits-count));
}
