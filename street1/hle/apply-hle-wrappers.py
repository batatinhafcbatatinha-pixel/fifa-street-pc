"""Route the game's D3D8 library calls through the HLE layer.

For every function listed in hle/hle_functions.json, the generated definition
`void sub_X(void)` is renamed `sub_X_gen` (declared in recomp_funcs.h too), and
project/src/hle_d3d8.c defines `sub_X` itself. Direct calls, the dispatch
table and indirect calls therefore all reach the HLE entry, which may call the
original library body as sub_X_gen. Idempotent; reapply after regeneration.
"""
from pathlib import Path
import json
import re

here = Path(__file__).resolve().parent
gen = here.parent / 'project/src/recomp/gen'
table = json.loads((here / 'hle_functions.json').read_text(encoding='utf-8'))
# DirectSound functions routed through project/src/hle_dsound.c.
ds_path = here / 'dsound_functions.json'
if ds_path.exists():
    table.update(json.loads(ds_path.read_text(encoding='utf-8')))
# D3D8 internals given native versions in project/src/hle_sync.c.
sync_path = here / 'sync_functions.json'
if sync_path.exists():
    table.update(json.loads(sync_path.read_text(encoding='utf-8')))
# Game functions observed by project/src/game_hooks.c (front-end screens).
hooks_path = here / 'game_hooks.json'
if hooks_path.exists():
    table.update(json.loads(hooks_path.read_text(encoding='utf-8')))
addrs = sorted(int(a, 16) for a in table)
address_set = set(addrs)
definition = re.compile(r'^void sub_([0-9A-F]{8})(_gen)?\(void\)(?=[ \t]*(?:\r?$|\{))', re.M)

renamed = 0
found = set()
for path in sorted(gen.glob('recomp_00*.c')):
    original = path.read_text(encoding='utf-8')
    text = original
    def route(match):
        global renamed
        a = int(match[1], 16)
        if a not in address_set:
            return match[0]
        found.add(a)
        if not match[2]:
            renamed += 1
        return f'void sub_{a:08X}_gen(void)'
    text = definition.sub(route, text)
    if text != original:
        path.write_text(text, encoding='utf-8', newline='\n')

missing = [f'{a:08X}' for a in addrs if a not in found]
if missing:
    raise SystemExit(f'HLE: no generated definition for {missing}')

funcs = gen / 'recomp_funcs.h'
original = funcs.read_text(encoding='utf-8')
text = original
for a in addrs:
    decl = f'void sub_{a:08X}_gen(void);'
    if decl not in text:
        text = text.replace(f'void sub_{a:08X}(void);', f'void sub_{a:08X}(void);\n{decl}', 1)
if text != original:
    funcs.write_text(text, encoding='utf-8', newline='\n')
print(f'HLE wrappers: {len(addrs)} D3D functions routed ({renamed} renamed this pass)')
