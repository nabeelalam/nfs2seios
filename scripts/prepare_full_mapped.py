"""Prepare statically translated methods with checked guest-copy boundaries."""
import re
import shutil
from pathlib import Path
from prepare_mapped_engine import prepare_mapped, rewrite_stores, closing, ROOT, TYPES
from batch_guest_copies import batch_copies
from native_controller_hooks import add_native_controller_hooks
from frontend_word_hooks import add_frontend_word_hooks
from background_scene_hooks import add_background_scene_hooks
from location_caption_hooks import add_location_caption_hooks
from location_scene_hooks import add_location_scene_hooks
from location_preview_hooks import add_location_preview_hooks
from audio_live_hooks import add_audio_live_hooks
from audio_dialog_word_hooks import add_audio_dialog_word_hooks
from graphics_dialog_word_hooks import add_graphics_dialog_word_hooks
from controls_text_hooks import add_controls_text_hooks
from controls_sidebar_hooks import add_controls_sidebar_hooks
from controls_live_hooks import add_controls_live_hooks
from controls_scene_hooks import add_controls_scene_hooks
from controls_setup_text_hooks import add_controls_setup_text_hooks
from controls_setup_sidebar_hooks import add_controls_setup_sidebar_hooks
from controls_setup_live_hooks import add_controls_setup_live_hooks
from controls_setup_scene_hooks import add_controls_setup_scene_hooks
from controls_steering_text_hooks import add_controls_steering_text_hooks
from controls_steering_chart_hooks import add_controls_steering_chart_hooks
from controls_steering_sidebar_hooks import add_controls_steering_sidebar_hooks
from controls_steering_live_hooks import add_controls_steering_live_hooks
from controls_steering_scene_hooks import add_controls_steering_scene_hooks
from audio_sidebar_cache_hooks import add_audio_sidebar_cache_hooks
from audio_scene_hooks import add_audio_scene_hooks
from audio_sidebar_measure_hooks import add_audio_sidebar_measure_hooks
from graphics_live_hooks import add_graphics_live_hooks
from graphics_sidebar_hooks import add_graphics_sidebar_hooks
from graphics_scene_hooks import add_graphics_scene_hooks
from options_scene_hooks import add_options_scene_hooks
from setup_scene_hooks import add_setup_scene_hooks
from player_scene_hooks import add_player_scene_hooks
from batch_rep_words import batch_rep_words
from batch_complete_helpers import batch_complete_helpers


def rewrite_casts(text, functions):
    pattern = re.compile(r"\(int32_t\)\(intptr_t\)\s*")
    for match in reversed(list(pattern.finditer(text))):
        start = match.end()
        if text[start] == "(":
            end = closing(text, start)
            expression = text[start+1:end]
            end += 1
        else:
            symbol = re.match(r"\w+", text[start:])
            if not symbol: raise ValueError(f"Unsupported pointer cast: {text[start:start+60]}")
            expression = symbol.group()
            end = start + len(expression)
            while text[end:end+1] == "[":
                bracket_end = closing(text, end, "[", "]")
                expression += text[end:bracket_end+1]
                end = bracket_end+1
        value = f"game_id_{expression}" if expression in functions else f"guest_address({expression})"
        text = text[:match.start()] + f"static_cast<int32_t>({value})" + text[end:]
    if "(intptr_t)" in text: raise ValueError("Unreviewed intptr_t cast remains")
    return text


def add_audio_operation_guards(text):
    # The original mixer shares scratch storage across voices. Protect complete
    # setup/update/mixing jobs, including callback selection and publication.
    # Insert after copy batching so ownership starts outside every copy scope.
    operations = ("4A7028", "4A734C", "4A7600", "4A7758", "4A77B8",
                  "4A7800", "4A7850", "4A78B0", "4A7A40")
    if "nfs::AudioOperationGuard audio_operation;" in text:
        raise ValueError("Original sound operation guards already applied")
    for operation in operations:
        needle = f'\tnfs_trace_guest_call(game_id_sub_{operation},uint32_t(esp));'
        if text.count(needle) != 1:
            raise ValueError(f"Unexpected sound operation entry: {operation}")
        text = text.replace(needle, needle +
                            '\n\tnfs::AudioOperationGuard audio_operation;')
    return '#include "AudioOperations.h"\n' + text


def prepare_full(bounded_rep_words=False, output_dir=None, checked_copy_bundle=False):
    if (bounded_rep_words or checked_copy_bundle) and output_dir is None:
        raise ValueError("Experimental word-copy generation requires a new output directory")
    dest = ROOT / ".build/fullmapped"
    if output_dir is not None:
        requested = Path(output_dir)
        dest = (requested if requested.is_absolute() else ROOT / requested).resolve()
        try:
            dest.relative_to((ROOT / ".build").resolve())
        except ValueError:
            raise ValueError("Generated output must stay inside the project build directory") from None
        if dest.exists():
            raise FileExistsError("Explicit generated output directory must be new")
    mapped = prepare_mapped()
    source = ROOT / ".build/engine/Cpp"
    dest.mkdir(parents=True, exist_ok=output_dir is None)
    for path in mapped.glob("*.h"):
        shutil.copyfile(path, dest / path.name)
    shutil.copyfile(mapped / "MappedMemory.cpp", dest / "MappedMemory.cpp")
    functions = set(re.findall(r"game_id_(\w+) =", (mapped / "FunctionIds.h").read_text()))
    transaction_blocks = 0
    rep_word_loops = 0
    complete_helpers = 0
    for path in sorted(source.glob("Methods_*.cpp")):
        text = path.read_text().replace('#include "Stubs.h"', '#include "FunctionIds.h"')
        # One platform-specific native buffer dereference in the pinned fork.
        text = text.replace('(uint8_t *)(intptr_t)eax',
                            'static_cast<uint8_t *>(nfs_host_address(uint32_t(eax), uint32_t(esi)))')
        construction = "Game *new_game_ctx = (Game *)malloc32(sizeof(Game));\n\tnew (new_game_ctx) Game();"
        if path.name == "Methods_11.cpp":
            if text.count(construction) != 1: raise ValueError("Unexpected game worker context construction")
            text = text.replace(construction, "Game *new_game_ctx = nfs_create_game_context();")
            text = '#include "GameContexts.h"\n' + text
        text = rewrite_casts(text, functions)
        if path.name == "Methods_14.cpp":
            start = text.index('Fn(void) Game::_sub_4B2824()')
            end = closing(text, text.index('{', start), '{', '}')
            body = text[start:end+1]
            replacements = body.count('edx = to32i(ebx); //mov') + body.count('edx = to32i(esi); //mov')
            if replacements != 6:
                raise ValueError(f"Unexpected QFS command lookahead sites: {replacements}")
            body = body.replace('edx = to32i(ebx); //mov', 'edx = nfs_qfs_command(uint32_t(ebx)); // bounded command')
            body = body.replace('edx = to32i(esi); //mov', 'edx = nfs_qfs_command(uint32_t(esi)); // bounded command')
            text = '#include "QfsCommand.h"\n' + text[:start] + body + text[end+1:]
        if path.name=="Methods_08.cpp":
            start=text.index('Fn(void) Game::_sub_4760F0()')
            end=closing(text,text.index('{',start),'{','}')
            body=text[start:end+1]
            needle='esp -= 4; _sub_486F50(); esp += 4; //call'
            if body.count(needle)!=1:raise ValueError("Required sound bank load guard no longer applies")
            body=body.replace(needle,needle+'\n\tif(!eax)nfs_required_game_file_failed(uint32_t(esp));')
            text=text[:start]+body+text[end+1:]
            text='#include "GameAssets.h"\n'+text
        if path.name=="Methods_02.cpp":
            start=text.index('Fn(void) Game::_sub_41B730()')
            end=closing(text,text.index('{',start),'{','}')
            body=text[start:end+1]
            needle='esp -= 4; _sub_486F40(); esp += 4; //call'
            if body.count(needle)!=1:raise ValueError("Required language resource load guard no longer applies")
            # Preserve the original filename and report a failed required load
            # before its callers dereference a null language resource.
            body=body.replace(needle,needle+'\n\tif(!eax)nfs_required_game_file_failed(uint32_t(esp)+8);')
            text=text[:start]+body+text[end+1:]
            text='#include "GameAssets.h"\n'+text
        if path.name=="Methods_07.cpp":
            start=text.index('Fn(void) Game::_sub_4642F0()')
            end=closing(text,text.index('{',start),'{','}')
            body=text[start:end+1]
            needle='esp -= 4; _sub_486F40(); esp += 4; //call'
            if body.count(needle)!=1:raise ValueError("Required install configuration load guard no longer applies")
            body=body.replace(needle,needle+'\n\tif(!eax)nfs_required_game_file_failed(guest_address(aInstall_win_0));')
            text=text[:start]+body+text[end+1:]
            text='#include "GameAssets.h"\n'+text
        # Two float-valued platform arguments bypassed the original to32f
        # helpers and directly dereferenced the logical game stack. Never treat
        # that 32-bit address as a native pointer, including unaligned slots.
        expected_float_reads={"Methods_03.cpp":1,"Methods_12.cpp":1}.get(path.name,0)
        text,float_reads=re.subn(r'\*\(float \*\)\(esp \+ (\d+)\)',
            lambda m:f'guest_load<float>(uint32_t(esp)+{m.group(1)})',text)
        if float_reads!=expected_float_reads:raise ValueError(f"Unexpected raw float stack arguments in {path.name}")
        text = re.sub(r"push32\((\w+)\)",
                      lambda m: f"push32(static_cast<int32_t>(game_id_{m.group(1)}))" if m.group(1) in functions else m.group(), text)
        # One xchg register/memory instruction: evaluate the address once and
        # exchange through a typed local before committing the memory operand.
        def exchange(match):
            register, suffix, address = match.groups()
            typename = TYPES[suffix]
            return (f"{{ const auto mapped_address = {address}; auto mapped_value = guest_load<{typename}>(mapped_address); "
                    f"swap({register}, mapped_value); guest_store<{typename}>(mapped_address, mapped_value); }}")
        text = re.sub(r"swap\((\w+), to(32i|8i)\(([^()]+)\)\);", exchange, text)
        text = rewrite_stores(text)
        if path.name=="Methods_07.cpp":
            # Only the original active frontend close transition consumes
            # held buttons; ordinary driving frames must never do so.
            needle='loc_46F907:\n\tpush32(edi);\n\txor_(edi, edi);\n\tguest_store<int32_t>(dword_5222D0, static_cast<int32_t>(edi)); //mov'
            if text.count(needle)!=1:raise ValueError("Original frontend close transition no longer matches")
            text=text.replace(needle,needle.replace('\n\tguest_store','\n\tnfs::input_devices().consume_held_buttons();\n\tguest_store'))
            text='#include "InputDevices.h"\n'+text
        def trace(match):
            method=match.group(2);name=method[1:]
            if name not in functions:raise ValueError(f"Unknown trace method ID: {method}")
            return match.group(1)+f'\n\tnfs_trace_guest_call(game_id_{name},uint32_t(esp));'
        text=re.sub(r'(Fn\(void\) Game::(\w+)\(\)\s*\{)',trace,text)
        text=add_native_controller_hooks(text,path.name)
        if path.name == "Methods_14.cpp":
            # Hold the same voice lock before reading its decoder callback.
            # A callee-entry guard alone permits callback selection to race
            # with an initializer that publishes a different codec layout.
            for voice_register in ("esi", "edi"):
                needle = f'\tcall(to32i({voice_register}+0xDA8));'
                if text.count(needle) != 1:
                    raise ValueError("Unexpected original codec dispatch site")
                text = text.replace(needle,
                    '\t{\n\t\tnfs::CodecOperationGuard codec_selection{uint32_t(eax)};\n\t' +
                    needle + '\n\t}')
            text = '#include "CodecOperations.h"\n' + text
        if path.name == "Methods_15.cpp":
            # Shared voice state must not be reinitialized midway through a
            # decoding job. Lock the complete operation, including parameter
            # setup and buffered phases, rather than individual sample stores.
            operations = ("4BD458", "4BD66C", "4BD848", "4BDC7C",
                          "4BDE94", "4C2040", "4BD24C", "4BD01C",
                          "4BD4B0", "4BD6C4", "4BD8EC", "4BDAB8",
                          "4BDCD0", "4C1FB0")
            for operation in operations:
                needle = f'\tnfs_trace_guest_call(game_id_sub_{operation},uint32_t(esp));'
                if text.count(needle) != 1:
                    raise ValueError(f"Unexpected codec operation entry: {operation}")
                text = text.replace(needle, needle +
                    '\n\tnfs::CodecOperationGuard codec_operation{uint32_t(eax)};')
            text = '#include "CodecOperations.h"\n' + text
        text='#include "GameTrace.h"\n'+text
        text, blocks = batch_copies(text)
        transaction_blocks += len(blocks)
        if bounded_rep_words or checked_copy_bundle:
            text, rep_blocks = batch_rep_words(text, path.name)
            rep_word_loops += sum(block["loops"] for block in rep_blocks)
        if checked_copy_bundle:
            text, helper_blocks = batch_complete_helpers(text, path.name)
            complete_helpers += len(helper_blocks)
        if path.name == "Methods_14.cpp":
            text = add_audio_operation_guards(text)
        text = add_frontend_word_hooks(text, path.name)
        text = add_background_scene_hooks(text, path.name)
        text = add_location_caption_hooks(text, path.name)
        text = add_location_scene_hooks(text, path.name)
        text = add_location_preview_hooks(text, path.name)
        text = add_audio_live_hooks(text, path.name)
        text = add_audio_sidebar_cache_hooks(text, path.name)
        text = add_audio_scene_hooks(text, path.name)
        text = add_audio_sidebar_measure_hooks(text, path.name)
        text = add_graphics_live_hooks(text, path.name)
        text = add_graphics_sidebar_hooks(text, path.name)
        text = add_graphics_scene_hooks(text, path.name)
        # Methods_01 owns overlapping menu lifetimes. Preserve their tested
        # nesting while other units keep their established callback order.
        if path.name == "Methods_01.cpp":
            text = add_player_scene_hooks(text, path.name)
        text = add_options_scene_hooks(text, path.name)
        text = add_setup_scene_hooks(text, path.name)
        if path.name != "Methods_01.cpp":
            text = add_player_scene_hooks(text, path.name)
        text = add_audio_dialog_word_hooks(text, path.name)
        text = add_graphics_dialog_word_hooks(text, path.name)
        text = add_controls_text_hooks(text, path.name)
        text = add_controls_sidebar_hooks(text, path.name)
        text = add_controls_live_hooks(text, path.name)
        text = add_controls_scene_hooks(text, path.name)
        # Child source owners must exist before the native scene can join them.
        text = add_controls_setup_text_hooks(text, path.name)
        text = add_controls_setup_sidebar_hooks(text, path.name)
        text = add_controls_setup_live_hooks(text, path.name)
        text = add_controls_setup_scene_hooks(text, path.name)
        text = add_controls_steering_text_hooks(text, path.name)
        text = add_controls_steering_chart_hooks(text, path.name)
        text = add_controls_steering_sidebar_hooks(text, path.name)
        text = add_controls_steering_live_hooks(text, path.name)
        text = add_controls_steering_scene_hooks(text, path.name)
        (dest / path.name).write_text(text)
    stubs = (source / "Stubs.cpp").read_text()
    dispatch = '#include "Game.h"\n#include "FunctionIds.h"\nextern "C" bool nfs_register_guest_function(uint32_t, NativeGameFunction);\n'
    registrations = []
    for name, method in re.findall(r"Fn\(void\) (\w+)\(Game \*self\) \{self->(\w+)\(\);\}", stubs):
        dispatch += f'static void mapped_{name}(void *self) {{ static_cast<Game *>(self)->{method}(); }}\n'
        registrations.append(f' if (!nfs_register_guest_function(game_id_{name}, mapped_{name})) return false;\n')
    if len(registrations) != len(functions): raise ValueError("Incomplete native function dispatch table")
    dispatch += 'extern "C" bool register_original_game_functions() {\n' + ''.join(registrations) + 'return true;\n}\n'
    (dest / "FullDispatch.cpp").write_text(dispatch)
    if (bounded_rep_words or checked_copy_bundle) and rep_word_loops != 12:
        raise ValueError("Incomplete reviewed bounded-word-copy method set")
    if checked_copy_bundle and complete_helpers != 2:
        raise ValueError("Incomplete reviewed complete-helper method set")
    print(f"Prepared all 15 method units with checked guest boundaries and {transaction_blocks} bounded arithmetic transactions")
    if bounded_rep_words or checked_copy_bundle:
        print(f"Enabled {rep_word_loops} reviewed bounded word-copy loops")
    if checked_copy_bundle:
        print(f"Enabled {complete_helpers} pinned complete-helper copy scopes")
    return dest


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bounded-rep-words", action="store_true",
                        help="enable the reviewed experimental word-copy scopes")
    parser.add_argument("--checked-copy-bundle", action="store_true",
                        help="enable the reviewed word loops and two pinned complete-helper scopes")
    parser.add_argument("--output-dir", type=Path,
                        help="new output directory inside .build (required for experimental scopes)")
    arguments = parser.parse_args()
    prepare_full(bounded_rep_words=arguments.bounded_rep_words,
                 output_dir=arguments.output_dir,
                 checked_copy_bundle=arguments.checked_copy_bundle)
