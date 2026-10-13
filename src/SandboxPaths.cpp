// SPDX-License-Identifier: MIT
#include "SandboxPaths.h"
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <stdexcept>
namespace nfs {
namespace fs=std::filesystem;
static std::string fold(std::string text){for(auto &ch:text){if(ch=='\\')ch='/';else if(ch>='A' && ch<='Z')ch=char(ch+32);}return text;}
SandboxPaths::SandboxPaths(const std::string &directory){std::error_code error;root=fs::canonical(directory,error).string();if(error || !fs::is_directory(root))throw std::invalid_argument("Game asset root is not a directory");}
bool SandboxPaths::relative_locked(std::string input,std::string &relative)const{
    if(input.empty())return false;
    input=fold(input);bool absolute=false;
    if(input=="c:/nfsiise" || input.compare(0,11,"c:/nfsiise/")==0){input=input.substr(10);absolute=true;}
    else if(!input.empty() && input[0]=='/'){absolute=true;}
    if(input.find(':')!=std::string::npos)return false;
    std::string path=absolute?input:(cwd.empty()?input:cwd+"/"+input);
    std::vector<std::string> parts;std::string part;
    auto commit=[&]{if(part==".."){if(parts.empty())return false;parts.pop_back();}else if(!part.empty() && part!=".")parts.push_back(part);part.clear();return true;};
    for(unsigned char ch:path){if(ch=='/'){if(!commit())return false;}else{if(ch<32 || ch=='*' || ch=='?')return false;part+=char(ch);}}
    if(!commit())return false;relative.clear();for(const auto &p:parts){if(!relative.empty())relative+='/';relative+=p;}return true;
}
bool SandboxPaths::native_locked(const std::string &relative,std::string &native)const{
    fs::path result(root);std::error_code error;
    for(const auto &part:fs::path(relative)){result/=part;if(fs::is_symlink(fs::symlink_status(result,error)))return false;if(error && error!=std::errc::no_such_file_or_directory)return false;error.clear();}
    native=result.string();return true;
}
bool SandboxPaths::resolve(const std::string &input,std::string &native)const{std::lock_guard<std::mutex> lock(mutex);std::string relative;return relative_locked(input,relative) && native_locked(relative,native);}
bool SandboxPaths::change(const std::string &input){std::lock_guard<std::mutex> lock(mutex);std::string relative,native;std::error_code error;if(!relative_locked(input,relative) || !native_locked(relative,native) || !fs::is_directory(native,error))return false;cwd=relative;return true;}
std::string SandboxPaths::current()const{std::lock_guard<std::mutex> lock(mutex);std::string result="C:\\NFSIISE";if(!cwd.empty())result+='\\';for(char ch:cwd)result+=ch=='/'?'\\':ch;return result;}
bool SandboxPaths::remove_file(const std::string &input){std::string native;if(!resolve(input,native))return false;std::error_code error;return fs::is_regular_file(native,error) && fs::remove(native,error);}
bool SandboxPaths::create_directories(const std::string &input){std::string native;if(!resolve(input,native))return false;std::error_code error;fs::create_directories(native,error);return !error && fs::is_directory(native,error);}
static bool matches(const std::string &pattern,const std::string &name){
    if(pattern=="*.*")return true;size_t p=0,n=0,star=std::string::npos,retry=0;
    while(n<name.size()){if(p<pattern.size() && (pattern[p]=='?' || pattern[p]==name[n])){++p;++n;}else if(p<pattern.size() && pattern[p]=='*'){star=p++;retry=n;}else if(star!=std::string::npos){p=star+1;n=++retry;}else return false;}
    while(p<pattern.size() && pattern[p]=='*')++p;return p==pattern.size();
}
bool SandboxPaths::list(const std::string &spec,std::vector<DirectoryEntry> &entries)const{
    entries.clear();const auto input=fold(spec);const auto slash=input.find_last_of('/');const auto pattern=input.substr(slash==std::string::npos?0:slash+1);
    if(pattern.empty() || pattern.find(':')!=std::string::npos)return false;
    std::string native;const std::string directory=slash==std::string::npos?".":(slash==0?"/":input.substr(0,slash));if(!resolve(directory,native))return false;
    std::error_code error;fs::directory_iterator iterator(native,error);if(error)return false;
    for(const auto &item:iterator){
        auto name=item.path().filename().string();if(name.size()>259 || name.empty() || name[0]=='.' || item.is_symlink(error) || !matches(pattern,fold(name)))continue;
        DirectoryEntry entry;entry.name=name;entry.directory=item.is_directory(error);if(error)return false;
        if(!entry.directory){if(!item.is_regular_file(error))continue;entry.size=item.file_size(error);if(error)return false;}
        const auto time=item.last_write_time(error);if(error)return false;
        const auto system=time-fs::file_time_type::clock::now()+std::chrono::system_clock::now();
        const auto seconds=std::chrono::duration_cast<std::chrono::seconds>(system.time_since_epoch()).count();
        if(seconds>=-11644473600ll)entry.modified=uint64_t(seconds+11644473600ll)*10000000;
        entries.push_back(std::move(entry));if(entries.size()>4096)return false;
    }
    std::sort(entries.begin(),entries.end(),[](const DirectoryEntry &a,const DirectoryEntry &b){return fold(a.name)<fold(b.name);});return true;
}
}
