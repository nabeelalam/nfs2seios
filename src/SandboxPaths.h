// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <stdint.h>
namespace nfs {
struct DirectoryEntry {std::string name;uint64_t size=0,modified=0;bool directory=false;};
class SandboxPaths {
    std::string root,cwd;
    mutable std::mutex mutex;
    bool relative_locked(std::string input,std::string &relative)const;
    bool native_locked(const std::string &relative,std::string &native)const;
public:
    explicit SandboxPaths(const std::string &directory);
    bool resolve(const std::string &input,std::string &native)const;
    bool change(const std::string &input);
    std::string current()const;
    bool remove_file(const std::string &input);
    bool create_directories(const std::string &input);
    bool list(const std::string &pattern,std::vector<DirectoryEntry> &entries)const;
};
}
