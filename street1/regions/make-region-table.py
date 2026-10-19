"""Build street1/regions/<region>.json: how the USA addresses used by the native
sources move in another release of the same game.

Offline maintainer tool. It needs both executables and never copies game bytes
into its output: only addresses (numbers) and the target executable's hash.

  python street1/regions/make-region-table.py --us us/default.xbe \
      --target eu/default.xbe --us-functions .local/source/street1/project/src/recomp/gen/recomp_funcs.h \
      --name europe --out street1/regions/europe.json \
      --seeds-out street1/regions/function-seeds-europe.json \
      --set-out street1/regions/function-set-europe.json

A function is mapped when its bytes are identical in both executables apart
from absolute pointers and call/jump displacements (the relink moved code and
data, the instructions did not change). Addresses inside a mapped function move
with it; library sections (D3D, XGRPH, DSOUND, XPP, DOLBY) move as a block and
are checked byte for byte the same way. Data addresses are matched by their
surrounding bytes.
"""
import argparse
import bisect
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from relocate import literals  # noqa: E402

LO, HI = 0x10000, 0x320000
LIBS = ('D3D', 'XGRPH', 'DSOUND', 'XPP', 'DOLBY')
ROOT = Path(__file__).resolve().parents[2]

# Hex literals in the native sources that lie in the image range but are not
# addresses (masks, sizes, NV2A methods and engine constants, colours).
CONSTANTS = {0x20000, 0x30000, 0x3FFC0, 0x3FFFF, 0x40000, 0x41EA4, 0x50000, 0x60000,
             0x70000, 0x80000, 0xA101E, 0xE0000, 0xF0000, 0xFFFFF, 0x100000, 0x200000,
             0x118700, 0x28C3FF}

def load(path):
    data = Path(path).read_bytes()
    base = struct.unpack_from('<I', data, 0x104)[0]
    count, table = struct.unpack_from('<II', data, 0x11C)
    sections = {}
    image = bytearray(HI - LO)
    image[0:0x1000] = data[0:0x1000]
    for i in range(count):
        flags, va, vsize, raw, rsize, name_va = struct.unpack_from('<IIIIII', data, table - base + i * 56)
        end = data.index(b'\0', name_va - base)
        name = data[name_va - base:end].decode()
        sections[name] = (va, va + vsize)
        n = min(rsize, vsize)
        image[va - LO:va - LO + n] = data[raw:raw + n]
    entry = struct.unpack_from('<I', data, 0x128)[0] ^ 0xA8FC57AB
    return data, bytes(image), sections, entry


def word(img, va):
    return int.from_bytes(img[va - LO:va - LO + 4], 'little')


def same_code(ia, ib, a, b, size, text):
    """True when [a, a+size) and [b, b+size) differ only in pointer operands."""
    xa, xb = ia[a - LO:a - LO + size], ib[b - LO:b - LO + size]
    if xa == xb:
        return True
    i = 0
    while i < size:
        if xa[i] == xb[i]:
            i += 1
            continue
        covered = False
        for s in range(max(0, i - 3), min(i, size - 4) + 1):
            wa, wb = word(ia, a + s), word(ib, b + s)
            if LO <= wa < HI and LO <= wb < HI:
                covered = True
            elif s >= 1 and xa[s - 1] in (0xE8, 0xE9) and xa[s - 1] == xb[s - 1]:
                ta = (a + s + 4 + wa) & 0xFFFFFFFF
                tb = (b + s + 4 + wb) & 0xFFFFFFFF
                if text[0] <= ta < text[1] and text[0] <= tb < text[1]:
                    covered = True
            if covered:
                i = s + 4
                break
        if not covered:
            return False
    return True


def build(us_path, target_path, functions_header):
    _, ia, sa, entry_a = load(us_path)
    target_bytes, ib, sb, entry_b = load(target_path)
    text = sa['.text']
    funcs = sorted({int(m, 16) for m in re.findall(r'sub_([0-9A-Fa-f]{8})\(', Path(functions_header).read_text())})
    tf = [f for f in funcs if text[0] <= f < text[1]]
    ends = {f: (tf[i + 1] if i + 1 < len(tf) else text[1]) for i, f in enumerate(tf)}
    fmap = {}
    # Anchors: functions of 32+ bytes found exactly once, unchanged, near their old place.
    for f in tf:
        size = ends[f] - f
        if size < 32:
            continue
        pattern = ia[f - LO:f - LO + 24]
        lo, hi = max(0, f - LO - 0x4000), f - LO + 0x4000
        hits = []
        k = ib.find(pattern, lo, hi)
        while k >= 0 and len(hits) < 3:
            hits.append(k + LO)
            k = ib.find(pattern, k + 1, hi)
        good = [h for h in hits if same_code(ia, ib, f, h, size, text)]
        if len(good) == 1:
            fmap[f] = good[0]
    # Keep anchors whose displacement agrees with a neighbour.
    keys = sorted(fmap)
    fmap = {f: fmap[f] for j, f in enumerate(keys)
            if (j and fmap[keys[j - 1]] - keys[j - 1] == fmap[f] - f) or
               (j + 1 < len(keys) and fmap[keys[j + 1]] - keys[j + 1] == fmap[f] - f)}
    keys = sorted(fmap)
    for f in tf:
        if f in fmap:
            continue
        j = bisect.bisect_left(keys, f)
        deltas = []
        if j:
            deltas.append(fmap[keys[j - 1]] - keys[j - 1])
        if j < len(keys):
            deltas.append(fmap[keys[j]] - keys[j])
        for d in dict.fromkeys(deltas):
            if same_code(ia, ib, f, f + d, ends[f] - f, text):
                fmap[f] = f + d
                break
    sections = {}
    for name in LIBS:
        a, b = sa[name], sb[name]
        if a[1] - a[0] != b[1] - b[0]:
            raise SystemExit(f'{name} changed size; libraries are not the same build')
        d = b[0] - a[0]
        lo = a[0]
        lib_funcs = [f for f in funcs if a[0] <= f < a[1]] + [a[1]]
        for f, nxt in zip(lib_funcs, lib_funcs[1:]):
            if not same_code(ia, ib, f, f + d, nxt - f, text):
                raise SystemExit(f'{name} function {f:08X} differs; refusing a block relocation')
            fmap[f] = f + d
        if lib_funcs[0] > lo and not same_code(ia, ib, lo, lo + d, lib_funcs[0] - lo, text):
            raise SystemExit(f'{name} head differs')
        sections[name] = (a[0], a[1], d)
    return dict(ia=ia, ib=ib, sa=sa, sb=sb, funcs=funcs, tf=tf, fmap=fmap, sections=sections,
                entry=(entry_a, entry_b), sha=hashlib.sha256(target_bytes).hexdigest())


def data_address(m, va):
    ia, ib = m['ia'], m['ib']
    for before in (0, 8, 16, 24):
        pattern = ia[va - before - LO:va - before - LO + 24]
        if pattern.count(0) > 18:
            continue
        lo, hi = max(0, va - before - LO - 0x2000), va - before - LO + 0x2000
        k = ib.find(pattern, lo, hi)
        if k >= 0 and ib.find(pattern, k + 1, hi) < 0:
            return k + before + LO
    return None


def translate(m, va):
    for name, (lo, hi, d) in m['sections'].items():
        if lo <= va < hi:
            return va + d
    text = m['sa']['.text']
    if text[0] <= va < text[1]:
        i = bisect.bisect_right(m['funcs'], va) - 1
        f = m['funcs'][i]
        return m['fmap'][f] + va - f if f in m['fmap'] else None
    for name in ('.rdata', '.data'):
        lo, hi = m['sa'][name]
        if lo <= va < hi:
            # Uninitialised data past the raw image has no bytes to match: it
            # moves with its section (the raw parts match at the same delta).
            got = data_address(m, va)
            return got if got is not None else va + m['sb'][name][0] - lo
    return None


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--us', required=True)
    p.add_argument('--target', required=True)
    p.add_argument('--us-functions', required=True)
    p.add_argument('--name', required=True)
    p.add_argument('--out', required=True)
    p.add_argument('--seeds-out', required=True)
    p.add_argument('--set-out', required=True)
    a = p.parse_args()
    m = build(a.us, a.target, a.us_functions)
    street = ROOT / 'street1'
    addresses, problems = {}, []
    for path, line_no, va, in_code in literals(street):
        if va in CONSTANTS:
            continue
        eu = translate(m, va)
        if eu is None:
            if in_code:
                problems.append(f'{path.relative_to(ROOT).as_posix()}:{line_no}: {va:08X}')
        else:
            addresses[va] = eu
    if problems:
        raise SystemExit('Unmapped addresses:\n  ' + '\n  '.join(problems))
    if translate(m, m['entry'][0]) != m['entry'][1]:
        raise SystemExit('Entry point does not map to the target entry point')
    seeds = json.loads((street / 'function-seeds.json').read_text(encoding='utf-8'))
    out_seeds, dropped = [], 0
    for s in seeds:
        va = int(s['start'], 16)
        eu = m['fmap'].get(va)
        if eu is None and LO <= va < HI:
            eu = translate(m, va)  # an entry inside a moved function or library
        if eu is None:
            dropped += 1
            continue
        out_seeds.append(dict(s, start=f'0x{eu:08X}'))
    fset = json.loads((street / 'function-set.json').read_text(encoding='utf-8'))
    moved_set = []
    for start, extent in fset['functions']:
        va = int(start, 16)
        eu = m['fmap'].get(va)
        if eu is None:
            eu = translate(m, va) if LO <= va < HI else None
            # Only whole functions whose bytes match move as a validated
            # extent; changed functions are left to the disassembler.
            if eu is None or not same_code(m['ia'], m['ib'], va, eu, extent, m['sa']['.text']):
                continue
        moved_set.append([f'{eu:08X}', extent])
    Path(a.set_out).write_text(json.dumps(dict(note=fset['note'].replace('USA build', a.name + ' executable, moved from the validated USA build'),
                                               functions=sorted(moved_set)), separators=(',', ':')).replace('],[', '],' + chr(10) + '[') + chr(10),
                               encoding='utf-8')
    table = dict(
        region=a.name, xbeSha256=m['sha'],
        note='USA address -> address in this release, for the native sources only. '
             'Generated by street1/regions/make-region-table.py; contains no game code.',
        constants=[f'{c:08X}' for c in sorted(CONSTANTS)],
        addresses={f'{k:08X}': f'{v:08X}' for k, v in sorted(addresses.items())})
    Path(a.out).write_text(json.dumps(table, indent=1) + '\n', encoding='utf-8')
    Path(a.seeds_out).write_text(json.dumps(out_seeds, indent=2) + '\n', encoding='utf-8')
    print(f'{len(addresses)} source addresses; functions mapped {len(m["fmap"])}/{len(m["funcs"])}; '
          f'seeds {len(out_seeds)} kept, {dropped} dropped; function set {len(moved_set)}/{len(fset["functions"])}')


if __name__ == '__main__':
    main()
