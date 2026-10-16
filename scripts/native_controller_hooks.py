"""Pinned original frontend/driver transitions for native controller input."""


def add_native_controller_hooks(text, name):
    def replace_one(before, after):
        nonlocal text
        if text.count(before) != 1:
            raise ValueError(f"Original native controller hook no longer matches in {name}")
        text = text.replace(before, after)

    if name == "Methods_01.cpp":
        # The original new-binding prompt owns native button edges until its
        # function returns, including cancellation/worker-stop unwinding.
        before = '\tnfs_trace_guest_call(game_id_sub_403FC0,uint32_t(esp));'
        replace_one(before, before + '\n\tnfs::NativeControlBindingScope nfs_binding_scope;')
        before = 'loc_4043E2:\n\teax = to32i(esp+0x18); //mov\n\tesp -= 4; _sub_477B70(); esp += 4; //call'
        replace_one(before, before + '\n\teax=int32_t(uint32_t(eax)|nfs_controller_binding_buttons(uint32_t(guest_load<int32_t>(esp+0x18))));')
        # Retain native button edges only while the character frontend owns input.
        before = '\tnfs_trace_guest_call(game_id_sub_405960,uint32_t(esp));'
        replace_one(before, before + '\n\tnfs_controller_begin_frontend();')
        before = 'loc_40597D:\n\tesp -= 4; _sub_482030(); esp += 4; //call'
        replace_one(before, '''loc_40597D:
    if(nfs_controller_frontend_cancel()){eax=0x1b;pop32(edx);return;}
    if(nfs_controller_frontend_confirm()){eax=0x0d;pop32(edx);return;}
    {const auto command=nfs_controller_frontend_hat();
        if(command){eax=int32_t(command);pop32(edx);return;}}
\tesp -= 4; _sub_482030(); esp += 4; //call''')
    elif name == "Methods_06.cpp":
        # The original per-player driving packet ends frontend edge retention.
        before = '\tnfs_trace_guest_call(game_id_sub_45BB50,uint32_t(esp));'
        replace_one(before, before + '\n\tnfs_controller_begin_driving();')
        # Capture Pause choices before the original joystick refresh can also
        # expose that same held button to the per-player driving packet.
        before = '\tesp -= 4; _sub_4779D0(); esp += 4; //call'
        replace_one(before, '\tif(guest_load<int32_t>(dword_5222D0))nfs_controller_capture_pause_buttons();\n' + before)
        before = '\tguest_store<int32_t>(dword_512848+eax*4, static_cast<int32_t>(edx)); //mov'
        replace_one(before, '\tif(guest_load<int32_t>(dword_5222D0))edx=int32_t(uint32_t(edx)|nfs_controller_frontend_actions());\n' + before)
        # Original F11 binding is digital index1 in the eight-bit player
        # packet, hence flag0x40. Preserve the original reset implementation.
        before = '\tguest_store<int8_t>(edi-1, static_cast<int8_t>(dl)); //mov'
        replace_one(before, '\tif(ebp==0 && !guest_load<int32_t>(dword_5222D0) && nfs_controller_reset_pressed())dl=int8_t(uint8_t(dl)|0x40u);\n' + before)
    elif name == "Methods_07.cpp":
        # Shared Pause result is used by both single-player and split screen.
        before = '\tnfs_trace_guest_call(game_id_sub_45FA80,uint32_t(esp));'
        replace_one(before, before + '\n\tif(nfs_controller_frontend_cancel()){eax=1;return;}')
    elif name == "Methods_08.cpp":
        before = 'loc_4721BE:\n\tesp -= 4; _sub_473260(); esp += 4; //call'
        replace_one(before, 'loc_4721BE:\n\tif(nfs_controller_pause_pressed())eax=1;\n\tesp -= 4; _sub_473260(); esp += 4; //call')
    else:
        return text
    if '#include "InputDevices.h"' not in text:
        text = '#include "InputDevices.h"\n' + text
    return text
