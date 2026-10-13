// SPDX-License-Identifier: MIT
#include "InputDevices.h"
#include "GuestAccess.h"
#include "Game.h"
#include <stdexcept>
extern "C" bool nfs_register_guest_function(uint32_t,NativeGameFunction);
namespace {
template<nfs::InputMethod method> void dispatch(void *context){
    auto &game=*static_cast<Game *>(context);const auto count=nfs::input_argument_count(method);
    const uint32_t stack=uint32_t(game.esp);
    if(uint64_t(stack)+(count+1)*4>uint64_t(UINT32_MAX)+1)throw std::out_of_range("Input call stack overflow");
    nfs_host_address(stack,(count+1)*4);uint32_t arguments[5]{};
    for(uint32_t i=0;i<count;++i)arguments[i]=guest_load<uint32_t>(stack+(i+1)*4);
    game.eax=nfs::input_devices().invoke(context,method,arguments);
    game.esp+=int32_t(count*4); // Original stdcall consumes args; caller owns return slot.
}
}
extern "C" bool nfs_register_input_functions(){
#define ADD(method) if(!nfs_register_guest_function(nfs::input_function_id(nfs::InputMethod::method),dispatch<nfs::InputMethod::method>))return false;
    ADD(query) ADD(addref) ADD(release) ADD(create_device) ADD(enumerate) ADD(capabilities)
    ADD(acquire) ADD(unacquire) ADD(state) ADD(data) ADD(format) ADD(event) ADD(cooperative)
    ADD(object_info) ADD(device_info) ADD(property) ADD(poll) ADD(effect) ADD(feedback)
    ADD(enum_objects) ADD(get_property) ADD(control) ADD(initialize) ADD(enum_effects)
    ADD(effect_info) ADD(feedback_state) ADD(enum_created) ADD(escape) ADD(send_data)
    ADD(status) ADD(root_initialize)
#undef ADD
    return true;
}
