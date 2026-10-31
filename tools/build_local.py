"""Generate private game code from a verified user-supplied XBE (USA or Europe), then build."""
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
RSPEC = importlib.util.spec_from_file_location('relocate', ROOT / 'street1/regions/relocate.py')
RELOCATE = importlib.util.module_from_spec(RSPEC)
RSPEC.loader.exec_module(RELOCATE)

def identify(xbe):
    """Which supported release this executable is, or a clear refusal."""
    digest = hashlib.sha256(xbe.read_bytes()).hexdigest()
    if digest == SETUP.XBE_SHA256:
        return dict(name='usa', sha256=digest, table=None, seeds=None)
    for table in sorted((ROOT / 'street1/regions').glob('*.json')):
        if table.name.startswith('function-'):
            continue
        info = json.loads(table.read_text(encoding='utf-8'))
        if info.get('xbeSha256') == digest:
            return dict(name=info['region'], sha256=digest, table=table,
                        seeds=table.with_name(f"function-seeds-{info['region']}.json"))
    raise ValueError('Unsupported game: only the verified original-Xbox USA or European executable is accepted')

def run(args, cwd, env):
    print('Running:', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), cwd=cwd, env=env, check=True)

def generate(xbe, release, local, env):
    runtime = ROOT / 'xboxrecomp'
    private_street = local / 'source/street1'
    shutil.copytree(ROOT / 'street1', private_street, dirs_exist_ok=True)
    seeds = ROOT / 'street1/function-seeds.json'
    function_set = ROOT / 'street1/function-set.json'
    if release['table']:
        # Another release of the same build: move the USA addresses named by the
        # native sources and routing tables before anything reads them.
        changed = RELOCATE.relocate(private_street, release['table'])
        print(f"Relocated {changed} USA addresses for the {release['name']} executable", flush=True)
        seeds = release['seeds']
        function_set = release['table'].with_name(f"function-set-{release['name']}.json")
    gen = private_street / 'project/src/recomp/gen'
    shutil.rmtree(gen, ignore_errors=True)  # never mix translations of two executables
    gen.mkdir(parents=True)
    disasm, identified, abi = (local / name for name in ('disasm', 'identified', 'abi'))
    for directory in (disasm, identified, abi):
        directory.mkdir(exist_ok=True)
    # Header/section analysis of this XBE, kept with the private output (not beside the game files).
    analysis = local / 'default_analysis.json'
    run([sys.executable, '-m', 'tools.xbe_parser', xbe, '--quiet', '--json', analysis], runtime, env)
    run([sys.executable, '-m', 'tools.disasm', xbe, '--analysis-json', analysis, '--text-only', '--extra-sections',
         'D3D,XGRPH,DSOUND,XPP,DOLBY', '--force', '--seed-functions',
         seeds, '--output', disasm], runtime, env)
    run([sys.executable, private_street / 'apply-function-set.py', disasm / 'functions.json', function_set], ROOT, env)
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
    (local / 'generation-complete.json').write_text(json.dumps({'xbeSha256': release['sha256'], 'release': release['name']}) + '\n', encoding='utf-8')
    return private_street / 'project'

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-files', type=Path, required=True, help='Extracted original-Xbox USA or European disc directory')
    parser.add_argument('--local-dir', type=Path, default=ROOT / '.local')
    parser.add_argument('--generate-only', action='store_true')
    parser.add_argument('--reuse-generated', action='store_true', help='Reuse this local directory after a completed generation')
    parser.add_argument('--dxvk-native-dir', type=Path)
    parser.add_argument('--vkd3d-dir', type=Path)
    parser.add_argument('--dxmt-dir', type=Path, help='macOS: DXMT prefix from tools/macos/build-deps.sh')
    args = parser.parse_args()
    assets = args.game_files.resolve()
    xbe = assets / 'default.xbe'
    release = identify(xbe)
    print(f"Executable: {release['name']} release ({release['sha256'][:16]}...)", flush=True)
    local = args.local_dir.resolve()
    local.mkdir(parents=True, exist_ok=True)
    if args.reuse_generated:
        marker = local / 'generation-complete.json'
        if not marker.is_file():
            raise ValueError('Generation has not finished in this directory; do not reuse incomplete output')
        if json.loads(marker.read_text(encoding='utf-8')).get('xbeSha256') != release['sha256']:
            raise ValueError('This directory was generated from a different executable; generate again or use another --local-dir')
    env = SETUP.developer_environment() if os.name == 'nt' and not args.generate_only else os.environ.copy()
    project = local / 'source/street1/project' if args.reuse_generated else generate(xbe, release, local, env)
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
    if sys.platform == 'darwin':
        if not (args.dxmt_dir and args.vkd3d_dir):
            raise ValueError('macOS: pass --dxmt-dir and --vkd3d-dir (see MACOS.md)')
        configure += ['-DDXMT_DIR=' + str(args.dxmt_dir.resolve()), '-DCMAKE_OSX_ARCHITECTURES=x86_64']
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
