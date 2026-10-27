"""Audit committed source files and create an explicitly allowlisted source ZIP."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
ALLOWED = {'.c', '.h', '.inc', '.hlsl', '.cmake', '.txt', '.md', '.py', '.json', '.cmd', '.sh', '.yml', '.yaml'}
ROOT_FILES = {'.gitignore', '.gitmodules', 'LICENSE', 'NOTICE', 'requirements.txt'}
FORBIDDEN_PARTS = {'.local', 'game_files', 'gen', 'output', 'fixtures', '__pycache__', 'build', 'dist'}
MAGIC = (b'MZ', b'XBEH', b'\x7fELF', b'PK\x03\x04', b'\x89PNG\r\n\x1a\n', b'RIFF', b'DDS ')

def submodules():
    """Paths of the submodules (the xboxrecomp toolkit), from .gitmodules."""
    if not (ROOT / '.gitmodules').is_file():
        return ()
    lines = subprocess.check_output(['git', 'config', '--file', '.gitmodules', '--get-regexp', r'\.path$'],
                                    cwd=ROOT).decode('utf-8').splitlines()
    return tuple(line.split(' ', 1)[1] + '/' for line in lines)

def audited_files():
    # The toolkit submodule goes into the kit too (it must be checked out). Of its files only the allowed source
    # types are taken; this repository's own files must all pass.
    toolkit = submodules()
    names = subprocess.check_output(['git', 'ls-files', '-z', '--recurse-submodules'],
                                    cwd=ROOT).decode('utf-8').split('\0')
    files = []
    for name in filter(None, names):
        path = PurePosixPath(name)
        vendored = name.startswith(toolkit) if toolkit else False
        if path.is_absolute() or '..' in path.parts or any(part in FORBIDDEN_PARTS for part in path.parts):
            if vendored:
                continue
            raise ValueError('Forbidden publication path: ' + name)
        source = ROOT / name
        if source.is_symlink() or not source.is_file():
            raise ValueError('Only regular files may be published: ' + name)
        if path.suffix.lower() not in ALLOWED and path.name not in ROOT_FILES and not path.name.startswith(('LICENSE', 'COPYING')):
            if vendored:
                continue
            raise ValueError('Not an allowed source file: ' + name)
        payload = source.read_bytes()
        if b'\0' in payload or payload.startswith(MAGIC):
            if vendored:
                continue
            raise ValueError('Binary content rejected: ' + name)
        if (name.startswith('street1/project/src/recomp/') or
            path.suffix in {'.c', '.h'} and b'Original: 0x' in payload and b'#define RECOMP_GENERATED_CODE' in payload):
            raise ValueError('Generated game code rejected: ' + name)
        files.append((name, payload))
    if not files:
        raise ValueError('No tracked files to audit')
    return sorted(files)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--zip', type=Path)
    args = parser.parse_args()
    files = audited_files()
    if args.zip:
        destination = args.zip.resolve()
        destination.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(destination, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
            manifest = []
            for name, payload in files:
                info = zipfile.ZipInfo('fifa-street-pc/' + name, (2026, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                archive.writestr(info, payload)
                manifest.append({'path': name, 'bytes': len(payload), 'sha256': hashlib.sha256(payload).hexdigest()})
            archive.writestr('fifa-street-pc/source-manifest.json', json.dumps(manifest, indent=2) + '\n')
        with zipfile.ZipFile(destination) as archive:
            if archive.testzip():
                raise ValueError('ZIP integrity failed')
        print('Source ZIP:', destination, destination.stat().st_size, 'bytes')
    print('PASS:', len(files), 'tracked source/license/documentation files; no game binaries or generated game units')

if __name__ == '__main__':
    main()
