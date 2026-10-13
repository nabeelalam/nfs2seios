#include "MenuInput.h"
#include <vector>
#include <cstdio>
#include <stdexcept>
struct Sink {bool ready=true;std::vector<nfs::GameMessage> messages;};
static bool send(void *context,const nfs::GameMessage &message){auto &sink=*static_cast<Sink *>(context);if(!sink.ready)return false;sink.messages.push_back(message);return true;}
static void check(bool ok){if(!ok)throw std::runtime_error("controller menu message check failed");}
int main(){try{
    nfs::MenuInput input;Sink sink;sink.ready=false;
    input.update(nfs::menu_up|nfs::menu_confirm,0,send,&sink);check(sink.messages.empty());sink.ready=true;
    input.update(nfs::menu_up|nfs::menu_confirm,10,send,&sink);
    check(sink.messages.size()==3 && sink.messages[0].wparam==0x26 && sink.messages[0].lparam==0x01480001 && sink.messages[1].message==0x100 && sink.messages[1].wparam==13 && sink.messages[2].message==0x102);
    input.update(nfs::menu_up|nfs::menu_confirm,409,send,&sink);check(sink.messages.size()==3);
    input.update(nfs::menu_up|nfs::menu_confirm,410,send,&sink);check(sink.messages.size()==4 && sink.messages.back().lparam==0x41480001);
    input.update(nfs::menu_up|nfs::menu_confirm,1000,send,&sink);check(sink.messages.size()==5); // No burst of accumulated repeat messages.
    input.update(0,1001,send,&sink);check(sink.messages.size()==7 && sink.messages[5].message==0x101 && sink.messages[5].lparam==0xc1480001 && sink.messages[6].wparam==13 && sink.messages[6].lparam==0xc01c0001);
    input.update(0,2000,send,&sink);check(sink.messages.size()==7);
    input.update(nfs::menu_cancel,2001,send,&sink);check(sink.messages.size()==9 && sink.messages[7].wparam==27 && sink.messages[8].message==0x102);
    input.update(nfs::menu_cancel,5000,send,&sink);check(sink.messages.size()==9);
    sink.ready=false;input.update(0,5001,send,&sink);check(sink.messages.size()==9);sink.ready=true;
    input.update(0,5002,send,&sink);check(sink.messages.size()==10 && sink.messages.back().message==0x101);
    std::puts("PASS: controller menu down/character/up, directional repeat without confirm/cancel repeat, failed-window retry and neutral release");
}catch(const std::exception &error){std::fprintf(stderr,"FAIL: %s\n",error.what());return 1;}return 0;}
