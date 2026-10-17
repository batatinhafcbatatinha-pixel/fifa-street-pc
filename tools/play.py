"""Launch a privately built game with the native graphics/audio profile."""
import argparse
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-files', type=Path, default=ROOT / 'game_files')
    parser.add_argument('--local-dir', type=Path, default=ROOT / '.local')
    args = parser.parse_args()
    assets, local = args.game_files.resolve(), args.local_dir.resolve()
    if assets.name != 'game_files' or not (assets / 'default.xbe').is_file():
        parser.error('Use an extracted disc folder named game_files containing default.xbe')
    executable = local / 'build' / ('fifastreet_recomp.exe' if os.name == 'nt' else 'fifastreet_recomp')
    if not executable.is_file():
        parser.error('Build locally with tools/build_local.py first')
    env = {k: v for k, v in os.environ.items() if not k.startswith('RECOMP_')}
    # 100% native: no model of any console device (NV2A, APU/DSP, AC97, USB, flash).
    for name in ('NO_HW', 'HLE_PRIMARY', 'HLE_NOGPU', 'DS_NATIVE', 'INPUT_NATIVE', 'VBLANK',
                 'TITLE_KEVENTS', 'GUEST_LOCK', 'GUEST_SMP', 'KEYBOARD'):
        env['RECOMP_' + name] = '1'
    env.setdefault('DXVK_LOG_LEVEL', 'none')
    logs = local / 'logs'
    logs.mkdir(parents=True, exist_ok=True)
    with (logs / 'stdout.log').open('wb') as stdout, (logs / 'stderr.log').open('wb') as stderr:
        raise SystemExit(subprocess.call([str(executable)], cwd=assets.parent, env=env, stdout=stdout, stderr=stderr))

if __name__ == '__main__':
    main()
