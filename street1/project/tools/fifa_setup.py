"""Local FIFA Street (original Xbox) installation. Python 3.10+, no downloads."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys

TITLE_ID = 0x45410072
XBE_SHA256 = 'f43dcca6fbd99e7a04dc94992443b368eefaa9d6bbc88432bf43b2d90e115ef1'
PROJECT = Path(__file__).resolve().parents[1]


class Disc:
    def __init__(self, filename):
        self.file = open(filename, 'rb')
        try:
            self._initialize()
        except Exception:
            self.file.close()
            raise

    def _initialize(self):
        self.length = os.fstat(self.file.fileno()).st_size
        self.base = next((b for b in (0, 0xFD90000, 0x2080000)
                          if b + 0x10800 <= self.length and
                          self.read(b + 0x10000, 20) == b'MICROSOFT*XBOX*MEDIA'), None)
        if self.base is None:
            raise ValueError('No supported original-Xbox XDVDFS volume found')
        volume = self.read(self.base + 0x10000, 2048)
        self.entries = []
        self.directories = set()
        self.names = set()
        self.walk(*struct.unpack_from('<II', volume, 20), prefix='', depth=0)
        executable = next((e for e in self.entries if e['path'].lower() == 'default.xbe'), None)
        if not executable:
            raise ValueError('default.xbe missing: Xbox 360 XEX games are unsupported')
        if executable['size'] > 128 * 1024 * 1024:
            raise ValueError('Unreasonable XBE size')
        xbe = self.read(self.base + executable['sector'] * 2048, executable['size'])
        if len(xbe) < 0x120 or xbe[:4] != b'XBEH':
            raise ValueError('Invalid original-Xbox XBE')
        certificate = struct.unpack_from('<I', xbe, 0x118)[0] - struct.unpack_from('<I', xbe, 0x104)[0]
        if certificate < 0 or certificate + 92 > len(xbe):
            raise ValueError('Invalid XBE certificate')
        self.title_id = struct.unpack_from('<I', xbe, certificate + 8)[0]
        if self.title_id != TITLE_ID:
            raise ValueError(f'Wrong game: TitleID {self.title_id:08X}, expected {TITLE_ID:08X}')
        self.xbe_hash = hashlib.sha256(xbe).hexdigest()
        if self.xbe_hash != XBE_SHA256:
            raise ValueError('Unsupported executable revision: this source targets the verified USA XBE only')

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset + size > self.length:
            raise ValueError('Disc extent outside ISO')
        self.file.seek(offset)
        data = self.file.read(size)
        if len(data) != size:
            raise ValueError('Truncated ISO')
        return data

    def walk(self, sector, size, prefix, depth):
        if depth > 64 or size > 16 * 1024 * 1024 or (sector, size) in self.directories:
            raise ValueError('Invalid directory tree')
        self.directories.add((sector, size))
        if not size:
            return
        data = self.read(self.base + sector * 2048, size)
        pending, seen = [0], set()
        while pending:
            offset = pending.pop()
            if offset in seen or offset + 14 > len(data):
                raise ValueError('Invalid/cyclic directory entry')
            seen.add(offset)
            left, right, start, length, attributes, n = struct.unpack_from('<HHIIBB', data, offset)
            if not n or offset + 14 + n > len(data):
                raise ValueError('Invalid directory name')
            name = data[offset + 14:offset + 14 + n].decode('ascii')
            if (name in ('.', '..') or name[-1] in ' .' or
                    any(c in name for c in '\\/:*?"<>|') or any(ord(c) < 32 for c in name) or
                    name.split('.')[0].upper() in {'CON', 'PRN', 'AUX', 'NUL', *(f'COM{i}' for i in range(10)), *(f'LPT{i}' for i in range(10))}):
                raise ValueError('Unsafe filename in ISO')
            relative = prefix + name
            if relative.lower() in self.names:
                raise ValueError('Duplicate filename in ISO')
            self.names.add(relative.lower())
            if left:
                pending.append(left * 4)
            if right:
                pending.append(right * 4)
            if attributes & 0x10:
                self.walk(start, length, relative + '/', depth + 1)
            else:
                if self.base + start * 2048 + length > self.length:
                    raise ValueError('File extends beyond ISO')
                self.entries.append(dict(path=relative, sector=start, size=length))
                if len(self.entries) > 200000:
                    raise ValueError('Unreasonable file count')

    def summary(self):
        names = {e['path'].lower() for e in self.entries}
        languages = [label for label, code in [('English', 'eng'), ('Spanish', 'sp')]
                     if f'data/fe{code}.loc' in names and f'data/mc{code}.loc' in names]
        return dict(titleId=f'{self.title_id:08X}', xbeSha256=self.xbe_hash,
                    volumeOffset=self.base, fileCount=len(self.entries),
                    extractedBytes=sum(e['size'] for e in self.entries),
                    languageResources=languages,
                    languageNote='Resource presence only; runtime selection must be separately verified.')

    def extract(self, destination):
        destination.mkdir()  # Never merge with or overwrite an existing installation.
        records = []
        for entry in self.entries:
            target = destination.joinpath(*entry['path'].split('/'))
            target.parent.mkdir(parents=True, exist_ok=True)
            digest = hashlib.sha256()
            remaining, offset = entry['size'], self.base + entry['sector'] * 2048
            with target.open('xb') as output:
                while remaining:
                    chunk = self.read(offset, min(1024 * 1024, remaining))
                    output.write(chunk)
                    digest.update(chunk)
                    remaining -= len(chunk)
                    offset += len(chunk)
            records.append(dict(path=entry['path'], size=entry['size'], sha256=digest.hexdigest()))
            print(f'Extracted {len(records)}/{len(self.entries)}: {entry["path"]}', flush=True)
        return records


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def developer_environment():
    """Discover already installed tools; never install or change system settings."""
    env = {k.upper(): v for k, v in os.environ.items()}
    roots = [PROJECT, *PROJECT.parents]
    extra = [Path(sys.executable).parent]
    for root in roots:
        extra.extend([root / 'recomp-python/Scripts', root / 'dependencies/cmake/bin',
                      root / 'dependencies/ninja', root / 'dependencies/llvm/bin'])
    for key in ('PROGRAMFILES', 'PROGRAMFILES(X86)'):
        if env.get(key):
            extra.extend([Path(env[key]) / 'LLVM/bin', Path(env[key]) / 'CMake/bin'])
    vswhere = shutil.which('vswhere')
    if not vswhere:
        candidate = Path(env.get('PROGRAMFILES(X86)', 'C:/Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
        if candidate.is_file():
            vswhere = str(candidate)
    if vswhere:
        result = subprocess.run([vswhere, '-latest', '-products', '*', '-requires',
            'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'],
            capture_output=True, text=True, check=True)
        location = result.stdout.strip()
        if location:
            install = Path(location)
            vcvars = install / 'VC/Auxiliary/Build/vcvars64.bat'
            extra.extend([install / 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin',
                          install / 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja',
                          install / 'VC/Tools/Llvm/x64/bin', install / 'VC/Tools/Llvm/bin'])
            if vcvars.is_file():
                # Only the verified installer path enters this shell command.
                if any(c in str(vcvars) for c in '"%\r\n'):
                    raise ValueError('Unsupported special character in Visual Studio installation path')
                result = subprocess.run(f'cmd.exe /d /s /c ""{vcvars}" >nul && set"',
                    capture_output=True, text=True, check=True)
                env.update((line.split('=', 1)[0].upper(), line.split('=', 1)[1])
                           for line in result.stdout.splitlines() if '=' in line and not line.startswith('='))
    env['PATH'] = os.pathsep.join(str(p) for p in extra if p.is_dir()) + os.pathsep + env.get('PATH', '')
    for command in ('cmake', 'ninja', 'clang-cl'):
        if not shutil.which(command, path=env['PATH']):
            raise ValueError(f'{command} missing. Install CMake, Ninja and LLVM plus Visual Studio C++ Build Tools/Windows SDK, then retry.')
    if not env.get('INCLUDE') or not env.get('LIB'):
        raise ValueError('Visual Studio x64 C++/Windows SDK environment not found. Install those Build Tools components and retry.')
    return env


def build(workspace, runtime):
    env = developer_environment()
    if not (runtime / 'CMakeLists.txt').is_file():
        raise ValueError('Runtime source directory is invalid')
    scratch = workspace / 'build-temp'
    scratch.mkdir(exist_ok=True)
    env['TMP'] = env['TEMP'] = str(scratch)
    build_dir = workspace / 'build'
    cmake = shutil.which('cmake', path=env['PATH'])
    compiler = shutil.which('clang-cl', path=env['PATH'])
    ninja = shutil.which('ninja', path=env['PATH'])
    configure = [cmake, '-S', str(PROJECT), '-B', str(build_dir), '-G', 'Ninja',
                 '-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DCMAKE_C_COMPILER=' + compiler,
                 '-DCMAKE_MAKE_PROGRAM=' + ninja,
                 '-DXBOXRECOMP_DIR=' + str(runtime)]
    print('Configuring local C/C++ build...', flush=True)
    subprocess.run(configure, check=True, env=env)
    print('Compiling FIFA Street locally...', flush=True)
    subprocess.run([cmake, '--build', str(build_dir), '--parallel', '4', '--target', 'fifastreet_recomp'], check=True, env=env)
    shutil.copy2(build_dir / 'fifastreet_recomp.exe', workspace / 'fifastreet_recomp.exe')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('inspect', 'install', 'build'))
    parser.add_argument('--iso', type=Path)
    parser.add_argument('--workspace', type=Path)
    parser.add_argument('--runtime', type=Path)
    parser.add_argument('--extract-only', action='store_true', help='Prepare assets without compiling')
    args = parser.parse_args()
    if args.action == 'build':
        if not args.workspace or not args.runtime:
            parser.error('build requires --workspace and --runtime')
        workspace = args.workspace.resolve()
        manifest_path = workspace / 'installation.json'
        manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
        actual = hashlib.sha256((workspace / 'game_files/default.xbe').read_bytes()).hexdigest()
        if actual != XBE_SHA256:
            raise ValueError('Installed XBE is not the supported revision')
        build(workspace, args.runtime.resolve())
        manifest['buildStatus'] = 'compiled'
        manifest['executableSha256'] = hashlib.sha256((workspace / 'fifastreet_recomp.exe').read_bytes()).hexdigest()
        write_json(manifest_path, manifest)
        return
    if not args.iso:
        parser.error('--iso is required')
    disc = Disc(args.iso)
    try:
        summary = disc.summary()
        print(json.dumps(summary, indent=2))
        if args.action == 'inspect':
            return
        if not args.workspace or (not args.extract_only and not args.runtime):
            parser.error('install requires --workspace and --runtime (or --extract-only)')
        workspace = args.workspace.resolve()
        if workspace.exists():
            raise ValueError('Choose a NEW workspace; existing directories are never overwritten')
        if not args.extract_only:
            developer_environment()  # Check before a potentially large extraction.
        if shutil.disk_usage(workspace.parent).free < summary['extractedBytes'] + 1024**3:
            raise ValueError('Insufficient free disk space (assets plus 1 GiB build reserve)')
        workspace.mkdir()  # All installation writes remain beneath this chosen directory.
        manifest = dict(summary, sourceIso=str(args.iso.resolve()), buildStatus='extracting')
        write_json(workspace / 'installation.json', manifest)
        manifest['files'] = disc.extract(workspace / 'game_files')
        manifest['buildStatus'] = 'assets-ready'
        shutil.copy2(Path(__file__).with_name('fifa_launch.py'), workspace / 'fifa_launch.py')
        shutil.copy2(Path(__file__).with_name('launch.cmd'), workspace / 'launch.cmd')
        write_json(workspace / 'installation.json', manifest)
        if not args.extract_only:
            build(workspace, args.runtime.resolve())
            manifest['buildStatus'] = 'compiled'
            manifest['executableSha256'] = hashlib.sha256((workspace / 'fifastreet_recomp.exe').read_bytes()).hexdigest()
            write_json(workspace / 'installation.json', manifest)
        print('Prepared:', workspace)
    finally:
        disc.file.close()


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print('Setup failed:', error, file=sys.stderr)
        sys.exit(1)
