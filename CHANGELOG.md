# Changelog

Formato baseado em [Keep a Changelog](https://keepachangelog.com/). Versões em [SemVer](https://semver.org/).

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
