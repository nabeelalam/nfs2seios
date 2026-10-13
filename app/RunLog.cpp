// SPDX-License-Identifier: MIT
#include "RunLog.h"
#include <cstdio>
#include <mutex>
namespace nfs {
namespace {std::mutex log_mutex;std::FILE *log_file=nullptr;}
void open_app_log(const std::string &path){
    std::lock_guard<std::mutex> lock(log_mutex);
    if(log_file)std::fclose(log_file);
    log_file=path.empty()?nullptr:std::fopen(path.c_str(),"w");
    if(!path.empty() && !log_file)std::fprintf(stderr,"Could not save native app log; continuing with console output\n");
}
void app_log(const char *kind,const std::string &text){
    std::lock_guard<std::mutex> lock(log_mutex);
    std::fprintf(stderr,"%s: %s\n",kind,text.c_str());
    if(log_file){std::fprintf(log_file,"%s: %s\n",kind,text.c_str());std::fflush(log_file);}
}
void close_app_log(){std::lock_guard<std::mutex> lock(log_mutex);if(log_file)std::fclose(log_file);log_file=nullptr;}
}
