// SPDX-License-Identifier: MIT
#include "AssetImport.h"
#include <filesystem>
#include <algorithm>
#include <mutex>
#include <random>
#include <stdexcept>
#include <set>
namespace nfs {
namespace fs=std::filesystem;
namespace {
std::mutex import_mutex;
std::string lowercase(std::string name){
    for(auto &c:name)if(c>='A' && c<='Z')c=char(c-'A'+'a');
    return name;
}
bool within(const fs::path &path,const fs::path &parent){
    auto p=path.begin();for(const auto &part:parent){if(p==path.end() || *p!=part)return false;++p;}return true;
}
struct Staging {
    fs::path root;
    ~Staging(){if(!root.empty()){std::error_code ignored;fs::remove_all(root,ignored);}}
};
void copy_file(const fs::path &source,const fs::path &target,AssetImportResult &result){
    if(!fs::is_regular_file(fs::symlink_status(source)))throw std::runtime_error("Game assets must contain regular files and folders only");
    const auto size=fs::file_size(source);
    if(result.files>=100000 || size>UINT64_C(8589934592)-result.bytes)throw std::runtime_error("Game import exceeds its file or storage limit");
    fs::copy_file(source,target,fs::copy_options::none);++result.files;result.bytes+=size;
}
}
AssetImportResult import_assets(const std::string &source_name,const std::string &support_name,const std::string &destination_name){
    std::lock_guard<std::mutex> lock(import_mutex);
    if(fs::is_symlink(fs::symlink_status(source_name)) || fs::is_symlink(fs::symlink_status(support_name)))throw std::runtime_error("Select a real game folder, not a symbolic link");
    const auto source=fs::canonical(source_name),support=fs::canonical(support_name);
    if(!fs::is_directory(source) || !fs::is_directory(support))throw std::runtime_error("Game and support locations must be folders");
    fs::path requested(destination_name);
    const auto leaf=requested.filename();
    if(leaf.empty() || leaf=="." || leaf=="..")throw std::runtime_error("Invalid game destination");
    const auto destination=fs::canonical(requested.parent_path())/leaf;
    if(within(destination,source) || within(source,destination) || within(destination,support) || within(support,destination))throw std::runtime_error("Imported storage must be separate from its source folders");
    if(fs::exists(fs::symlink_status(destination)))throw std::runtime_error("Game data already exists; it will not be replaced");
    Staging staging;
    std::random_device random;
    for(unsigned attempt=0;attempt<20 && staging.root.empty();++attempt){
        const auto candidate=destination.parent_path()/ (".nfs-import-"+std::to_string(random())+"-"+std::to_string(random()));
        if(fs::create_directory(candidate))staging.root=candidate;
    }
    if(staging.root.empty())throw std::runtime_error("Could not create game import staging folder");
    AssetImportResult result;bool game_files=false,front_files=false;
    std::set<std::string> names;
    for(fs::recursive_directory_iterator item(source),end;item!=end;++item){
        const auto relative=item->path().lexically_relative(source);
        const auto top=lowercase(relative.begin()->string());
        if(top!="gamedata" && top!="fedata"){item.disable_recursion_pending();continue;}
        const auto status=item->symlink_status();
        if(!fs::is_directory(status) && !fs::is_regular_file(status))throw std::runtime_error("Symbolic links and special files are not accepted in game assets");
        fs::path normalized;
        for(const auto &part:relative){
            const auto name=lowercase(part.string());
            if(name.find_first_of("\\:*?")!=std::string::npos || std::any_of(name.begin(),name.end(),[](unsigned char c){return c<32;}))throw std::runtime_error("Game asset name cannot be represented by the original file APIs");
            normalized/=name;
        }
        if(!names.insert(normalized.generic_string()).second)throw std::runtime_error("Two game paths collide after conversion to lowercase");
        if(names.size()>100000)throw std::runtime_error("Game import exceeds its entry limit");
        const auto target=staging.root/normalized;
        if(fs::is_directory(status))fs::create_directory(target);
        else {copy_file(item->path(),target,result);if(top=="gamedata")game_files=true;else front_files=true;}
    }
    if(!game_files || !front_files)throw std::runtime_error("Select the folder containing both GAMEDATA and FEDATA with files inside");
    for(const auto name:{"install.win","text.eng","text.ger","text.fre","text.ita","text.spa","text.swe"}){
        const auto file=support/name;
        if(std::string(name)!="install.win" && std::string(name)!="text.eng" && !fs::exists(fs::symlink_status(file)))continue;
        copy_file(file,staging.root/name,result);
    }
    // Both paths are in the caller's app-owned parent. Publish only after every
    // copy and required support file succeeds; failed staging is removed above.
    fs::rename(staging.root,destination);staging.root.clear();return result;
}
}
