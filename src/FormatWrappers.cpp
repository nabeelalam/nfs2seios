// SPDX-License-Identifier: MIT
#include "FormatWrappers.h"
#include "GuestAccess.h"
#include <cstdio>
#include <string>
#include <stdexcept>
#include <vector>
#include <type_traits>
namespace {
constexpr size_t limit=16384;
struct Arguments {
    uint64_t address;
    template<class T> T take() {
        if (address>UINT32_MAX || sizeof(T)>uint64_t(UINT32_MAX)+1-address)
            throw std::out_of_range("Guest format argument overflow");
        T value=guest_load<T>(uint32_t(address)); address+=sizeof(T); return value;
    }
};
std::string string_at(uint32_t address,size_t maximum,bool require_terminator) {
    std::string value;
    if (!address) return "(null)";
    for (size_t i=0;i<maximum;++i) {
        if (uint64_t(address)+i>UINT32_MAX) throw std::out_of_range("Guest string overflow");
        char ch=char(guest_load<int8_t>(address+uint32_t(i)));
        if (!ch) return value;
        value+=ch;
    }
    if (require_terminator) throw std::out_of_range("Unterminated guest format string");
    return value;
}
template<class T> std::string print_value(const std::string &spec,T value,bool width_star,bool precision_star,int width,int precision) {
    const auto print=[&](char *output,size_t size) {
        if (width_star && precision_star) return std::snprintf(output,size,spec.c_str(),width,precision,value);
        if (width_star) return std::snprintf(output,size,spec.c_str(),width,value);
        if (precision_star) return std::snprintf(output,size,spec.c_str(),precision,value);
        return std::snprintf(output,size,spec.c_str(),value);
    };
    int length=print(nullptr,0);
    if (length<0 || size_t(length)>limit) throw std::out_of_range("Formatted game value too long");
    std::vector<char> bytes(size_t(length)+1);
    if (print(bytes.data(),bytes.size())!=length) throw std::runtime_error("Unstable game format result");
    return std::string(bytes.data(),size_t(length));
}
}
extern "C" int32_t vsprintf_wrap(int32_t output,int32_t format,int32_t arguments) {
    const std::string input=string_at(uint32_t(format),4096,true);
    Arguments args{uint32_t(arguments)};
    std::string result;
    for (size_t i=0;i<input.size();) {
        if (input[i]!='%') { result+=input[i++]; }
        else if (++i<input.size() && input[i]=='%') { result+='%'; ++i; }
        else {
            std::string spec="%";
            while (i<input.size() && std::string("-+ #0").find(input[i])!=std::string::npos) spec+=input[i++];
            bool width_star=false,precision_star=false;
            int width=0,precision=-1;
            const auto digits=[&](int &value) {
                while (i<input.size() && input[i]>='0' && input[i]<='9') {
                    value=value*10+(input[i]-'0'); spec+=input[i++];
                    if (value>int(limit)) throw std::out_of_range("Guest format width/precision too large");
                }
            };
            if (i<input.size() && input[i]=='*') { width_star=true; spec+=input[i++]; width=args.take<int32_t>(); }
            else digits(width);
            if (i<input.size() && input[i]=='.') {
                spec+=input[i++]; precision=0;
                if (i<input.size() && input[i]=='*') { precision_star=true; spec+=input[i++]; precision=args.take<int32_t>(); }
                else digits(precision);
            }
            if (width>int(limit) || width<-int(limit) || precision>int(limit)) throw std::out_of_range("Guest format dynamic width/precision too large");
            std::string length;
            if (i<input.size() && std::string("hlztjL").find(input[i])!=std::string::npos) {
                length+=input[i++];
                if (i<input.size() && (length=="h" || length=="l") && input[i]==length[0]) length+=input[i++];
            }
            spec+=length;
            if (i==input.size()) throw std::invalid_argument("Incomplete game format conversion");
            const char conversion=input[i++]; spec+=conversion;
            const auto emit=[&](auto value) { result+=print_value(spec,value,width_star,precision_star,width,precision); };
            if (conversion=='s') {
                if (!length.empty()) throw std::invalid_argument("Wide guest format strings need an explicit adapter");
                const uint32_t address=args.take<uint32_t>();
                const auto text=string_at(address,precision>=0?size_t(precision):limit,precision<0);
                emit(text.c_str());
            } else if (conversion=='c') {
                if (!length.empty()) throw std::invalid_argument("Wide guest characters need an explicit adapter");
                emit(int(args.take<int32_t>()));
            } else if (conversion=='n') {
                const uint32_t address=args.take<uint32_t>();
                if (length=="hh") guest_store<int8_t>(address,int8_t(result.size()));
                else if (length=="h") guest_store<int16_t>(address,int16_t(result.size()));
                else if (length=="ll" || length=="j") guest_store<int64_t>(address,int64_t(result.size()));
                else if (length.empty() || length=="l" || length=="z" || length=="t") guest_store<int32_t>(address,int32_t(result.size()));
                else throw std::invalid_argument("Unsupported guest count length");
            } else if (std::string("fFeEgGaA").find(conversion)!=std::string::npos) {
                if (!length.empty() && length!="l") throw std::invalid_argument("Unsupported guest floating format length");
                emit(args.take<double>());
            } else if (conversion=='d' || conversion=='i') {
                if (length=="ll") emit(static_cast<long long>(args.take<int64_t>()));
                else if (length=="j") emit(static_cast<intmax_t>(args.take<int64_t>()));
                else if (length=="l") emit(static_cast<long>(args.take<int32_t>()));
                else if (length=="z") emit(static_cast<std::make_signed<size_t>::type>(args.take<int32_t>()));
                else if (length=="t") emit(static_cast<ptrdiff_t>(args.take<int32_t>()));
                else if (length.empty() || length=="h" || length=="hh") emit(static_cast<int>(args.take<int32_t>()));
                else throw std::invalid_argument("Unsupported guest integer format length");
            } else if (std::string("uoxX").find(conversion)!=std::string::npos) {
                if (length=="ll") emit(static_cast<unsigned long long>(args.take<uint64_t>()));
                else if (length=="j") emit(static_cast<uintmax_t>(args.take<uint64_t>()));
                else if (length=="l") emit(static_cast<unsigned long>(args.take<uint32_t>()));
                else if (length=="z") emit(static_cast<size_t>(args.take<uint32_t>()));
                else if (length=="t") emit(static_cast<std::make_unsigned<ptrdiff_t>::type>(args.take<uint32_t>()));
                else if (length.empty() || length=="h" || length=="hh") emit(static_cast<unsigned int>(args.take<uint32_t>()));
                else throw std::invalid_argument("Unsupported guest unsigned format length");
            } else throw std::invalid_argument("Unsupported game format conversion");
        }
        if (result.size()>limit) throw std::out_of_range("Formatted game output too long");
    }
    __builtin_memcpy(nfs_host_address(uint32_t(output),result.size()+1),result.c_str(),result.size()+1);
    return int32_t(result.size());
}
