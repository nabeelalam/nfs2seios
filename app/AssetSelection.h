// SPDX-License-Identifier: MIT
#pragma once
#include <string>
namespace nfs {std::string select_app_assets(int argc,char **argv);}
namespace nfs {std::string app_log_path(int argc,char **argv);}

#ifdef NFS_CLASSIC_BUNDLED_ART
namespace nfs {std::string app_artwork_template_path();}
#endif
