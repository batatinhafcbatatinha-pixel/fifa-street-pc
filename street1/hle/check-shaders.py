"""Extract HLSL string literals from the HLE sources and compile them with fxc."""
import re, subprocess, sys, os
src_dir, out_dir = sys.argv[1], sys.argv[2]
fxc = r"C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe"
jobs = [
    ('hle_draw.c', 'static const char k_ps_source[] =', [('main', 'ps_5_0')]),
    ('hle_draw.c', 'static const char k_clear_vs[] =', [('main', 'vs_5_0')]),
    ('hle_draw.c', 'static const char k_clear_ps[] =', [('main', 'ps_5_0')]),
    ('hle_native_geometry.c', 'static const char s_source[] =',
     [('points', 'gs_5_0'), ('lines', 'gs_5_0'), ('triangles', 'gs_5_0')]),
    ('hle_native_present.c', 'source[] =', [('vs', 'vs_5_0'), ('ps', 'ps_5_0'), ('ps_fxaa', 'ps_5_0')]),
    ('hle_options.c', 'static const char src[] =', [('vs', 'vs_5_0'), ('ps', 'ps_5_0')]),
]
bad = 0
for fname, marker, entries in jobs:
    text = open(os.path.join(src_dir, fname), encoding='utf-8').read()
    i = text.index(marker)
    j = text.index(';\n', i)
    parts = re.findall(r'"((?:[^"\\]|\\.)*)"', text[i:j])
    hlsl = ''.join(parts).encode().decode('unicode_escape')
    path = os.path.join(out_dir, fname + '.' + marker.split()[-2].strip('[]') + '.hlsl')
    open(path, 'w').write(hlsl)
    for entry, prof in entries:
        r = subprocess.run([fxc, '/nologo', '/T', prof, '/E', entry, '/Fo', os.devnull, path],
                           capture_output=True, text=True)
        msg = (r.stdout + r.stderr).strip()
        errs = [l for l in msg.splitlines() if 'error' in l]
        print(('OK  ' if r.returncode == 0 else 'FAIL'), fname, entry, prof)
        for l in errs[:5]:
            print('   ', l)
        bad += r.returncode != 0
sys.exit(1 if bad else 0)
