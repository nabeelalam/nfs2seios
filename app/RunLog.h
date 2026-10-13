// SPDX-License-Identifier: MIT
#pragma once
#include <string>
namespace nfs {
void open_app_log(const std::string &path);
void app_log(const char *kind,const std::string &text);
void close_app_log();
}
