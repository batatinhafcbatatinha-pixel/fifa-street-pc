# FIFA Street — port experimental para Windows

Este projeto hospeda uma recompilação estática do FIFA Street de Xbox original, título `45410072`, entrada `0015BA1E`. A construção atual usa a GPU física pelo Direct3D 11 e já renderizou o logo e o vídeo de abertura. Ainda não é um port jogável: não foram verificados partida, áudio correto nem salvar/carregar, e há problemas de sincronização USB, imagem e desempenho. As evidências de menu abaixo pertencem a construções anteriores e não comprovam que a construção atual chega ao menu.

## Construção local atual — 10 de outubro

Use `build-fifa-local.cmd release` na raiz do workspace e `street1/Jogar-FIFA-Street.cmd` para abrir a versão experimental. O executável ativo é `street1/build-fixed-release/fifastreet_recomp.exe`. O teste de boot usa Direct3D 11 por padrão, registra o hash e preserva os resultados em `street1/runs`.

O backend de rasterização passou 1.452 verificações na RTX 3050 Laptop GPU. Um teste integrado registrou 1.771 dispatches reais sem falhas nem fallback até o vídeo de abertura. Há leitura de volta por triângulo e trabalho de CPU em vertex programs e texturas; isso ainda limita o desempenho. Ver `../GPU_BACKEND.md` e `../CONTINUACAO.md` para detalhes e falhas atuais.

## Evidências de construções anteriores

- Executável Windows x64 compilado com Clang 22.1.2 e o Windows SDK.
- XBE carregado, onze seções mapeadas e 123 importações vinculadas ao runtime.
- Inicialização de threads e eventos executada; corrigida a espera em múltiplos eventos criados pelo jogo.
- Comandos NV2A consumidos; semáforos DMA escritos nos endereços configurados pelo miniport.
- Tela de carregamento real renderizada, com imagem parcialmente incorreta.
- Teste de 100 segundos passou do carregamento e mostrou o aviso de licença e a tela de título; terminou pelo limite de tempo, sem a falha anterior no frontend.
- Nove destinos de chamadas indiretas ausentes foram registrados em execução e usados como sementes de análise, incluindo funções do vídeo de abertura, da enumeração USB e da cópia de relatórios de controle.
- Controlador USB ligado à inicialização e ao tratamento de acessos de hardware do executável; o teste de 150 segundos enumerou o controle e entregou 233 relatórios de entrada.
- No teste de 180 segundos com as nove funções recuperadas, os comandos Start e A foram enviados pela interface USB, e Start na tela de título abriu o menu de opções de salvamento. Ainda não foi validada uma partida.
- No teste de 300 segundos, A avançou das opções de salvamento para a confirmação de salvar e, depois, ao menu principal. A captura está em `../fifa-street-menu-principal.bmp`. Não houve avisos de chamadas sem resolução nem falha de acesso à memória nesse teste. O processo terminou pelo limite de tempo (código 3), não por saída normal do jogo. Desempenho observado no frontend: aproximadamente 0,5 a 2 quadros por segundo no executável de diagnóstico.
- Chamadas diretas a funções sem corpo agora emitem um aviso quando executadas. A geração ainda tem 62 desses destinos; a contagem não significa que todos sejam alcançados pelo jogo.

A tela de carregamento só foi alcançada com um teste que confirma artificialmente a conclusão dos comandos dos efeitos de áudio. Essa opção é um diagnóstico, não implementação do DSP. O runtime atual faz passagem direta do áudio e não executa esses efeitos. Não usar esse teste como prova de áudio correto.

## Estrutura e reprodução

O toolkit está em `../../work/xboxrecomp`, revisão `193e2995ffaec871bb3be43a43bd66aa5fccfeb1`. As alterações locais estão em `xboxrecomp-runtime.patch`. Os arquivos do jogo foram extraídos para `../../work/street1/game_files`, preservando as imagens originais.

Os scripts `../../work/build-fifa.cmd` e `../../work/build-fifa-release.cmd` produzem executáveis de diagnóstico e otimizado, respectivamente. A execução precisa usar `../../work/street1` como diretório de trabalho. O script `../../work/test-fifa-boot.ps1` registra resultado, hash do executável, opções de execução, saída e erros, preservando cada teste em `../../work/street1/runs`. `../../work/regenerate-fifa.ps1` repete a análise e a geração a partir do XBE e das sementes observadas, conferindo cada etapa antes de atualizar o código.

REA está instalado e conectado; a análise do XBE usa a cadeia específica de Xbox e o Ghidra diretamente, porque o perfil Windows do REA instalado não aceita XBE como aplicação PE.

## Trabalho ainda necessário

Implementar o comportamento necessário do DSP; corrigir a imagem, desempenho e serviços de hardware alcançados; validar a seleção de times, início e término de partida, áudio e salvar/carregar. Depois integrar as opções reais do launcher solicitadas e repetir o processo para FIFA Street 2.

Referências: [xboxrecomp](https://github.com/sp00nznet/xboxrecomp), [OpenCE](https://github.com/OpenCommunityEdition/OpenCE), [REA](https://github.com/morluto/rea), [NV2A no xemu](https://github.com/xemu-project/xemu/tree/master/hw/xbox/nv2a).
