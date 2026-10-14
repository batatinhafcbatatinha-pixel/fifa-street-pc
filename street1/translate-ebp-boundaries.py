"""Durably repair EBP register bridges in existing generated FIFA Street C.

The generator now emits the same semantics; this scoped post-pass preserves
all existing instruction fixes and passive trace injections. Default is a
read-only report. --apply writes only changed recomp_NNNN.c units. It never
builds, relinks, starts, or changes memory in a game process.
"""
import argparse
from pathlib import Path
import re

FUNCTION = re.compile(r'(void sub_[0-9A-Fa-f]+\(void\)\n\{)(.*?)(\n\})', re.S)
BRIDGE = 'g_ebp = ebp; g_seh_ebp = ebp; '
CALL_MARKER = BRIDGE + '/* EBP boundary call */'
READBACK = 'ebp = g_ebp; /* read back architectural EBP */'


def patch_source(source):
    counts = {'functions': 0, 'tails': 0, 'calls': 0, 'returns': 0, 'entries': 0}

    def function(match):
        original = body = match[2]
        if 'uint32_t ebp' not in body:
            return match[0]
        # FPO is an instruction layout, not a different EBP register bank.
        body, n = re.subn(r'ebp = g_seh_ebp; /\* fpo_leaf: inherit caller\x27s frame \*/',
                         "ebp = g_ebp; /* fpo_leaf: inherit architectural EBP */", body)
        counts['entries'] += n
        # External conditional/unconditional tails and SEH frame publication.
        body, n = re.subn(r'(?<!g_ebp = ebp; )g_seh_ebp = ebp;(?= )',
                         BRIDGE.rstrip(), body)
        counts['tails'] += n
        # A normal return publishes EBP after POP/LEAVE restored it.
        body, n = re.subn(r'(?<!g_seh_ebp = ebp; )esp \+= (\d+); return; /\* ret',
                         lambda m: BRIDGE + 'esp += ' + m[1] + '; return; /* ret', body)
        counts['returns'] += n
        lines = body.splitlines(keepends=True)
        output = []
        for i, line in enumerate(lines):
            call = ('RECOMP_ABI_CALL(' in line or 'RECOMP_ICALL_SAFE' in line or
                    'setjmp(*recomp_setjmp_slot(' in line)
            if call:
                indent = line[:len(line) - len(line.lstrip())]
                previous = lines[i - 1].strip() if i else ''
                if previous != CALL_MARKER:
                    output.append(indent + CALL_MARKER + '\n')
                    counts['calls'] += 1
                output.append(line)
                following = lines[i + 1].strip() if i + 1 < len(lines) else ''
                if following != READBACK:
                    output.append(indent + READBACK + '\n')
            else:
                output.append(line)
        body = ''.join(output)
        if body != original:
            counts['functions'] += 1
        return match[1] + body + match[3]

    return FUNCTION.sub(function, source), counts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--directory', type=Path,
                        default=Path(__file__).parent / 'project/src/recomp/gen')
    args = parser.parse_args()
    units = changed = 0
    total = {}
    for path in sorted(args.directory.glob('recomp_[0-9][0-9][0-9][0-9].c')):
        units += 1
        with path.open('r', encoding='utf-8', newline='') as stream:
            source = stream.read()
        # Existing generated units may contain CRLF; preserve their style.
        newline = '\r\n' if '\r\n' in source else '\n'
        normalized = source.replace('\r\n', '\n')
        result, counts = patch_source(normalized)
        for key, value in counts.items():
            total[key] = total.get(key, 0) + value
        if result != normalized:
            changed += 1
            print(path.name + ': ' + ', '.join(f'{k}={v}' for k, v in counts.items()))
            if args.apply:
                with path.open('w', encoding='utf-8', newline='') as stream:
                    stream.write(result.replace('\n', newline))
    print(f'{"applied" if args.apply else "read-only"}: units={units}, changed={changed}, {total}')


if __name__ == '__main__':
    main()
