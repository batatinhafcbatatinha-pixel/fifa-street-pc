# FIFA Street: instalação local a partir da sua ISO

Este instalador de código-fonte prepara o port experimental do **Xbox Original**. Não é um instalador de jogos Xbox 360. Não inclui ISO, XBE, músicas, vídeos ou outros dados do jogo. Não baixa dependências nem altera o registro ou configurações globais do Windows.

Requisitos: Windows x64, Python 3.10+ com Tkinter, Visual Studio C++ Build Tools e Windows SDK, LLVM clang-cl, CMake 3.20+ e Ninja. Tenha o projeto e o código do runtime xboxrecomp já disponíveis; o script não precisa de rede.

## Instalação pela janela

Abra **Install FIFA Street.cmd**. Selecione sua ISO e uma pasta nova; clique **Validar ISO**, depois **Extrair e instalar**. O código do runtime é localizado automaticamente na árvore local ou no pacote completo. A janela permanece disponível e mostra andamento da extração e compilação. Uma instalação concluída ganha `launch.cmd`; os detalhes ficam em `setup.log`. Se a compilação falhar após a extração, corrija o requisito indicado e use **Retomar compilação** com a mesma pasta.

O instalador prepara o ambiente Visual Studio x64 por `vswhere`/`vcvars64` e procura ferramentas no PATH, na instalação Visual Studio, nos caminhos locais LLVM/CMake e em `recomp-python/Scripts` quando presente. Não é necessário abrir um terminal especial. O pacote não redistribui Python ou compiladores; a detecção reutiliza instalações locais.

## Interface de linha de comando opcional

Inspecionar a ISO sem extrair ou gravar arquivos:

```powershell
python tools/fifa_setup.py inspect --iso "D:\Xbox 360\FIFA Street (USA)\FIFA Street (USA).iso"
```

Extrair os dados próprios e compilar localmente (escolha uma pasta NOVA cujo diretório pai já exista):

```powershell
python tools/fifa_setup.py install --iso "D:\Xbox 360\FIFA Street (USA)\FIFA Street (USA).iso" --workspace "C:\Games\FIFAStreet-local" --runtime "C:\Sources\xboxrecomp"
```

Para apenas preparar dados, substitua `--runtime ...` por `--extract-only`. Para compilar depois ou recompilar, execute:

```powershell
python tools/fifa_setup.py build --workspace "C:\Games\FIFAStreet-local" --runtime "C:\Sources\xboxrecomp"
```

Execute `launch.cmd` na pasta instalada. O launcher distingue idioma de sua interface (português/inglês) e idioma solicitado ao jogo (inglês/espanhol, somente quando os arquivos correspondentes estão presentes). O contrato do runtime é `RECOMP_LANGUAGE=1` ou `5`; a resposta visual do jogo ainda exige validação. Não há tradução portuguesa dos recursos do jogo neste pacote. O launcher não expõe Vulkan ou renderização interna 4K como recursos prontos.

O instalador valida TitleID **45410072** e o SHA-256 exato do XBE compatível com o código recompilado. Outras revisões são rejeitadas mesmo que tenham o mesmo TitleID. Cada arquivo extraído ganha SHA-256 no `installation.json`. Falhas de extração mantêm a pasta parcial e o estado `extracting`; não sobrescreva nem misture instalações. Escolha outra pasta ou remova conscientemente a pasta parcial. O comando `build` permite recuperar uma compilação que falhou após a extração.

Todas as saídas de extração, compilação, launcher e logs ficam na pasta escolhida. O código de origem é lido no local em que este script está; mantenha a árvore de projeto acessível para recompilar. A compilação usa quatro tarefas e gera RelWithDebInfo. O manifesto registra o hash do executável concluído; isto não promete binários idênticos entre versões de compiladores.

O launcher utiliza o perfil de runtime confirmado pelo projeto, limpa opções RECOMP herdadas e não aplica watchdog ou atalhos de diagnóstico ao jogo. Fechar a janela do launcher não encerra à força o jogo. Os logs de cada partida ficam em `logs`.
