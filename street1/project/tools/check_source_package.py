"""Unpack a source installer in a NEW directory and configure CMake (no build)."""
import argparse
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import zipfile
from fifa_setup import developer_environment


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('destination', type=Path)
    args = parser.parse_args()
    destination = args.destination.resolve()
    if destination.exists():
        raise ValueError('Validation requires a new destination directory')
    with zipfile.ZipFile(args.archive) as archive:
        for name in archive.namelist():
            path = PurePosixPath(name)
            if path.is_absolute() or '..' in path.parts or ':' in name or '\\' in name:
                raise ValueError('Unsafe archive entry: ' + name)
        destination.mkdir()
        archive.extractall(destination)
    env = developer_environment()
    scratch = destination / 'build-temp'
    scratch.mkdir()
    env['TMP'] = env['TEMP'] = str(scratch)
    command = [shutil.which('cmake', path=env['PATH']), '-S', str(destination / 'project'),
               '-B', str(destination / 'build'), '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=RelWithDebInfo',
               '-DCMAKE_C_COMPILER=' + shutil.which('clang-cl', path=env['PATH']),
               '-DCMAKE_MAKE_PROGRAM=' + shutil.which('ninja', path=env['PATH']),
               '-DXBOXRECOMP_DIR=' + str(destination / 'xboxrecomp')]
    with (destination / 'configure.log').open('w', encoding='utf-8') as log:
        result = subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT)
    print((destination / 'configure.log').read_text(encoding='utf-8'))
    if result.returncode:
        raise SystemExit(result.returncode)
    print('Source package CMake configure passed. No game objects were built.')


if __name__ == '__main__':
    main()
