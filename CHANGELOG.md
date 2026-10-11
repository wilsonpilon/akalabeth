# Changelog

Formato baseado em [Keep a Changelog](https://keepachangelog.com/). Versões em [SemVer](https://semver.org/).

## [0.4.0] — 2026-10-11

> ⚠️ **A música ainda precisa de refinamento:** pode engasgar em alguns momentos (principalmente ao andar na masmorra).

### Adicionado
- **Uso da RAM mapeada (256 KB) do MSX-DOS 2.** Novo módulo `ramstore` aloca segmentos de 16 KB e copia dados segmento ↔ RAM/VRAM
  com a página 2 trocada só durante a cópia (interrupções desligadas; a rotina fica na página 0). O programa ocupa as 4 páginas da CPU,
  por isso os segmentos nunca ficam mapeados.
- **`AKALABET.DAT`** (112 KB, gerado por `tools/gen_assets.sh`): 3 telas pixel art de 256×212 (abertura, ajuda, vitória) e o segmento de música.
  Carregado do disco para a RAM mapeada na partida (`LOADING...`); se faltar o arquivo, o mapper ou memória, o jogo segue sem os extras.
- **Tela de abertura** (castelo ao luar) ao iniciar; **tela de ajuda** (tecla `H`, também no título); **tela de vitória** (aurora sobre o castelo) ao ser sagrado Lord, com tema novo (`VICTORY`).
- **Músicas longas** (24 compassos, 30–50 s; antes 8 compassos): cada tema ganhou uma seção B e volta à A. As trilhas são lidas aos poucos
  da RAM mapeada para um anel de 32 bytes por voz (`music_pump`), então a interrupção só lê RAM comum.
  Sem RAM mapeada, tocam as versões curtas embutidas no código.
- `tools/gen_art.js` (pixel art em JS, com paleta do jogo e prévia PNG), `tools/gen_assets.sh`.

### Corrigido
- Música "enroscando" ao andar na masmorra: os efeitos só de ruído (passo, golpe, erro) substituíam a melodia no canal A; agora o ruído é somado e a melodia continua.

### Alterado
- Salvar/carregar sem o buffer de 1248 bytes: o arquivo (mesmo formato `AKS1`) é gravado/lido em blocos direto das variáveis do jogo.
  Motivo: o **TPA do MSX-DOS 2 termina em 0xDB06** (≈55 KB para código + dados + pilha); a memória do programa está praticamente esgotada.
- `AUTOEXEC.BAT` do disco executa `RAMDISK /D` antes do jogo: com o RAM disk padrão do DOS 2, quase nenhum segmento fica livre.
- Título: dicas de teclas `ANY KEY:NEW / L: LOAD / H: HELP`.
- Versão 0.4.0.

### Medido e verificado (11/out/2026, openMSX, NMS 8250 com 256 KB)
- Abertura, ajuda (título e jogo), vitória, 5 músicas (40+ s sem engasgo, transição masmorra/mundo), salvar/carregar após a mudança de formato de E/S.
- Máquina de 128 KB (NMS 8250 original, 1 segmento livre): o jogo inicia direto no título, com as músicas curtas.

## [0.3.0] — 2026-10-11

### Alterado (desempenho)
- Passo no mundo: ~19 → **~15 quadros** (20 ms); passo na masmorra: ~33 → **~27 quadros**; giro na masmorra: 26 → 20.
  - **Troca de páginas** (R#2) na masmorra: desenha na página oculta e alterna, sem cópia de 256×160.
  - HUD duplicado nas duas páginas (linhas 160+ em ambas).
  - Atlas de tiles movido para a página 0 (linhas 212–255, tiles cortados em 44 linhas); sprites desativados (`VDP_DisableSprite`), pois suas tabelas ficam nessa área.
  - Texto por escrita direta na VRAM (`blit_glyph` em assembly), em vez de ~5 ms/caractere via módulo Print.
  - Motor de som com sombra dos registradores do PSG: só escreve o que mudou.
- Perfil por fase do turno (`g_tparts`) para depuração.

### Corrigido
- Título: o logotipo grande "AKALABETH" e "WORLD OF DOOM" não apareciam (eram desenhados na página oculta depois da troca de páginas). O texto agora é desenhado na página exibida (`gfx_target_front`).

### Medido e verificado (11/out/2026, openMSX, MSX-DOS 2)
- Passo no mundo **15–16 quadros** (v0.2.0: 19–20); frente na masmorra **27–28** (33–34); giro **20** (26).
- Verificados na v0.3.0: mundo (tiles, texto, HUD), masmorra com troca de páginas e HUD duplicado, minimapa, salvar/carregar, tela de morte e tela da missão do castelo.
- Observação: os tiles do mundo passaram a ter 44 linhas (a base de montanhas e casas ficou mais curta).
- A sombra do PSG **não** foi verificada contra a música que engasga.

## [0.2.0] — 2026-10-10

### Adicionado
- **Salvar e carregar** (`W`/`L`, e `L` na tela de título) via handles do MSX-DOS 2; arquivo `AKALABET.SAV` (1248 bytes). Avisos "NO SAVE" e "NEEDS DOS2".
- **Minimapa da masmorra** com neblina de exploração, portas, baús, escadas, buracos, monstros visíveis e direção do jogador.
- **Música** em 3 vozes (4 temas originais: título, mundo, masmorra, corte) e motor de **efeitos** pelo PSG executados na interrupção do VDP. Tecla `M` liga/desliga.
- Versão exibida na tela de título (`V0.2.0`).
- Modos de depuração `AK_DEBUG` 5–8 (armadilha, mímico, amuleto, feitiço KILL).
- Scripts do harness de testes (`tools/test/`) e ferramenta de disco `tools/mkdisk.*` (`msxdisk.exe`).
- Documentação: manual do jogador, outline técnico com status e roadmap.

### Alterado (desempenho)
- Passo no mundo: **~1,3 s → ~0,4 s**; passo na masmorra: **~1,3 s → ~0,6 s**.
  - HUD reescreve só o que mudou.
  - A espera fixa de 0,5 s por passo na masmorra só ocorre quando há mensagem para ler.
  - Tiles do mundo desenhados uma vez num atlas da VRAM e copiados; painéis laterais e minimapa persistentes.
  - Paredes da masmorra preenchidas com colunas `HMMV` de 4 e 8 px; detalhes distantes (juntas, arestas, sombras) omitidos.
- Andamento da música ~18 % mais rápido do que na primeira versão do motor de som.

### Corrigido
- Chamadas à BIOS travavam em MSX-DOS (página 0 é RAM): agora via `CALSLT`.
- `Draw_Line` desenhava na página errada (`UY` de 8 bits).
- Saída (`ESC`) restaura texto, paleta e o gancho de interrupção.
- Telas de castelo ("missão cumprida") e de morte (lápide) com sobreposição de texto.

### Problemas conhecidos
- A música às vezes engasga (investigação pendente — ver `docs/OUTLINE.md`).
- Joystick, máquina de 60 Hz com o jogo rodando e hardware real ainda não testados.

## [0.1.0] — 2026-10-10 (interna)

- Primeiro porte jogável a partir do `basic.bas` de Nanochess: criação de personagem, loja, mundo, masmorra 3D preenchida com 10 monstros em vetores coloridos, combate, armadilhas, escadas, baús, amuleto mágico, castelo e missões, tela de morte.
- Efeitos sonoros simples no PSG; disco de 720 KB com MSX-DOS 2.
