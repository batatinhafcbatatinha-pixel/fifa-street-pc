"""Dedicated local launcher. Run from a prepared installation; no downloads."""
import json
import os
from pathlib import Path
import subprocess
import tkinter as tk
from tkinter import ttk, messagebox

ROOT = Path(__file__).resolve().parent
# 100% native: no model of any console device (NV2A, APU/DSP, AC97, USB, flash).
PROFILE = {key: '1' for key in ('RECOMP_NO_HW', 'RECOMP_HLE_PRIMARY', 'RECOMP_HLE_NOGPU',
    'RECOMP_DS_NATIVE', 'RECOMP_INPUT_NATIVE', 'RECOMP_VBLANK', 'RECOMP_TITLE_KEVENTS',
    'RECOMP_GUEST_LOCK', 'RECOMP_GUEST_SMP', 'RECOMP_KEYBOARD')}


def main():
    window = tk.Tk()
    window.title('FIFA Street — local PC port')
    window.geometry('570x360')
    window.resizable(False, False)
    manifest = json.loads((ROOT / 'installation.json').read_text(encoding='utf-8'))
    config_path = ROOT / 'launcher.json'
    config = json.loads(config_path.read_text(encoding='utf-8')) if config_path.exists() else {}
    language = tk.StringVar(value=config.get('gameLanguage', 'English'))
    ui_language = tk.StringVar(value=config.get('interfaceLanguage', 'Português'))
    available = manifest.get('languageResources', [])
    if language.get() not in available and available:
        language.set(available[0])
    frame = ttk.Frame(window, padding=22)
    frame.pack(fill='both', expand=True)
    ttk.Label(frame, text='FIFA Street', font=('Segoe UI', 22, 'bold')).pack(anchor='w')
    ttk.Label(frame, text='Xbox Original · Title ID 45410072 · port experimental').pack(anchor='w', pady=(0, 15))
    ui_label = ttk.Label(frame)
    ui_label.pack(anchor='w')
    ui = ttk.Combobox(frame, textvariable=ui_language, values=['Português', 'English'], state='readonly')
    ui.pack(anchor='w')
    game_label = ttk.Label(frame)
    game_label.pack(anchor='w', pady=(10, 0))
    ttk.Combobox(frame, textvariable=language, values=available, state='readonly').pack(anchor='w')
    status = ttk.Label(frame, wraplength=520)
    status.pack(anchor='w', pady=12)
    process = [None]
    streams = []

    def save():
        config_path.write_text(json.dumps(dict(gameLanguage=language.get(), interfaceLanguage=ui_language.get()), indent=2), encoding='utf-8')

    def translate(*_):
        pt = ui_language.get() == 'Português'
        ui_label.config(text='Idioma do launcher' if pt else 'Launcher language')
        game_label.config(text='Idioma do jogo (recursos presentes na ISO)' if pt else 'Game language (resources present in ISO)')
        status.config(text=('Versão experimental para PC. Escolha o idioma e inicie o jogo.' if pt else
                           'Experimental PC version. Choose a language and start the game.'))
        button.config(text='Iniciar jogo' if pt else 'Start game')

    def poll():
        if process[0] is None:
            return
        result = process[0].poll()
        if result is None:
            window.after(500, poll)
            return
        for stream in streams:
            stream.close()
        streams.clear()
        process[0] = None
        button.config(state='normal')
        status.config(text=f'Exit code: {result}. Logs: {ROOT / "logs"}')

    def launch():
        executable = ROOT / 'fifastreet_recomp.exe'
        if not executable.is_file():
            messagebox.showerror('FIFA Street', 'Executable missing. Complete the local source build first.')
            return
        if language.get() not in available:
            messagebox.showerror('FIFA Street', 'No verified language resources found in this installation.')
            return
        save()
        env = {k: v for k, v in os.environ.items() if not k.startswith('RECOMP_')}
        env.update(PROFILE)
        env['RECOMP_LANGUAGE'] = {'English': '1', 'Spanish': '5'}[language.get()]
        logs = ROOT / 'logs'
        logs.mkdir(exist_ok=True)
        import datetime
        stamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f')
        streams.extend([(logs / (stamp + '.stdout.log')).open('wb'), (logs / (stamp + '.stderr.log')).open('wb')])
        try:
            process[0] = subprocess.Popen([str(executable)], cwd=ROOT, env=env,
                stdout=streams[0], stderr=streams[1], creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        except OSError as error:
            for stream in streams:
                stream.close()
            streams.clear()
            messagebox.showerror('FIFA Street', str(error))
            return
        button.config(state='disabled')
        status.config(text='FIFA Street running. Logs: ' + str(logs))
        window.after(500, poll)

    button = ttk.Button(frame, command=launch)
    button.pack(anchor='w')
    ui.bind('<<ComboboxSelected>>', translate)
    translate()
    window.mainloop()


if __name__ == '__main__':
    main()
