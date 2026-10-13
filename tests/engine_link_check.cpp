#include "EngineMemory.h"
#include "GameContexts.h"
#include "InputDevices.h"
#include "TrackRecords.h"
#include "RuntimeServices.h"
#include "BSS.h"
#include "DATA.h"
#include "FunctionIds.h"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include "Game.h"
#include "MenuInput.h"
#include "QfsCommand.h"
#undef true
#undef false
extern "C" bool register_original_game_functions();
int main(){
    try{
        nfs::EngineMemory memory(32*1024*1024);
        if(!register_original_game_functions() || !nfs_register_input_functions())throw std::runtime_error("Full native function registration failed");
        bool invalid_dispatch_rejected=false;
        try{(void)memory.function(0xc515e9e4u);}catch(const std::out_of_range &error){
            const std::string message=error.what();
            if(message.find("function ID=0xc515e9e4")==std::string::npos)throw std::runtime_error("Invalid dispatch lost its exact function ID");
#if defined(__linux__) && defined(__GLIBC__)
            if(message.find("native dispatch stack:\n")==std::string::npos)throw std::runtime_error("Invalid dispatch lost its native callsite");
#endif
            invalid_dispatch_rejected=true;
        }
        if(!invalid_dispatch_rejected)throw std::runtime_error("Unregistered function ID was accepted");
        const auto font=guest_address(dword_4DF380);
        if(guest_load<uint32_t>(font)!=0x46544e46u || guest_load<uint32_t>(font+4)!=0x86du || guest_load<uint32_t>(font+28)!=0x4e8u)throw std::runtime_error("Original embedded font binary header shifted");
        const auto before=memory.available();
        {std::unique_ptr<void,void(*)(void *)> context(nfs_create_game_context(),nfs_destroy_game_context);
            if(!context || reinterpret_cast<uintptr_t>(context.get())<=UINT32_MAX)throw std::runtime_error("Full native Game context creation failed");
            const auto address=nfs_guest_address(context.get());if(nfs_resolve_game_context(address)!=context.get())throw std::runtime_error("Full native Game context mapping failed");
            auto &registers=*static_cast<Game *>(context.get());
            // Literal command followed by an exact-boundary end marker with
            // zero through three trailing literals: no four-byte lookahead.
            for(unsigned tail=0;tail<4;++tail){
                std::unique_ptr<void,void(*)(void *)> compressed(memory.allocate(16),free32);
                std::unique_ptr<void,void(*)(void *)> decoded(memory.allocate(4+tail),free32);
                if(!compressed || !decoded)throw std::bad_alloc();
                auto bytes=static_cast<uint8_t *>(compressed.get())+16-(11+tail);
                const uint8_t header[]={0x10,0xfb,0,0,uint8_t(4+tail),0xe0,'a','b','c','d',uint8_t(0xfc+tail)};
                std::memcpy(bytes,header,sizeof(header));for(unsigned i=0;i<tail;++i)bytes[11+i]=uint8_t('X'+i);
                const auto stack=registers.esp;
                registers.eax=int32_t(nfs_guest_address(bytes));registers.edx=int32_t(nfs_guest_address(decoded.get()));registers.ebx=1;
                registers.push32(0);registers._sub_4B2824();registers.esp+=4;
                if(registers.esp!=stack || uint32_t(registers.eax)!=4+tail || std::memcmp(decoded.get(),"abcdXYZ",4+tail))throw std::runtime_error("Original QFS exact-boundary literal decode failed");
            }
            {
                std::unique_ptr<void,void(*)(void *)> truncated(memory.allocate(16),free32);
                if(!truncated)throw std::bad_alloc();
                const auto last=nfs_guest_address(truncated.get())+15;
                guest_store<uint8_t>(last,0);bool rejected=false;
                try{(void)nfs_qfs_command(last);}catch(const std::out_of_range &){rejected=true;}
                if(!rejected)throw std::runtime_error("Truncated QFS backreference was accepted");
            }
            for(int count:{0,32,64,-32}){
                registers.flags.setWord(0x8d7);const auto flags=registers.flags.getWord();int32_t value=int32_t(0x81234567u);
                registers.sar(value,count);registers.shr(value,count);registers.shl(value,count);
                if(uint32_t(value)!=0x81234567u || registers.flags.getWord()!=flags)throw std::runtime_error("Zero/masked-zero shift changed operand or flags");
            }
            int32_t signed_value=int32_t(0x80000001u);registers.sar(signed_value,1);
            if(uint32_t(signed_value)!=0xc0000000u || !registers.flags.cf || registers.flags.of)throw std::runtime_error("Arithmetic shift sign/carry failed");
            signed_value=int32_t(0x80000001u);registers.shl(signed_value,1);
            if(signed_value!=2 || !registers.flags.cf || !registers.flags.of)throw std::runtime_error("Left shift carry/overflow failed");
            int8_t byte=int8_t(0x81);registers.sar(byte,8);if(byte!=-1 || !registers.flags.cf)throw std::runtime_error("Narrow arithmetic shift failed");
            byte=int8_t(0x81);registers.shr(byte,9);if(byte!=0)throw std::runtime_error("Narrow logical shift failed");
            byte=int8_t(0x81);registers.rol(byte,9);if(uint8_t(byte)!=3)throw std::runtime_error("Masked narrow rotate failed");
            registers.ror(byte,1);if(uint8_t(byte)!=0x81)throw std::runtime_error("Right rotate failed");
            int16_t word=int16_t(0x8001);registers.shl(word,1);
            if(word!=2 || !registers.flags.cf || !registers.flags.of)throw std::runtime_error("Word shift carry/overflow failed");
            word=int16_t(0x8001);registers.sar(word,16);if(word!=-1 || !registers.flags.cf)throw std::runtime_error("Word arithmetic shift saturation failed");
            word=int16_t(0x8001);registers.rol(word,17);if(uint16_t(word)!=3)throw std::runtime_error("Word rotate masking failed");
            signed_value=1;registers.shl(signed_value,-1);if(uint32_t(signed_value)!=0x80000000u)throw std::runtime_error("Signed count masking failed");
            signed_value=int32_t(0x89abcdefu);registers.shrd(signed_value,int32_t(0x12345678),4);
            if(uint32_t(signed_value)!=0x889abcdeu)throw std::runtime_error("Double shift failed");
            nfs::MenuInput menu;
            const auto key_sink=[](void *pointer,const nfs::GameMessage &message){
                auto &game=*static_cast<Game *>(pointer);const auto saved=game.esp;
                const uint32_t function=message.message==0x100?game_id_sub_4818B0:message.message==0x101?game_id_sub_481860:game_id_sub_4819D0;
                // These are original six-argument internal key handlers reached
                // by the window dispatcher, not four-argument WndProc callbacks.
                game.push32(0);game.push32(message.lparam);game.push32(message.wparam);game.push32(message.message);game.push32(0xb1000001);game.push32(1);game.push32(0);
                nfs_lookup_guest_function(function)(&game);game.esp+=4;
                if(game.esp!=saved)throw std::runtime_error("Original key handler stack imbalance");
                return true;
            };
            menu.update(nfs::menu_up|nfs::menu_confirm,0,key_sink,context.get());
            if(guest_load<uint8_t>(byte_4DB2F4+0x48)!=1 || guest_load<uint8_t>(byte_4DB2F4+0x1c)!=1)throw std::runtime_error("Original controller menu key-down failed");
            menu.update(0,1,key_sink,context.get());
            if(guest_load<uint8_t>(byte_4DB2F4+0x48)!=0 || guest_load<uint8_t>(byte_4DB2F4+0x1c)!=0)throw std::runtime_error("Original controller menu key-up failed");
            std::unique_ptr<void,void(*)(void *)> prefix(memory.allocate(16),[](void *p){if(p)free32(p);});
            std::unique_ptr<void,void(*)(void *)> buffer(memory.allocate(260),[](void *p){if(p)free32(p);});
            if(!prefix || !buffer)throw std::bad_alloc();
            std::strcpy(static_cast<char *>(prefix.get()),"stats/");guest_store<uint32_t>(dword_5134C4,nfs_guest_address(prefix.get()));
            nfs_game_record_path(context.get(),0,nfs_guest_address(buffer.get()));
            if(std::strcmp(static_cast<char *>(buffer.get()),"stats/oval.stf"))throw std::runtime_error("Original binary record path failed");
            nfs_game_record_path(context.get(),20,nfs_guest_address(buffer.get()));
            if(std::strcmp(static_cast<char *>(buffer.get()),"stats/oval.ssf"))throw std::runtime_error("Original text record path failed");
            guest_store<uint32_t>(dword_5134C4,0);
            nfs::RuntimeServices services({nullptr,nullptr,nullptr});nfs::bind_runtime_services(&services);
            bool exited=false;try{nfs_dispatch_game_procedure(context.get(),game_id_ExitProcess1);}catch(const nfs::GameExited &exit){exited=exit.code==1;}
            nfs::bind_runtime_services(nullptr);if(!exited)throw std::runtime_error("Original managed exit dispatch failed");
        }
        if(memory.available()!=before)throw std::runtime_error("Full native Game context leaked heap storage");
        std::puts("PASS: full translated engine linked with production bridges, all 2502 original/31 controller callbacks registered, original SSF/STF path generation and managed exit executed, mapped native Game context above 4 GB allocated and retired; no game startup/assets/gameplay claim");
    }catch(const std::exception &error){std::fprintf(stderr,"FAIL: %s\n",error.what());return 1;}
}
