#include "AssetImport.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace fs=std::filesystem;
static void require(bool pass,const char *message){if(!pass)throw std::runtime_error(message);}
static void write(const fs::path &path,const std::string &text){fs::create_directories(path.parent_path());std::ofstream out(path,std::ios::binary);out<<text;if(!out)throw std::runtime_error("fixture write failed");}
static std::string read(const fs::path &path){std::ifstream in(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(in),{});}
template<class F> static void fails(F action){bool failed=false;try{action();}catch(const std::exception &){failed=true;}require(failed,"invalid import unexpectedly succeeded");}
static void no_staging(const fs::path &root){for(const auto &entry:fs::directory_iterator(root))require(entry.path().filename().string().find(".nfs-import-")!=0,"failed import left staging behind");}
int main(int argc,char **argv){
    if(argc!=2 && argc!=3)return 2;
    try{
        const auto root=fs::absolute(fs::path(argv[1])/"asset-import-tests");
        fs::remove_all(root);fs::create_directories(root);
        const auto source=root/"selected",support=root/"support",destination=root/"game";
        write(source/"GAMEDATA"/"AUDIO"/"Fixture.BIN",std::string("a\0b",3));
        write(source/"FEDATA"/"PC"/"Art.DAT","front");
        write(source/"NFS2SE.EXE","never copied");write(source/"unrelated"/"secret.txt","never copied");
        write(support/"install.win","4nnenglish\r\n");write(support/"text.eng","language fixture");
        auto result=nfs::import_assets(source.string(),support.string(),destination.string());
        require(result.files==4 && result.bytes==3+5+12+16,"import accounting changed");
        require(read(destination/"gamedata"/"audio"/"fixture.bin")==std::string("a\0b",3),"binary asset changed");
        require(read(destination/"install.win")==read(support/"install.win") && read(destination/"text.eng")==read(support/"text.eng"),"support files changed");
        require(!fs::exists(destination/"nfs2se.exe") && !fs::exists(destination/"unrelated"),"import included unrelated files");
        fails([&]{nfs::import_assets(source.string(),support.string(),destination.string());});
        require(read(destination/"fedata"/"pc"/"art.dat")=="front","existing game was replaced");
        fs::remove(support/"text.eng");
        fails([&]{nfs::import_assets(source.string(),support.string(),(root/"missing-support").string());});
        require(!fs::exists(root/"missing-support"),"partial import published");no_staging(root);
        write(support/"text.eng","language fixture");
        const auto no_support=root/"executable-only-support";fs::create_directories(no_support);
        fails([&]{nfs::import_assets(source.string(),no_support.string(),(root/"missing-selected-support").string());});
        require(!fs::exists(root/"missing-selected-support"),"missing startup files published an import");no_staging(root);
        write(source/"install.win","selected configuration");write(source/"text.eng","selected language");
        write(source/"text.fre","selected optional language");
        const auto selected_support=root/"selected-support-result";
        nfs::import_assets(source.string(),no_support.string(),selected_support.string());
        require(read(selected_support/"install.win")=="selected configuration" && read(selected_support/"text.eng")=="selected language" && read(selected_support/"text.fre")=="selected optional language","selected startup files changed");
        const auto mixed=root/"mixed-support";write(mixed/"install.win","partial bundled configuration");
        fails([&]{nfs::import_assets(source.string(),mixed.string(),(root/"mixed-support-result").string());});
        require(!fs::exists(root/"mixed-support-result"),"mixed support sets published an import");no_staging(root);
        fails([&]{nfs::import_assets(source.string(),support.string(),(source/"nested-game").string());});
        const auto empty=root/"empty";fs::create_directories(empty/"GAMEDATA");write(empty/"FEDATA"/"item","front");
        fails([&]{nfs::import_assets(empty.string(),support.string(),(root/"empty-result").string());});no_staging(root);
        // Case collisions and symlinks are tested on case-sensitive Linux/macOS.
        const auto collision=root/"collision";write(collision/"GAMEDATA"/"ONE","one");write(collision/"gamedata"/"TWO","two");write(collision/"FEDATA"/"item","front");
        if(!fs::equivalent(collision/"GAMEDATA",collision/"gamedata")){
            fails([&]{nfs::import_assets(collision.string(),support.string(),(root/"collision-result").string());});no_staging(root);
        }
        std::error_code error;fs::create_symlink(source/"GAMEDATA"/"AUDIO"/"Fixture.BIN",source/"FEDATA"/"escape",error);
        if(!error){fails([&]{nfs::import_assets(source.string(),support.string(),(root/"symlink-result").string());});no_staging(root);}
        if(argc==3){
            fs::remove(source/"FEDATA"/"escape");
            const auto engine=root/"engine-fixture";
            nfs::import_assets(source.string(),argv[2],engine.string());
            require(read(engine/"install.win")==read(fs::path(argv[2])/"install.win") && read(engine/"text.eng")==read(fs::path(argv[2])/"text.eng"),"pinned support copy changed");
        }
        std::cout<<"PASS: owned asset import, exact binary/support copies, lowercase paths, no executable, rollback, no replacement, separate roots, missing trees, case collisions and available symlink rejection\n";
    }catch(const std::exception &error){std::cerr<<error.what()<<'\n';return 1;}return 0;
}
