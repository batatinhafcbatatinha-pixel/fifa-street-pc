"""Cooperative scheduling applies to taken guest backedges only."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", ".."))

from tools.recomp import config
from tools.recomp.translator import FunctionTranslator, _yield_guest_backedges
from tools.recomp.disasm import Instruction
from tools.recomp.lifter import Lifter

BASE = 0x10000


def translate(hex_bytes):
    image = bytes.fromhex(hex_bytes)
    config._install(
        [config.Section('.text', BASE, len(image), 0, len(image), True)],
        entry_point=BASE, kernel_thunk_addr=BASE, origin='guest-yield-test')
    db = {BASE: {'end': BASE + len(image), 'size': len(image)}}
    return FunctionTranslator(image, db).translate_function(BASE, db[BASE])


def test_self_jump_yields():
    code = translate('eb fe')
    assert '{ guest_cpu_yield(); goto loc_00010000; }' in code


def test_counted_backedge_yields_only_when_taken():
    # dec eax / jne dec / ret: the fused Jcc must keep yield inside the if.
    code = translate('48 75 fd c3')
    assert code.count('guest_cpu_yield();') == 1
    branch = next(line for line in code.splitlines() if 'guest_cpu_yield();' in line)
    assert 'if (' in branch and ') { guest_cpu_yield(); goto loc_00010000; }' in branch


def test_loop_backedge_and_forward_branch():
    code = translate('90 e2 fd c3')
    assert code.count('guest_cpu_yield();') == 1
    assert 'ecx' in code and '{ guest_cpu_yield(); goto loc_00010000; }' in code
    assert 'guest_cpu_yield' not in translate('eb 00 c3')
    assert 'guest_cpu_yield' not in translate('85 c0 74 00 c3')


def test_switch_yields_for_local_backward_cases_only():
    statements = ['switch (eax) {',
                  'case 0: goto loc_00010000;',
                  'case 1: goto loc_00010030;',
                  'default: goto loc_00009000;', '}']
    lifted = _yield_guest_backedges(statements, 0x10020, {0x10000, 0x10030})
    assert lifted[1] == 'case 0: { guest_cpu_yield(); goto loc_00010000; }'
    assert lifted[2:] == statements[2:]


def test_cli_sti_use_runtime_and_keep_unsupported_instruction_accounting():
    lifter = Lifter()
    for offset, mnemonic, raw in ((0, 'cli', 'fa'), (1, 'sti', 'fb')):
        code = lifter.lift_instruction(Instruction(BASE + offset, 1, mnemonic, '', raw))
        assert code == [f'guest_cpu_{mnemonic}(); /* {mnemonic}: guest interrupt flag */']
    assert not lifter.unimplemented
    lifter.lift_instruction(Instruction(BASE + 2, 2, 'wbinvd', '', '0f09'))
    assert lifter.unimplemented == {'wbinvd': [BASE + 2]}
    # test eax,eax / cli / sti / jne test / ret: CLI/STI preserve ZF.
    code = translate('85 c0 fa fb 75 fa c3')
    assert 'guest_cpu_cli();' in code and 'guest_cpu_sti();' in code
    assert 'if (_flags /*' not in code


if __name__ == '__main__':
    test_self_jump_yields()
    test_counted_backedge_yields_only_when_taken()
    test_loop_backedge_and_forward_branch()
    test_switch_yields_for_local_backward_cases_only()
    test_cli_sti_use_runtime_and_keep_unsupported_instruction_accounting()
    print('guest CPU backedge checks passed')
