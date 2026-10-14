"""Compile lifted calls and verify architectural EBP across C boundaries."""
from pathlib import Path
import shutil
import importlib.util
import re
import struct
import subprocess
import tempfile

from tools.recomp import config
from tools.recomp.translator import FunctionTranslator

BASE = 0x10000
DATA = 0x22334455
PRELUDE = r"""
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint32_t g_eax,g_ebp,g_seh_ebp,g_esp;
#define eax g_eax
#define esp g_esp
static uint8_t memory[65536];
#define MEM32(a) (*(uint32_t*)(memory+(a)))
#define PUSH32(s,v) do{(s)-=4;MEM32(s)=(v);}while(0)
#define POP32(s,v) do{(v)=MEM32(s);(s)+=4;}while(0)
#define RECOMP_ABI_CALL(va,fn) (fn)()
#define RECOMP_ICALL_SAFE_AT(va,s,site) sub_00010040()
#define RECOMP_ICALL_SAFE_AT_CC(va,s,site) sub_00010040()
"""


def _call(at, target):
    return b'\xe8' + struct.pack('<i', target - (at + 5))


def _functions(case):
    target = BASE + 0x40
    funcs = {target: bytes.fromhex('558bec8b45005dc3')}
    if case == 'tail':
        # Actual allocator thunk: push ebp; mov ebp,esp; pop ebp; jmp.
        funcs[BASE] = bytes.fromhex('558bec5de9') + struct.pack('<i', target - (BASE + 9))
    elif case in ('direct', 'indirect'):
        # Frameless routine uses EBP as a saved general register.
        head = b'\x55\xbd' + struct.pack('<I', DATA)
        call = (_call(BASE + len(head), target) if case == 'direct' else
                b'\xb8' + struct.pack('<I', target) + bytes.fromhex('ffd0'))
        funcs[BASE] = head + call + bytes.fromhex('5dc3')
    elif case == 'clobber':
        funcs[target] = b'\xbd' + struct.pack('<I', DATA) + b'\xc3'
        funcs[BASE] = b'\x55' + _call(BASE + 1, target) + bytes.fromhex('8bc55dc3')
    elif case == 'siblings':
        reader = BASE + 0x60
        funcs[reader] = bytes.fromhex('8bc5c3')  # mov eax,ebp; ret
        # No local EBP: framed callee then caller-register reader.
        funcs[BASE] = _call(BASE, target) + _call(BASE + 5, reader) + b'\xc3'
    return funcs


def _translate(funcs):
    image = bytearray(b'\x90' * 0x80)
    for at, body in funcs.items():
        image[at - BASE:at - BASE + len(body)] = body
    config._install([config.Section('.text', BASE, len(image), 0, len(image), True)],
                    entry_point=BASE, kernel_thunk_addr=BASE, origin='ebp-boundary-runtime')
    db = {at: {'start': hex(at), 'end': at + len(body), '_addr': at, 'size': len(body)}
          for at, body in funcs.items()}
    translator = FunctionTranslator(bytes(image), db)
    return '\n'.join(translator.translate_function(at, db[at]) for at in funcs)


def _legacy_patched(code):
    path=Path(__file__).resolve().parents[3]/'street1/translate-ebp-boundaries.py'
    spec=importlib.util.spec_from_file_location('ebp_postpass',path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    # Reconstruct the old missing-boundary forms without changing instructions.
    code=re.sub(r'g_ebp = ebp; g_seh_ebp = ebp; (?=esp \+=)', '',code)
    code=code.replace('g_ebp = ebp; g_seh_ebp = ebp; ', 'g_seh_ebp = ebp; ')
    code=re.sub(r'^    ebp = g_ebp; /\* read back architectural EBP \*/\n', '',code,flags=re.M)
    code=re.sub(r'^    g_ebp = ebp; /\* frame stays current across calls \*/\n    g_seh_ebp = ebp;\n', '',code,flags=re.M)
    code=re.sub(r'^    g_ebp = ebp; g_seh_ebp = ebp;\n', '',code,flags=re.M)
    result,_=module.patch_source(code)
    again,counts=module.patch_source(result)
    assert again==result,counts
    return result


def _check_boundaries(transform=lambda source:source):
    cc = shutil.which('clang-cl') or shutil.which('cl')
    if not cc:
        cc = 'C:/Program Files/LLVM/bin/clang-cl.exe'
    failures = []
    for case in ('tail', 'direct', 'indirect', 'siblings', 'clobber'):
        funcs = _functions(case)
        prototypes = '\n'.join(f'void sub_{at:08X}(void);' for at in funcs)
        want = DATA if case in ('direct', 'indirect', 'clobber') else 0x1234
        main = f'''
int main(void) {{
    g_ebp=g_seh_ebp=0x1234; g_esp=0x8000; MEM32(g_esp)=0xf00d;
    sub_00010000();
    if(g_eax!={want}u||g_ebp!=0x1234||g_seh_ebp!=0x1234||g_esp!=0x8004) {{
        fprintf(stderr,"{case}: eax=%08x ebp=%08x seh=%08x esp=%08x\\n",
            g_eax,g_ebp,g_seh_ebp,g_esp);return 1;
    }}
    return 0;
}}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'boundary.c'
            exe = Path(tmp) / 'boundary.exe'
            c.write_text(PRELUDE + prototypes + '\n' + transform(_translate(funcs)) + main)
            built = subprocess.run([cc, '/nologo', '/O2', '/w', str(c),
                                    '/Fe:' + str(exe), '/Fo:' + str(Path(tmp) / 'boundary.obj')],
                                   capture_output=True, text=True)
            assert built.returncode == 0, built.stdout + built.stderr
            result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=5)
            if result.returncode:
                failures.append(result.stdout + result.stderr)
    assert not failures, '\n'.join(failures)


def test_ebp_register_crosses_all_function_boundaries():
    _check_boundaries()


def test_postpass_repairs_legacy_boundaries_idempotently():
    _check_boundaries(_legacy_patched)


if __name__ == '__main__':
    test_ebp_register_crosses_all_function_boundaries()
    test_postpass_repairs_legacy_boundaries_idempotently()
    print('PASS ten compiled EBP boundary cases (generator and idempotent post-pass)')
