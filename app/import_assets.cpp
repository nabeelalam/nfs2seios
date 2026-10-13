// SPDX-License-Identifier: MIT
#include "AssetImport.h"
#include <exception>
#include <iostream>
int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "Usage: nfs_import_assets SOURCE_FOLDER SUPPORT_FOLDER NEW_DESTINATION\n";
        return 2;
    }
    try {
        const auto result = nfs::import_assets(argv[1], argv[2], argv[3]);
        std::cout << "Imported " << result.files << " files (" << result.bytes
                  << " bytes) into " << argv[3] << '\n';
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Game import failed: " << error.what() << '\n';
        return 1;
    }
}
