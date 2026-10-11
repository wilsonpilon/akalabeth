# Akalabeth MSX2 — Outline técnico, status e pendências

**Versão atual: 0.4.0** (11/out/2026) · Alvo: MSX2 com 256 KB de RAM / 64 KB de VRAM e MSX-DOS 2 (roda com menos, sem os extras) · SDCC 4.6 + MSXgl

> Este documento é o ponto de retomada do projeto: o que existe, como funciona, o que foi testado,
> o que está pendente e o que melhorar. Atualize a seção **2. Onde estamos** a cada release.

---

## 1. Visão geral

Porte do *Akalabeth* (Richard Garriott, 1979) para MSX2, a partir do porte GW-BASIC de Nanochess (2019).
O programa original em BASIC (`basic.bas`, ~400 linhas) é a referência de regras, textos e vetores dos monstros.
O objetivo de longo prazo é: (1) este porte MSX2 usando mais recursos da máquina (VDP, PSG, DOS 2, mapper); (2) um porte para PC em Go reaproveitando as regras e os dados.

### Repositório

```
E:\akalabeth\
├── README.md · CHANGELOG.md
├── docs\                    MANUAL.md, OUTLINE.md (este), releases\, images\
├── MSXgl\                   biblioteca MSXgl (NÃO versionada; é dependência externa)
│   └── projects\akalabeth\  << o código do jogo (versionado)
│       ├── akalabeth.c      regras, mundo, masmorra, combate, loja, castelo, títulos
│       ├── gfx.c            paleta, primitivas, renderizador 3D, mundo, minimapa da masmorra
│       ├── ui.c             console, HUD, entrada, motor de som (música em fluxo + efeitos), RNG
│       ├── ramstore.c/.h    RAM mapeada do DOS 2 (segmentos, cópia segmento <-> RAM/VRAM)
│       ├── akalabeth.h      constantes, paleta, estado compartilhado, versão, AK_DEBUG
│       ├── content\         data_gen.h e music_gen.h (GERADOS) e AKALABET.DAT (arte + música, GERADO)
│       └── tools\           gen_data.js, gen_music.js, gen_art.js, gen_assets.sh, mkdisk.bat/.sh, test\ (harness openMSX)
├── dist\                    saída (akalabeth.dsk, AKALABET.COM) — não versionada
└── msxdisk.exe              utilitário de disco (terceiros) — não versionado
```

### Módulos

| Arquivo | Responsabilidade |
| --- | --- |
| `akalabeth.c` | Estado do jogo, geração do mundo (21×21) e das masmorras (11×11, semente = nº da sorte + entrada + andar), IA dos monstros, combate, loja, castelo/missões, salvar/carregar (handles do DOS 2), título e criação de personagem, `main`. |
| `gfx.c` | Paleta de 16 cores (de 512), primitivas com troca de páginas (R#2) e HUD duplicado nas duas páginas, **escritor de texto direto na VRAM** (`blit_glyph`, em assembly), renderizador 3D preenchido, tiles do mundo (atlas na VRAM), minimapas. |
| `ui.c` | Console de mensagens, HUD incremental, teclado (CHGET via CALSLT; sem joystick desde a v0.4.1), **motor de som** na interrupção (com sombra dos registros do PSG: só escreve o que mudou), gerador aleatório (xorshift16). |
| `tools/gen_data.js` | Lê `basic.bas` e gera `content/data_gen.h`: polilinhas dos monstros e tabelas de perspectiva reparametrizadas de 280 para 256 px. |
| `ramstore.c` | `rs_init` (aloca segmentos), `rs_put`/`rs_get`/`rs_to_vram` (via `rs_exec`, em assembly, que troca a página 2 pela porta 0xFE com `DI`). Fica na página 0 (módulo linkado primeiro). |
| `tools/gen_music.js` | Notação textual (`A4:3 C#5:1 R:2`) → `content/music_gen.h` (versões curtas de reserva no código) e `content/music.seg` (músicas completas). |
| `tools/gen_art.js` | Telas pixel art 256×212 com a paleta do jogo → `content/art.seg` (e prévia PNG). `tools/gen_assets.sh` junta tudo em `AKALABET.DAT`. |

### Mapa de VRAM (SCREEN 5)

| Região | Uso |
| --- | --- |
| Página 0, linhas 0–211 | Tela A. Na masmorra é uma das duas telas completas (3D + HUD). |
| Página 0, linhas 212–255 | **Atlas de tiles** do mundo (montanha, árvores, cidade, masmorra, castelo), 50×**44** cada (cortados em 44 linhas). |
| Página 1, linhas 256–467 | Tela B (3D + HUD). O mundo desenha os tiles aqui e copia 150×150 por `HMMM`. |
| Página 1, linhas 470–502 | Imagem do **minimapa da masmorra** (34×33), atualizada incrementalmente. |

A masmorra desenha na página oculta e alterna a página exibida (`R#2`), sem cópia. Como as tabelas de sprites do SCREEN 5
caem em página 0, linhas 212–255 (onde está o atlas), os **sprites são desativados** (`VDP_DisableSprite`); o jogo não usa sprites.

### Decisões de projeto importantes

- **Chamadas à BIOS:** em MSX-DOS a página 0 é RAM, então qualquer chamada direta à BIOS trava. Tudo passa por `CALSLT` (`bios_call` em `ui.c`).
- **`VDP_UNIT_U16`** no `msxgl_config.h`: com 8 bits em `UY` o `Draw_Line` truncava `y + 256`.
- **Som na interrupção:** gancho em `H.TIMI` (0FD9Fh); só a rotina de interrupção escreve no PSG. Andamento compensado em 60 Hz (pula 1 em cada 6 ticks).
- **Texto direto na VRAM:** o módulo Print do MSXgl custava ~5 ms por caractere. `blit_glyph` (assembly) escreve as 8 linhas do glifo 6×8 como 3 bytes por linha (todo texto começa em x par). O HUD ainda só reescreve o que mudou e o mundo só reescreve X/Y/nome quando mudam.
- **Troca de páginas na masmorra:** desenha-se na página oculta e troca-se `R#2`. O HUD (linhas 160+) é desenhado **nas duas páginas** (`gfx_rect`/`hline`/`vline`/texto duplicam sozinhos quando `y >= 160`).
- **Sombra do PSG:** `psg_w` só escreve o registrador se o valor mudou (menos tempo na interrupção).
- **Salvar/carregar** só em MSX-DOS 2 (handles). Em MSX-DOS 1 o jogo avisa.
- **Mapas ≠ GW-BASIC:** o gerador aleatório é outro; o número da sorte reproduz o **próprio** mapa deste porte.

### RAM mapeada e orçamento de memória (v0.4.0)

- **Ela não acelera o desenho:** o gargalo é CPU e VDP. Serve para guardar conteúdo (telas, músicas) sem esperar o disco.
- O `.COM` ocupa as 4 páginas da CPU (código 0x0100–0xCE55, dados até ≈0xD79C), então nenhum segmento pode ficar mapeado.
  `rs_exec` roda na página 0 com `DI`, lê o segmento atual da página 2 (`GET_P2` da tabela de saltos do mapper), escreve na porta `0xFE`,
  copia (`LDIR`, `OTIR` para a VRAM) e restaura. As rotinas `PUT_Pn` do DOS **não** servem: podem reabilitar interrupções com a página trocada.
- **TPA termina em 0xDB06** (`*(u16*)0x0006`): código + dados + pilha precisam caber em ≈55 KB. Na v0.4.0 sobram ≈900 bytes de pilha.
  Por isso o salvar/carregar não usa buffer, o `AKALABET.DAT` é lido por um buffer de 256 bytes emprestado do mapa do mundo (ainda vazio na partida) e os anéis de música têm 32 bytes por voz.
  **Qualquer novo dado global precisa ser compensado em outro lugar.** Candidatos: remover perfis (`g_prof`, `g_tparts`), compilar partes para tamanho, mover código frio.
- O DOS 2 cria um **RAM disk** que consome quase todo o mapper (no NMS 8250 de 128 KB sobra 1 segmento): o `AUTOEXEC.BAT` executa `RAMDISK /D`.
  Em 128 KB mesmo sem RAM disk faltam segmentos (o jogo pede 7 + 2 de folga) e o jogo segue sem os extras.
- **Formato do `AKALABET.DAT`:** 7 segmentos de 16 KB: 3 telas (2 segmentos cada: linhas 0–127, linhas 128–211) e o segmento de música
  (`[n] [velocidade, off/len das 3 vozes] × n` + trilhas de pares `(nota, duração)`). Índices em `akalabeth.h` (`ART_*`, `MUS_SEG`).
- **Música em fluxo:** a interrupção lê pares (nota, duração) de 3 anéis de 32 bytes; `music_pump` (chamada em `ui_sfx_update`, nos laços de espera) reabastece
  do segmento (ou da trilha curta embutida, sem mapper). Anel vazio = pausa de 1 tick, sem travar.

### Desvios do listing original (intencionais)

1. Laço infinito do ladrão sem item (linha 4620): corrigido.
2. "Sorte" (HP ao sair) e conclusão da missão só **quando o monstro morre**; o BASIC aplicava a cada golpe (linha 1667).
3. `A` aceita minúsculas; `S` (status) não gasta turno.
4. A espera de 0,5 s por passo só ocorre quando há algo para ler.

---

## 2. Onde estamos (v0.4.0)

### Pronto e testado no emulador (openMSX, Philips NMS 8250, MSX-DOS 2)

- [x] Título renderizado com o próprio motor 3D, criação de personagem, loja.
- [x] Mundo colorido com tiles, minimapa incremental, HUD incremental.
- [x] Masmorra 3D preenchida (paredes, portas, portas laterais, escadas, buracos, baús), minimapa da masmorra com neblina.
- [x] 10 monstros em vetores coloridos (do BASIC), IA, combate corpo a corpo, arco, machado arremessado, escudo.
- [x] Amuleto mágico: escada acima/abaixo, KILL, BAD?? (toad, lizard man, backfire).
- [x] Armadilha, buraco, escada, baú, mímico, ladrão, gremlin.
- [x] Castelo, missões, cavaleiro (Lord), tela de morte.
- [x] Salvar/carregar (`W`/`L`, título com `L`); avisos "NO SAVE" e "NEEDS DOS2".
- [x] Música (5 temas de 3 vozes, 30–50 s cada com RAM mapeada) e 14 efeitos pelo PSG; `M` liga/desliga.
- [x] **RAM mapeada** (v0.4.0): abertura, ajuda (`H`), tela de vitória, músicas longas; com fallback sem mapper.
- [x] Saída para o DOS (`ESC`) restaurando texto e paleta; gancho de interrupção restaurado.
- [x] Disco de 720 KB (`msxdisk.exe`) que inicia sozinho.

### Medidas de desempenho (quadros de 20 ms por passo; PAL, openMSX, MSX-DOS 2)

| Ação | v0.1 | v0.2.0 | v0.3.0 |
| --- | --- | --- | --- |
| Passo no mundo | 60–67 | 19–20 | **15–16** (≈0,3 s) |
| Passo na masmorra (frente) | 62–70 | 33–34 | **27–28** (≈0,55 s) |
| Virar na masmorra | 62 | 26 | **20** (≈0,4 s) |
| Redesenho 3D (parte do passo) | 19–27 | 19–27 | **13–21** |

Medido com `g_turntime`/`g_drawtime`/`g_hudtime` e `tools/test/runtim.sh`. Os números da v0.3.0 foram conferidos nesta data
(mundo: 3 passos; masmorra: início, 2 passos em frente, giro e passo).

### Verificado na v0.3.0 (openMSX, MSX-DOS 2)

- [x] Mundo: tiles do atlas (cidade, pinheiros, montanhas), texto e HUD incremental, volta da tela de status.
- [x] Masmorra com troca de páginas: HUD correto nas duas páginas, minimapa, porta, buraco, descer de nível, status.
- [x] Salvar/carregar (estado restaurado: ouro, comida, andar, minimapa).
- [x] Tela de morte e tela da missão do castelo com o texto novo.
- [ ] Não repetidos após a v0.3.0: loja com compras, amuleto (KILL/BAD??), armadilha, mímico, ESC. O código desses fluxos não mudou, mas só o texto e o HUD foram retestados.

### Não testado ainda

- [x] ~~Joystick~~: **removido** na v0.4.1 (jogo só de teclado).
- [ ] **Máquina de 60 Hz** com o jogo rodando (o andamento da música a 60 Hz é só deduzido).
- [ ] **Hardware real** e outros MSX2 (VRAM de 64 KB, outras BIOS). Só C-BIOS/NMS 8250 no emulador.
- [ ] Opção 3 do amuleto matando um monstro (a rotina de morte foi vista com o rato, mas não com o feitiço).

---

## 3. Problemas conhecidos

| # | Problema | Notas |
| --- | --- | --- |
| 1 | **A música ainda precisa de refinamento.** Na v0.3.0 estava fluida; na v0.4.0 o usuário voltou a ouvir engasgos ao andar na masmorra, na melodia. | Medido no openMSX (NMS 8250 256 KB): 50 interrupções/s estáveis, anéis nunca esvaziam (só ~40 ticks na troca de música), notas regulares (+1 tick fixo por nota, ~2% mais lento; +1 tick em ~1% por `g_psgBusy`). Causa provável corrigida: efeitos só de ruído (passo/golpe/erro) cortavam a melodia no canal A; agora somam ruído. **Não confirmado auditivamente.** Próximo: ~~corrigir o +1 tick por nota~~ (feito na v0.4.1), ~~tirar o polling de joystick~~ (removido na v0.4.1), revisar volumes/envelopes e a mixagem, compor os temas com mais cuidado. |
| 8 | **Memória do programa quase esgotada** (TPA até 0xDB06; sobram ≈900 bytes de pilha). | Ver "RAM mapeada e orçamento de memória". |
| 9 | Em MSX2 de 128 KB faltam segmentos livres: sem abertura/ajuda ilustrada/vitória, músicas curtas. | Só a arte poderia caber (6 segmentos); decidir se vale um modo "só ajuda". |
| 2 | Passo na masmorra ainda leva ~0,55 s. | O que sobra é o desenho do 3D (CPU: cálculo e comandos do VDP). Falta um perfil por função (usar `out/*.sym`, que inclui as `static`). |
| 3 | Os tiles do mundo ficaram com 44 linhas (antes 50): a base das montanhas e das casas aparece mais curta. | Consequência de guardar o atlas na página 0 (linhas 212–255). |
| 4 | `build.bat` do MSXgl falha na etapa do disco (`msxtar --size=720K`). | Inofensivo; use `tools/mkdisk.bat`. |
| 5 | Os arquivos do MSX-DOS 2 no `.dsk` vêm das ferramentas do MSXgl. | Conferir licença antes de distribuir o disco. |
| 6 | Aplicam-se float → inteiro: HP/2 no backfire arredonda para baixo. | Diferenças mínimas do BASIC. |
| 7 | Sprites desativados (tabelas colidem com o atlas). | Se algum dia usar sprites, mover o atlas. |

### Música picotando — hipóteses (histórico; problema considerado resolvido na v0.3.0)

1. **Ticks perdidos pela flag `g_psgBusy`.** `ui_pollkey` liga a flag durante as chamadas BIOS ao joystick (`GTSTCK`/`GTTRIG`, 4 `CALSLT` por poll). Se a interrupção cai nessa janela, `snd_tick` retorna sem avançar a música: o andamento oscila. **Correção provável:** ler o joystick (porta 14/15 do PSG) dentro da própria interrupção, ou adiar só a escrita (não o avanço da música).
2. **Seções longas com interrupções desabilitadas** (`DI`) em rotinas de VDP/BIOS (comandos do VDP, CALSLT). Medir quantas interrupções são atendidas atrasadas (`g_sndTicks` vs. `g_JIFFY`).
3. **Pausas do DOS** durante acesso a disco (salvar/carregar) e telas muito pesadas (várias interrupções seguidas atendidas de uma vez).
4. **Compensação de 60 Hz** (`s_skip`) só deve atuar em NTSC; conferir `VDP_GetFrequency()` na máquina do usuário.
5. **Envelope curto** (`left <= 1` reduz 3 de volume) pode soar como corte em notas de 1 passo. Testar sem a "release".

Instrumentar primeiro: contador de ticks pulados por `g_psgBusy`, duração máxima da interrupção e atraso `g_JIFFY − g_sndTicks`.

---

## 4. Roadmap sugerido

**Próxima sessão (curto prazo)**
1. **Refinar a música** (ver problema 1): confirmar o corte da melodia, envelopes/mixagem, percussão (o +1 tick por nota e o polling de joystick foram eliminados na v0.4.1). Instrumentar anéis vazios (contador de subexecução).
2. **Perfil por função do 3D** (amostrar o PC e atribuir com `out/akalabeth.sym`, que traz as funções `static`) e atacar os maiores custos. O passo da masmorra ainda é ~0,55 s.
3. Repetir a regressão completa (loja, amuleto, armadilha, mímico, ESC) na v0.3.0.
4. Testar **60 Hz**; testar em MSX2 real.

**Médio prazo**
5. ~~Usar a RAM mapeada~~ (feito na v0.4.0: abertura, ajuda, vitória, música longa). Próximo: mais telas (morte, loja com arte), compressão RLE das imagens (carga do disco), mais músicas (loja, combate).
6. Percussão (canal de ruído) na música; efeitos extras (portas, magia). **Antes, liberar memória do programa** (seção "orçamento").
7. Opção de salvar a preferência de música/volume junto do jogo.
8. Rolagem de mensagens.

**Longo prazo**
9. **Porte para PC em Go** reaproveitando regras e dados: separar o núcleo de regras (já isolado em `akalabeth.c`) e tornar `gen_data.js`/`gen_music.js` geradores de dados neutros (JSON).
10. Licenciamento: decidir a licença do código deste porte e confirmar a do listing de Nanochess e dos arquivos do MSX-DOS 2.

---

## 5. Como compilar, testar e gerar release

```
# compilar (MSXgl/projects/akalabeth)
build.bat                         -> out/akalabeth.com   (ignore o erro da etapa de disco)
tools/mkdisk.bat                  -> dist/akalabeth.dsk + dist/AKALABET.COM

# regenerar dados (só se mudar o basic.bas, a música ou a arte)
node tools/gen_data.js ../../../basic.bas content/data_gen.h
bash tools/gen_assets.sh [dir-png]   -> content/music_gen.h + content/AKALABET.DAT (dir-png: prévias das telas)

# rodar (o NMS 8250 original tem só 128 KB: para testar os extras use uma cópia da máquina com <size>256</size>)
openmsx -machine Philips_NMS_8250 -ext msxdos2 -diska dist/akalabeth.dsk
```

### Modos de depuração (`AK_DEBUG` em `akalabeth.h`)

| Valor | Cenário |
| --- | --- |
| 0 | **Jogo normal (obrigatório em releases).** |
| 1 | Masmorra nível 1 com um rato à frente. |
| 2 | Parado sobre o castelo. |
| 3 | Balrog ao lado com 1 de HP (testa a morte). |
| 4 | Masmorra de teste (porta, baú, buraco). |
| 5 | Armadilha logo à frente. |
| 6 | Mímico a 2 células. |
| 7 | Amuleto mágico no nível 2. |
| 8 | Mímico a 3 células (testa o feitiço KILL). |

### Cheat de teste

`C` no mundo (v0.4.1): revela o castelo no minimapa e mostra `CASTLE AT x,y`. O mesmo número da sorte sempre gera o mesmo mundo, e o castelo é único por mapa.

### Roteiro de teste no openMSX

`tools/test/` guarda o harness (Tcl + PowerShell) usado nas verificações: sincroniza com o jogo pelo contador `g_keywait`, grava capturas e mede quadros (`g_drawtime`, `g_hudtime`, `g_turntime`). Os caminhos estão fixos para a máquina original; adapte-os. Armadilhas descobertas:

- Chamadas BIOS diretas travam em MSX-DOS (use `CALSLT`).
- `throttle off` faz o openMSX pular quadros de vídeo: a captura sai velha. Espere tempo real (`after realtime`) antes de `screenshot`.
- `::samples` é variável reservada do openMSX; não a use.
- `g_keywait` vem de BSS não zerado: o jogo o zera em `ui_init`.

### Checklist de release

1. `AK_DEBUG 0`; incrementar `AK_VERSION` (`akalabeth.h`).
2. `build.bat` + `tools/mkdisk.bat`; conferir `dist/AKALABET.COM == out/akalabeth.com`.
3. Teste de fumaça do `.dsk` no openMSX (título → loja → mundo → masmorra → salvar).
4. Atualizar `CHANGELOG.md`, `docs/releases/vX.Y.Z.md` e a seção **2. Onde estamos** deste arquivo.
5. `git commit`, `git tag -a vX.Y.Z`, `gh release create` com `release/akalabeth-vX.Y.Z.zip`.
