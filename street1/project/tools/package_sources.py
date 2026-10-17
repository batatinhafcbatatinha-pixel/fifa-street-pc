"""Create offline source/installer ZIP; never include disc data or binaries."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile


def package(project, runtime, destination):
    files = []
    for folder, prefix, parts in [(project, 'project', ('src', 'tools')),
                                   (runtime, 'xboxrecomp', ('src', 'include', 'cmake', 'LICENSES', 'templates', 'tests/d3d8_smoke'))]:
        for part in parts:
            for source in sorted((folder / part).rglob('*')):
                if source.name in ('nv2a_pb_replay.c', 'nv2a_pb_test.c', 'font_atlas_data.h') or source.name.startswith('menu_pb_'):
                    continue
                if source.is_file() and (part == 'LICENSES' or source.suffix.lower() in ('.c', '.h', '.inc', '.hlsl', '.cmake', '.txt', '.md', '.py', '.cmd', '.0', '.1')):
                    if '__pycache__' not in source.parts:
                        files.append((source, prefix + '/' + source.relative_to(folder).as_posix()))
        for name in ('CMakeLists.txt', 'LICENSE', 'NOTICE', 'LICENSE.xboxrecomp'):
            if (folder / name).is_file():
                files.append((folder / name, prefix + '/' + name))
    if not (project / 'src/main.c').is_file() or not (runtime / 'src/kernel/kernel_bridge.c').is_file():
        raise ValueError('Project/runtime source tree incomplete')
    manifest = []
    with zipfile.ZipFile(destination, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for source, name in files:
            payload = source.read_bytes()
            if name == 'xboxrecomp/src/nv2a/CMakeLists.txt':
                # This title does not reference the optional replay/test entry points.
                # Remove their build entries in the package only, retaining the local runtime.
                payload = b'\n'.join(line for line in payload.split(b'\n')
                                     if b'nv2a_pb_replay.c' not in line and b'nv2a_pb_test.c' not in line)
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, payload)
            manifest.append(dict(path=name, bytes=len(payload), sha256=hashlib.sha256(payload).hexdigest()))
        archive.writestr('Install FIFA Street.cmd', '@echo off\r\ncall "%~dp0project\\tools\\Install FIFA Street.cmd"\r\n')
        archive.writestr('LEIA-ME.txt', 'FIFA Street - instalador local experimental\n\nExtraia TODO este ZIP. Abra Install FIFA Street.cmd.\nUse sua propria ISO do Xbox Original. Nenhuma ISO ou dado do jogo acompanha este pacote.\nPython 3 com Tkinter e ferramentas C++ (Visual Studio Build Tools, Windows SDK, LLVM, CMake, Ninja) sao requisitos locais.\nO instalador detecta ferramentas existentes; nao baixa ou instala dependencias globais.\nDetalhes: project/tools/INSTALL.md\n')
        archive.writestr('source-manifest.json', json.dumps(manifest, indent=2))
        archive.writestr('PACKAGING-NOTES.txt', 'Source snapshot, not a pristine upstream archive.\nNV2A replay/test compilation entries and their sources are omitted for this title.\nmenu_pb_* and font_atlas_data.h are omitted; they were zero-data placeholders in the inspected tree.\nThe font include is guarded by GAME_HAS_FONT_ATLAS, which this FIFA Street target does not define.\nDSP interpreter .inc source and license files, plus runtime templates, are retained.\nGenerated game C sources contain translated instructions/constants; this package is not a claim of independent source authorship or redistribution rights.\nNo ISO/XBE files or extracted disc resource files are included.\n')
    with zipfile.ZipFile(destination) as archive:
        if archive.testzip() is not None:
            raise ValueError('ZIP integrity check failed')
    print(f'{destination}: {len(files)} source files, {destination.stat().st_size} bytes; ZIP CRC verified')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    arguments = parser.parse_args()
    package(arguments.project.resolve(), arguments.runtime.resolve(), arguments.output.resolve())
