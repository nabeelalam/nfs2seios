"""Batch bounded arithmetic in the reviewed, pinned generated C++ dialect.

This is a positive whitelist, not a general C++ parser. Unknown syntax stays
unbatched. Calls, control flow and native declarations end the current scope.
"""
import re
TYPES={'8i':'int8_t','16i':'int16_t','32i':'int32_t','64i':'int64_t','32f':'float','64f':'double'}
HELPERS=set('add sub adc sbb inc dec and_ or_ xor_ neg cmp test shl shr sar rol ror shrd imul32 imul16 mul32 idiv32 div32 bsr fld fild fmul fadd fsub fdiv fdivr fsubr fld_st fld1 fldz fxch_st fstp fst fistp fstcw fnstsw fcom fcomp fcompp fcom_st fcomp_st faddp_st fmulp_st fsubp_st fsubrp_st fdivp_st fdivrp_st fstp_st fst_st setnbe sahf lahf'.split())
CALLS=HELPERS|{'guest_load','guest_store','guest_address','static_cast'}|{'to'+suffix for suffix in TYPES}
REGISTERS=r'(?:eax|ebx|ecx|edx|esi|edi|ebp|esp|ax|bx|cx|dx|al|ah|bl|bh|cl|ch|dl|dh)'
PREFIX=re.compile(r'^\s*(?:'+REGISTERS+r'\s*(?:=|\+=|-=|\^=|&=|\|=|<<=|>>=)|(?:'+ '|'.join(sorted(HELPERS))+r')\s*\(|guest_store<|\{ const auto mapped_address = )')
MEMORY=re.compile(r'\b(?:to(?:8i|16i|32i|64i|32f|64f)|guest_load|guest_store)(?:<[^<>]+>)?\s*\(')
def eligible(line):
    code=line.split('//',1)[0].strip()
    if not code or not code.endswith(';') or not PREFIX.match(code):return False
    if any(token in code for token in ('->','[',']','reinterpret_cast','"',"'",'/*','*/')):return False
    if re.search(r'\*\s*(?:\(|[A-Za-z_])',code):return False
    if re.search(r'\)\s*\(',code):return False
    if re.search(r'\bguest_(?:load|store)\s+<',code):return False
    if re.search(r'\b(if|else|for|while|goto|return|throw|new|delete|break|continue)\b',code):return False
    calls=re.findall(r'\b([A-Za-z_]\w*)(?:<[^<>]+>)?\s*\(',code)
    if any(name not in CALLS for name in calls):return False
    # A source line is one statement, or the exact generated operand-local form.
    # This prevents an unrelated C++ declaration/call hiding after a semicolon.
    if code.startswith('{ const auto mapped_address = '):
        operand=re.fullmatch(r'\{ const auto mapped_address = [^;]+; auto mapped_value = guest_load<(int8_t|int16_t|int32_t|int64_t|float|double)>\(mapped_address\); (\w+)\(mapped_value[^;]*\); guest_store<\1>\(mapped_address, mapped_value\); \};',code)
        if not operand or operand.group(2) not in HELPERS:return False
    elif any(token in code[:-1] for token in (';','{','}')):return False
    return True
def rewrite(chunk):
    result=re.sub(r'\bto(8i|16i|32i|64i|32f|64f)\s*\(',lambda m:'guest_transaction.load<'+TYPES[m.group(1)]+'>(',chunk)
    return re.sub(r'\bguest_load<','guest_transaction.load<',result).replace('guest_store<','guest_transaction.store<')
def batch_copies(text):
    result=[];pending=[];manifest=[];method=None;copies=0;skip_next=False;in_comment=False
    def flush():
        nonlocal pending,copies
        if copies>=4:
            indent=re.match(r'\s*',pending[0][1]).group();original=''.join(line for _,line in pending)
            result.append(indent+'{ // checked copy transaction begin\n'+indent+'nfs::GuestCopyTransaction guest_transaction;\n'+rewrite(original)+indent+'} // checked copy transaction end\n')
            manifest.append({'method':method,'startLine':pending[0][0],'endLine':pending[-1][0],'checkedCopies':copies,'lines':len(pending)})
        else:result.extend(line for _,line in pending)
        pending=[];copies=0
    for number,line in enumerate(text.splitlines(keepends=True),1):
        code=line.split('//',1)[0]
        if in_comment or '/*' in code:
            flush();result.append(line)
            if '/*' in code:in_comment='*/' not in code.split('/*',1)[1]
            elif '*/' in code:in_comment=False
            continue
        declaration=re.match(r'Fn\(void\) Game::(\w+)\(\)',line)
        if declaration:flush();method=declaration.group(1)
        control=bool(re.match(r'^\s*(?:if|else|while|for)\b',line))
        if skip_next:
            flush();result.append(line)
            if line.strip() and not line.lstrip().startswith('//'):skip_next=control
            continue
        if not eligible(line):flush();result.append(line);skip_next=control;continue
        count=len(MEMORY.findall(line.split('//',1)[0]))
        if len(pending)>=64 or copies+count>32:flush()
        pending.append((number,line));copies+=count
    flush()
    if manifest:return '#include "GuestCopyTransaction.h"\n'+''.join(result),manifest
    return text,manifest
