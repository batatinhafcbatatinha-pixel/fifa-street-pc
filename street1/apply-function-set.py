"""Make the disassembler's function list match the validated function set.

The disassembler finds function starts heuristically. Seeded or not, it can
split a validated function in two (a jump table read as code, an alignment
guess, a call into the middle of a body), and the translation then turns the
internal jump across the cut into a call of a "function" that may have no body
or a different frame. The validated private build used one particular set of
functions; street1/function-set.json lists it as [start, extent] pairs
(addresses only).

  apply-function-set.py <functions.json> <function-set.json>

* A detected start that falls inside a validated function is dropped and the
  containing function grows to cover it.
* A validated function the disassembler missed is added with its extent.
* Everything else (code outside every validated extent) is kept as detected.
"""
import bisect
import json
from pathlib import Path
import sys


def main():
    functions_path, set_path = Path(sys.argv[1]), Path(sys.argv[2])
    functions = json.loads(functions_path.read_text(encoding='utf-8'))
    reference = sorted((int(s, 16), n) for s, n in json.loads(set_path.read_text(encoding='utf-8'))['functions'])
    ref_starts = [s for s, _ in reference]
    ref_extent = dict(reference)
    by_start = {int(f['start'], 16): f for f in functions}

    longest = max(n for _, n in reference)

    def owner(address):
        # Validated functions may overlap (a body that runs on into the next
        # entry point), so the covering one is not always the nearest start.
        i = bisect.bisect_right(ref_starts, address) - 1
        while i >= 0 and ref_starts[i] + longest > address:
            if ref_starts[i] < address < ref_starts[i] + reference[i][1]:
                return ref_starts[i]
            i -= 1
        return None

    dropped = added = grown = 0
    for start in sorted(by_start):
        if start in ref_extent:
            continue
        parent = owner(start)
        if parent is None:
            continue
        cut = by_start.pop(start)
        dropped += 1
        holder = by_start.get(parent)
        if holder is not None and int(holder['end'], 16) < int(cut['end'], 16):
            holder['end'] = cut['end']
            holder['size'] = int(cut['end'], 16) - parent
            grown += 1
    for start, extent in reference:
        if start in by_start:
            # The translator calls every jump past 'end' a tail call; the
            # validated body reached this far, so the jumps stay inside it.
            entry = by_start[start]
            if int(entry['end'], 16) < start + extent:
                entry['end'] = f'0x{start + extent:08X}'
                entry['size'] = extent
                grown += 1
            continue
        section = next((f['section'] for f in functions
                        if int(f['start'], 16) <= start < int(f['end'], 16)), '.text')
        by_start[start] = dict(start=f'0x{start:08X}', end=f'0x{start + extent:08X}', size=extent,
                               name=f'sub_{start:08X}', section=section, confidence=1.0,
                               detection_method='validated_set', num_instructions=0,
                               has_prologue=False, calls_to=[], called_by=[])
        added += 1
    result = [by_start[s] for s in sorted(by_start)]
    functions_path.write_text(json.dumps(result, indent=1) + '\n', encoding='utf-8')
    print(f'function set: {len(result)} functions ({dropped} split points dropped, '
          f'{grown} functions regrown, {added} validated functions added)')


if __name__ == '__main__':
    main()
