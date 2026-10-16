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
    uint32_t consumed_buttons[2]{};
    uint32_t frontend_button_edges[2]{};
    bool frontend_capture=false,binding_capture=false;
    uint8_t frontend_direction=0;
    uint64_t frontend_repeat=0;
    struct DirectionPress {uint8_t direction,slot;};
    std::array<DirectionPress,8> frontend_presses{};
    unsigned frontend_press_count=0;
    uint8_t frontend_cancel_pending=0;
    uint8_t frontend_confirm_pending=0;
    bool reset_held=false,reset_consumed=false;
    InputEnumeration callback;
    std::shared_ptr<Object> find(uint32_t);
    uint32_t make(bool device,uint32_t kind,uint32_t slot);
    bool consume_buttons_locked(uint32_t mask);
    void consume_held_locked();
public:
    explicit InputDevices(InputEnumeration enumerate):callback(enumerate){}
    ~InputDevices();
    int32_t create(uint32_t version,uint32_t output,uint32_t outer);
    int32_t invoke(void *context,InputMethod,const uint32_t *arguments);
    // Publish on connect/disconnect and each input update. A null state resets
    // every axis/button so an unplugged controller cannot leave throttle held.
    void publish(uint32_t slot,const GameJoystickState *state);
    void pointer_motion(int32_t dx,int32_t dy,uint32_t now);
    void pointer_click(uint32_t now);
    void clear_pointer(uint32_t now);
    // Call only at the original frontend-to-driving transition, never for
    // ordinary frames. Held menu buttons become available after release.
    void consume_held_buttons();
    void begin_frontend();
    void begin_driving();
    void begin_binding();
    void end_binding();
    uint32_t binding_buttons(uint32_t slot);
    // Original frontend command values, without keyboard messages or state.
    // Retain short press edges between original polls, aggregate both hats,
    // and apply bounded menu navigation repeat while a direction is held.
    uint32_t frontend_hat_command(uint64_t now);
    uint32_t frontend_direction_actions(); // Original UI action bits; no keyboard state.
    // Consume only designated system/menu buttons. Other native bindings and
    // analog inputs remain available; a held button cannot trigger twice.
    bool consume_frontend_buttons(uint32_t mask);
    void capture_frontend_cancel(uint32_t mask);
    bool take_frontend_cancel(uint32_t mask);
    void capture_pause_buttons(uint32_t cancel_mask);
    uint32_t pause_actions();
    // Reserved first-controller View/Back action. The original race query
    // decides when reset is permitted; no keyboard key or second-pad alias.
    void publish_reset(bool pressed);
    bool take_reset();
};
void bind_input_devices(InputDevices *);
InputDevices &input_devices();
// Original recording prompt owns native presses independently of menu actions.
class NativeControlBindingScope {
    InputDevices &devices;
public:
    NativeControlBindingScope():devices(input_devices()){devices.begin_binding();}
    ~NativeControlBindingScope(){devices.end_binding();}
    NativeControlBindingScope(const NativeControlBindingScope &)=delete;
    NativeControlBindingScope &operator=(const NativeControlBindingScope &)=delete;
};
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
extern "C" uint32_t nfs_controller_frontend_hat();
extern "C" bool nfs_controller_frontend_cancel();
extern "C" bool nfs_controller_frontend_confirm();
extern "C" bool nfs_controller_pause_pressed();
extern "C" uint32_t nfs_controller_frontend_actions();
extern "C" void nfs_controller_capture_frontend_cancel();
extern "C" bool nfs_controller_reset_pressed();
extern "C" void nfs_controller_capture_pause_buttons();

extern "C" void nfs_controller_begin_frontend();
extern "C" void nfs_controller_begin_driving();

extern "C" uint32_t nfs_controller_binding_buttons(uint32_t slot);
