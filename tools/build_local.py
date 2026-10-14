"""Generate private game code from a verified user-supplied USA XBE, then build."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('fifa_setup', ROOT / 'street1/project/tools/fifa_setup.py')
SETUP = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(SETUP)

def run(args, cwd, env):
    print('Running:', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), cwd=cwd, env=env, check=True)

def generate(xbe, local, env):
    runtime = ROOT / 'xboxrecomp'
    private_street = local / 'source/street1'
    shutil.copytree(ROOT / 'street1', private_street, dirs_exist_ok=True)
    gen = private_street / 'project/src/recomp/gen'
    gen.mkdir(parents=True, exist_ok=True)
    disasm, identified, abi = (local / name for name in ('disasm', 'identified', 'abi'))
    for directory in (disasm, identified, abi):
        directory.mkdir(exist_ok=True)
    run([sys.executable, '-m', 'tools.disasm', xbe, '--text-only', '--extra-sections',
         'D3D,XGRPH,DSOUND,XPP,DOLBY', '--force', '--seed-functions',
         ROOT / 'street1/function-seeds.json', '--output', disasm], runtime, env)
    run([sys.executable, '-m', 'tools.func_id', xbe, '--functions', disasm / 'functions.json',
         '--strings', disasm / 'strings.json', '--xrefs', disasm / 'xrefs.json', '--output', identified], runtime, env)
    run([sys.executable, '-m', 'tools.abi_analysis', xbe, '--disasm-dir', disasm,
         '--func-id-dir', identified, '--output', abi / 'abi_functions.json'], runtime, env)
    run([sys.executable, '-m', 'tools.recomp', xbe, '--game-name', 'FIFA Street', '--all',
         '--split', '250', '--disasm-dir', disasm, '--func-id-dir', identified,
         '--abi-dir', abi, '--gen-dir', gen], runtime, env)
    # Preserve hardware acknowledgement semantics without publishing translated instructions.
    count = 0
    for path in gen.glob('recomp_*.c'):
        text = path.read_text(encoding='utf-8')
        text, changed = re.subn(r'MEM32\(((?:esi|edi|ecx|eax) \+ 0x(?:400100|600100))\) = ([^;]+);',
                               r'xbox_Nv2aAckInterrupt(\1, \2);', text)
        if changed:
            text = text.replace('#include <math.h>', '#include <math.h>\nextern void xbox_Nv2aAckInterrupt(uint32_t, uint32_t);', 1)
            path.write_text(text, encoding='utf-8')
            count += changed
    if count != 6:
        raise ValueError(f'Expected 6 interrupt acknowledgement sites, found {count}; refusing an unverified build')
    run([sys.executable, private_street / 'translate-ebp-boundaries.py', '--apply'], ROOT, env)
    run([sys.executable, private_street / 'hle/apply-hle-wrappers.py'], ROOT, env)
    return private_street / 'project'

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-files', type=Path, required=True, help='Extracted original-Xbox USA disc directory')
    parser.add_argument('--local-dir', type=Path, default=ROOT / '.local')
    parser.add_argument('--generate-only', action='store_true')
    parser.add_argument('--reuse-generated', action='store_true', help='Reuse this local directory after a completed generation')
    parser.add_argument('--dxvk-native-dir', type=Path)
    parser.add_argument('--vkd3d-dir', type=Path)
    args = parser.parse_args()
    assets = args.game_files.resolve()
    xbe = assets / 'default.xbe'
    if hashlib.sha256(xbe.read_bytes()).hexdigest() != SETUP.XBE_SHA256:
        raise ValueError('Unsupported game: only the verified original-Xbox USA executable is accepted')
    local = args.local_dir.resolve()
    local.mkdir(parents=True, exist_ok=True)
    env = SETUP.developer_environment() if os.name == 'nt' and not args.generate_only else os.environ.copy()
    project = local / 'source/street1/project' if args.reuse_generated else generate(xbe, local, env)
    if not (project / 'src/recomp/gen/recomp_funcs.h').is_file():
        raise ValueError('Generated code missing: run without --reuse-generated first')
    if args.generate_only:
        print('Private game code generated in', local)
        return
    cmake = shutil.which('cmake', path=env.get('PATH'))
    compiler = shutil.which('clang-cl' if os.name == 'nt' else 'clang', path=env.get('PATH'))
    if not cmake or not compiler:
        raise ValueError('CMake and LLVM/Clang are required')
    configure = [cmake, '-S', project, '-B', local / 'build', '-G', 'Ninja',
                 '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DCMAKE_C_COMPILER=' + compiler,
                 '-DXBOXRECOMP_DIR=' + str(ROOT / 'xboxrecomp')]
    if args.dxvk_native_dir:
        configure.append('-DDXVK_NATIVE_DIR=' + str(args.dxvk_native_dir.resolve()))
    if args.vkd3d_dir:
        configure.append('-DVKD3D_DIR=' + str(args.vkd3d_dir.resolve()))
    run(configure, ROOT, env)
    run([cmake, '--build', local / 'build', '--parallel', '4', '--target', 'fifastreet_recomp'], ROOT, env)
    print('Built privately in', local / 'build')
    print('Run from the folder containing game_files, following README.md. Do not upload the generated code or executable.')

if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print('Build failed:', error, file=sys.stderr)
        sys.exit(1)
