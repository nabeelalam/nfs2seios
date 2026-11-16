// SPDX-License-Identifier: MIT
#pragma once
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace nfs_hd {
// Used by screen-owned providers. An independent token can travel through
// the renderer queue, so its generation remains valid through the last draw.
class ScopedArtworkResidency {
    using Load=int (*)(const char*,char*,size_t);
    using Clear=void (*)();
    struct Visit {
        ScopedArtworkResidency* owner=nullptr;
        ~Visit(){
            if(owner)owner->release();
        }
    };

    std::mutex mutex;
    std::string path;
    size_t visits=0;
    bool unavailable=false;
    const char* label;
    Load load;
    Clear clear;

    void release(){
        std::lock_guard<std::mutex> lock(mutex);
        if(--visits==0)clear();
    }

public:
    ScopedArtworkResidency(const char* name,Load open,Clear close)
        : label(name),load(open),clear(close){}

    void configure(std::string resource){
        std::lock_guard<std::mutex> lock(mutex);
        if(path.empty())path=std::move(resource);
    }

    std::shared_ptr<void> acquire() noexcept{
        try{
            auto visit=std::make_shared<Visit>();
            std::lock_guard<std::mutex> lock(mutex);
            if(path.empty()||unavailable)return {};
            if(!visits){
                char error[128]{};
                if(!load(path.c_str(),error,sizeof error)){
                    unavailable=true;
                    std::fprintf(stderr,"Optional %s artwork: %s\n",label,error);
                    return {};
                }
            }
            ++visits;
            visit->owner=this;
            return visit;
        }catch(...){return {};}
    }
};
}
