// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
namespace nfs {
struct GameJoystickState {
    int32_t axes[8];uint32_t pov[4];uint8_t buttons[32];
    GameJoystickState();
};
static_assert(sizeof(GameJoystickState)==80,"Original DIJOYSTATE ABI");
enum class InputMethod : uint32_t {
    query,addref,release,create_device,enumerate,capabilities,acquire,unacquire,
    state,data,format,event,cooperative,object_info,device_info,property,
    poll,effect,feedback,enum_objects,get_property,control,initialize,
    enum_effects,effect_info,feedback_state,enum_created,escape,send_data,status,root_initialize,count
};
constexpr uint32_t input_function_id(InputMethod method){return 0xfe100000u+4*uint32_t(method);}
uint32_t input_argument_count(InputMethod);
using InputEnumeration=int32_t(*)(void *,uint32_t,uint32_t,uint32_t);
class InputDevices {
    struct Object;
    std::mutex mutex;
    std::unordered_map<uint32_t,std::shared_ptr<Object>> objects;
    std::array<GameJoystickState,2> snapshots;
    InputEnumeration callback;
    std::shared_ptr<Object> find(uint32_t);
    uint32_t make(bool device,uint32_t kind,uint32_t slot);
public:
    explicit InputDevices(InputEnumeration enumerate):callback(enumerate){}
    ~InputDevices();
    int32_t create(uint32_t version,uint32_t output,uint32_t outer);
    int32_t invoke(void *context,InputMethod,const uint32_t *arguments);
    // Publish on connect/disconnect and each input update. A null state resets
    // every axis/button so an unplugged controller cannot leave throttle held.
    void publish(uint32_t slot,const GameJoystickState *state);
};
void bind_input_devices(InputDevices *);
InputDevices &input_devices();
// Own on the SDL/app main thread. Pump after SDL input events and before game
// input consumption; destroy before SDL_Quit and before InputDevices retirement.
class SDLGamepadInput {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    explicit SDLGamepadInput(InputDevices &);
    ~SDLGamepadInput();
    void update(bool active=true);
    uint8_t menu_buttons()const;
};
}
extern "C" int32_t DirectInputCreateA_wrap(int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t SDL_NumJoysticks_wrap();
extern "C" bool nfs_register_input_functions();
