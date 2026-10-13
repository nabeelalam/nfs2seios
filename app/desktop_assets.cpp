// SPDX-License-Identifier: MIT
#include "AssetSelection.h"
#include <stdexcept>
namespace nfs {
std::string app_log_path(int argc,char **argv){return argc==2?std::string(argv[1])+"/native-port.log":std::string();}
std::string select_app_assets(int argc,char **argv){
    if(argc!=2)throw std::invalid_argument("Provide the imported original game folder");
    return argv[1];
}
}
