"""Translate the six original NV2A status writes to hardware W1C semantics.

This preserves the instruction's address and value; it does not skip the
original ISR/DPC or signal any title event. Reapply after code regeneration.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parent / 'project/src/recomp/gen'
total = 0
for name, expected in [('recomp_0051.c', 4), ('recomp_0052.c', 2)]:
    path = root / name
    original = path.read_text(encoding='utf-8')
    text = original
    pattern = r'MEM32\(((?:esi|edi|ecx|eax) \+ 0x(?:400100|600100))\) = ([^;]+);'
    text, changed = re.subn(pattern, r'xbox_Nv2aAckInterrupt(\1, \2);', text)
    calls = len(re.findall(r'xbox_Nv2aAckInterrupt\((?:esi|edi|ecx|eax) \+ 0x(?:400100|600100),', text))
    if calls != expected:
        raise RuntimeError(f'{name}: expected {expected} original writes, found {calls}')
    declaration = 'extern void xbox_Nv2aAckInterrupt(uint32_t address, uint32_t value);'
    if declaration not in text:
        text = text.replace('#include <math.h>', '#include <math.h>\n' + declaration, 1)
    if text != original:
        path.write_text(text, encoding='utf-8', newline='\n')
    total += changed
print(f'NV2A W1C: six original status writes verified, {total} translated')
