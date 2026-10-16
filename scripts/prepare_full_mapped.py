"""Prepare statically translated methods with checked guest-copy boundaries."""
import re
import shutil
from prepare_mapped_engine import prepare_mapped, rewrite_stores, closing, ROOT, TYPES
from batch_guest_copies import batch_copies
from native_controller_hooks import add_native_controller_hooks


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


def prepare_full():
    mapped = prepare_mapped()
    source = ROOT / ".build/engine/Cpp"
    dest = ROOT / ".build/fullmapped"
    dest.mkdir(exist_ok=True)
    for path in mapped.glob("*.h"):
        shutil.copyfile(path, dest / path.name)
    shutil.copyfile(mapped / "MappedMemory.cpp", dest / "MappedMemory.cpp")
    functions = set(re.findall(r"game_id_(\w+) =", (mapped / "FunctionIds.h").read_text()))
    transaction_blocks = 0
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
    print(f"Prepared all 15 method units with checked guest boundaries and {transaction_blocks} bounded arithmetic transactions")
    return dest


if __name__ == "__main__":
    prepare_full()
