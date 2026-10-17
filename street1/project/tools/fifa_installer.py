"""Graphical offline installer for a user's own original-Xbox FIFA Street ISO."""
import json
import os
from pathlib import Path
import queue
import re
import subprocess
import sys
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

TOOLS = Path(__file__).resolve().parent
PROJECT = TOOLS.parent


def find_runtime():
    for parent in (PROJECT, *PROJECT.parents):
        for candidate in (parent / 'xboxrecomp', parent / 'runtime'):
            if (candidate / 'src/kernel/kernel_bridge.c').is_file():
                return str(candidate)
    return ''


def main():
    app = tk.Tk()
    app.title('Instalar FIFA Street para PC')
    app.geometry('780x650')
    app.minsize(650, 500)
    pane = ttk.Frame(app, padding=20)
    pane.pack(fill='both', expand=True)
    ttk.Label(pane, text='FIFA Street para PC', font=('Segoe UI', 21, 'bold')).pack(anchor='w')
    ttk.Label(pane, text='Instalação local · versão experimental', font=('Segoe UI', 11)).pack(anchor='w', pady=(0, 8))
    ttk.Label(pane, text='Escolha sua ISO do Xbox Original. O instalador valida os arquivos, extrai os dados e compila o jogo neste computador. Nenhum arquivo do jogo é incluído neste pacote.', wraplength=720).pack(anchor='w', pady=(0, 10))
    iso, destination, runtime = tk.StringVar(), tk.StringVar(), tk.StringVar(value=find_runtime())

    def row(title, variable, choose):
        ttk.Label(pane, text=title).pack(anchor='w', pady=(5, 0))
        frame = ttk.Frame(pane)
        frame.pack(fill='x')
        ttk.Entry(frame, textvariable=variable).pack(side='left', fill='x', expand=True)
        ttk.Button(frame, text='Escolher…', command=choose).pack(side='right', padx=(6, 0))

    def choose_iso():
        selected = filedialog.askopenfilename(title='Sua ISO do FIFA Street (Xbox Original)', filetypes=[('Imagem ISO', '*.iso')])
        if selected:
            iso.set(selected)

    def choose_folder():
        selected = filedialog.askdirectory(title='Pasta onde criar uma nova instalação FIFAStreet-PC')
        if selected:
            destination.set(str(Path(selected) / 'FIFAStreet-PC'))

    def choose_runtime():
        selected = filedialog.askdirectory(title='Código-fonte xboxrecomp')
        if selected:
            runtime.set(selected)

    row('ISO do jogo', iso, choose_iso)
    row('Nova pasta da instalação', destination, choose_folder)
    row('Código do port (detectado automaticamente quando incluído)', runtime, choose_runtime)
    status = tk.StringVar(value='Pronto para validar sua ISO. C++ Build Tools, Windows SDK, LLVM, CMake e Ninja devem estar instalados.')
    ttk.Label(pane, textvariable=status, wraplength=720).pack(anchor='w', pady=12)
    progress = ttk.Progressbar(pane, mode='determinate', maximum=100)
    progress.pack(fill='x')
    output = tk.Text(pane, height=12, wrap='word', state='disabled')
    output.pack(fill='both', expand=True, pady=10)
    events = queue.Queue()
    busy = [False]
    transcript = []

    def worker(action, image, folder, source):
        command = [sys.executable, '-u', str(TOOLS / 'fifa_setup.py'), action]
        if action != 'build':
            command += ['--iso', image]
        if action != 'inspect':
            command += ['--workspace', folder, '--runtime', source]
        try:
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                text=True, encoding='utf-8', errors='replace',
                env={**os.environ, 'PYTHONIOENCODING': 'utf-8'},
                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            for line in process.stdout:
                events.put(('line', line))
            result = process.wait()
            events.put(('done', (result, action, folder)))
        except OSError as error:
            events.put(('line', str(error) + '\n'))
            events.put(('done', (1, action, folder)))

    def start(action):
        if busy[0]:
            return
        if action != 'build' and not Path(iso.get()).is_file():
            messagebox.showerror('Selecionar ISO', 'Selecione uma ISO existente do FIFA Street para Xbox Original.')
            return
        if action != 'inspect' and (not destination.get() or not (Path(runtime.get()) / 'CMakeLists.txt').is_file()):
            messagebox.showerror('Preparar instalação', 'Escolha a pasta de instalação e o código-fonte xboxrecomp.')
            return
        if action == 'install' and Path(destination.get()).exists():
            messagebox.showerror('Pasta já existe', 'Escolha uma pasta nova. Para uma extração concluída anteriormente, use Retomar compilação.')
            return
        busy[0] = True
        for button in controls:
            button.config(state='disabled')
        progress['value'] = 0
        progress.config(mode='indeterminate')
        progress.start()
        status.set('Validando e preparando… A janela continuará respondendo durante o trabalho.')
        transcript.clear()
        threading.Thread(target=worker, args=(action, iso.get(), destination.get(), runtime.get()), daemon=True).start()

    def pump():
        while not events.empty():
            kind, value = events.get_nowait()
            if kind == 'line':
                transcript.append(value)
                output.config(state='normal')
                output.insert('end', value)
                output.see('end')
                output.config(state='disabled')
                count = re.search(r'Extracted (\d+)/(\d+):', value)
                if count:
                    progress.stop()
                    progress.config(mode='determinate')
                    progress['value'] = 100 * int(count[1]) / int(count[2])
                    status.set(f'Extraindo arquivos: {count[1]} de {count[2]}')
                elif 'Compiling FIFA Street' in value or 'Configuring local' in value:
                    progress.config(mode='indeterminate')
                    progress.start()
                    status.set('Compilando no seu computador. Esta etapa pode levar vários minutos.')
                elif re.match(r'^\[\d+/\d+\]', value):
                    status.set('Compilando: ' + value.split(']', 1)[0] + ']')
            else:
                code, action, folder = value
                busy[0] = False
                progress.stop()
                progress.config(mode='determinate')
                progress['value'] = 100 if code == 0 else 0
                for button in controls:
                    button.config(state='normal')
                if action != 'inspect' and (Path(folder) / 'installation.json').is_file():
                    (Path(folder) / 'setup.log').write_text(''.join(transcript), encoding='utf-8')
                if code:
                    status.set('Não foi possível concluir. Consulte o detalhe abaixo; os arquivos existentes foram preservados.')
                elif action == 'inspect':
                    status.set('ISO compatível validada. Agora você pode instalar.')
                else:
                    status.set('Instalação concluída! Abra launch.cmd na pasta escolhida para jogar.')
                    messagebox.showinfo('Pronto', 'Port compilado localmente.\nAbra launch.cmd em:\n' + folder)
        app.after(100, pump)

    buttons = ttk.Frame(pane)
    buttons.pack(fill='x')
    controls = []
    for label, action in [('Validar ISO', 'inspect'), ('Extrair e instalar', 'install'), ('Retomar compilação', 'build')]:
        button = ttk.Button(buttons, text=label, command=lambda a=action: start(a))
        button.pack(side='left', padx=(0, 8))
        controls.append(button)

    def close():
        if busy[0]:
            messagebox.showinfo('Trabalho em andamento', 'Aguarde a conclusão da extração ou compilação antes de fechar esta janela.')
        else:
            app.destroy()

    app.protocol('WM_DELETE_WINDOW', close)
    pump()
    app.mainloop()


if __name__ == '__main__':
    main()
