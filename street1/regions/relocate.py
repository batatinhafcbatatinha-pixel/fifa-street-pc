"""Move the USA addresses written in the native sources to another release.

The hand-written C and the routing tables under street1/ name game and library
functions by their USA address (sub_0025DB90, 0x260FA0u, "0025DB90": ...).
For another release of the same build, street1/regions/<region>.json lists
where each of those addresses went. relocate() rewrites a private copy of
street1/ with it before generation; the published sources stay USA.

Every image-range literal in code must be either listed in the table or
declared a constant there; anything else stops the build rather than leaving
a USA address behind in another executable.
"""
import json
from pathlib import Path
import re

IMAGE = (0x11000, 0x314008)
# Files whose addresses are rewritten (relative to street1/).
SOURCES = ['project/src/*.c', 'project/src/*.h', 'hle/hle_functions.json', 'hle/dsound_functions.json',
           'hle/sync_functions.json', 'hle/input_functions.json', 'hle/smp_serial_functions.json',
           'hle/game_hooks.json']
TOKEN = re.compile(r'(?<![0-9A-Za-z_])(?:(0x)([0-9A-Fa-f]{5,8})(?![0-9A-Fa-f])|'
                   r'(sub_|HLE_API\(|HLE_DS\(|")([0-9A-Fa-f]{8})(?![0-9A-Za-z]))')


def token_value(match):
    return int(match[2] or match[4], 16)


def strip_comments(text):
    """Blank C comments, keeping offsets (string contents are kept)."""
    out, i, n = [], 0, len(text)
    while i < n:
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
        elif text[i] == '"':
            j = i + 1
            while j < n and text[j] != '"' and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
            continue
        else:
            out.append(text[i])
            i += 1
            continue
        out.append(re.sub(r'[^\n]', ' ', text[i:j]))
        i = j
    return ''.join(out)


def literals(street):
    """Yield (path, line, value, in_code) for every image-range literal."""
    for pattern in SOURCES:
        for path in sorted(street.glob(pattern)):
            text = path.read_text(encoding='utf-8')
            code = strip_comments(text) if path.suffix in ('.c', '.h') else text
            for m in TOKEN.finditer(text):
                value = token_value(m)
                if IMAGE[0] <= value < IMAGE[1]:
                    line = text.count('\n', 0, m.start()) + 1
                    yield path, line, value, code[m.start()] == text[m.start()]


def relocate(street, table_path):
    table = json.loads(Path(table_path).read_text(encoding='utf-8'))
    moved = {int(k, 16): int(v, 16) for k, v in table['addresses'].items()}
    constants = {int(c, 16) for c in table['constants']}
    unknown, changed = [], 0
    for pattern in SOURCES:
        for path in sorted(street.glob(pattern)):
            text = path.read_text(encoding='utf-8')
            code = strip_comments(text) if path.suffix in ('.c', '.h') else text

            def swap(m):
                nonlocal changed
                value = token_value(m)
                if not IMAGE[0] <= value < IMAGE[1] or value in constants:
                    return m[0]
                in_code = code[m.start()] == text[m.start()]
                if value not in moved:
                    if in_code:
                        unknown.append(f'{path.name}:{text.count(chr(10), 0, m.start()) + 1}: {value:08X}')
                    return m[0]
                digits = m[2] or m[4]
                new = f'{moved[value]:0{len(digits)}X}'
                if digits.lower() == digits and any(c.isalpha() for c in digits):
                    new = new.lower()
                changed += 1
                return (m[1] or m[3]) + new
            result = TOKEN.sub(swap, text)
            if result != text:
                path.write_text(result, encoding='utf-8')
    if unknown:
        raise ValueError('Addresses without a ' + table['region'] + ' mapping:\n  ' + '\n  '.join(unknown))
    return changed
