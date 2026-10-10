# Akalabeth MSX2 — Outline técnico, status e pendências

**Versão atual: 0.2.0** (10/out/2026) · Alvo: MSX2, 64 KB RAM / 64 KB VRAM, MSX-DOS 1 ou 2 · SDCC 4.6 + MSXgl

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
│       ├── ui.c             console, HUD, entrada, motor de som (música + efeitos), RNG
│       ├── akalabeth.h      constantes, paleta, estado compartilhado, versão, AK_DEBUG
│       ├── content\         data_gen.h e music_gen.h (GERADOS)
│       └── tools\           gen_data.js, gen_music.js, mkdisk.bat/.sh, test\ (harness openMSX)
├── dist\                    saída (akalabeth.dsk, AKALABET.COM) — não versionada
└── msxdisk.exe              utilitário de disco (terceiros) — não versionado
```

### Módulos

| Arquivo | Responsabilidade |
| --- | --- |
| `akalabeth.c` | Estado do jogo, geração do mundo (21×21) e das masmorras (11×11, semente = nº da sorte + entrada + andar), IA dos monstros, combate, loja, castelo/missões, salvar/carregar (handles do DOS 2), título e criação de personagem, `main`. |
| `gfx.c` | Paleta de 16 cores (de 512), primitivas com buffer fora da tela (página 1 do SCREEN 5), renderizador 3D preenchido, tiles do mundo (atlas na VRAM), minimapas. |
| `ui.c` | Console de mensagens, HUD incremental, teclado (CHGET via CALSLT) e joystick, **motor de som** na interrupção, gerador aleatório (xorshift16). |
| `tools/gen_data.js` | Lê `basic.bas` e gera `content/data_gen.h`: polilinhas dos monstros e tabelas de perspectiva reparametrizadas de 280 para 256 px. |
| `tools/gen_music.js` | Gera `content/music_gen.h` a partir de uma notação textual (`A4:3 C#5:1 R:2`). |

### Mapa de VRAM (SCREEN 5)

| Região | Uso |
| --- | --- |
| Página 0, linhas 0–211 | Tela visível. |
| Página 1, linhas 256–415 | Buffer fora da tela (3D da masmorra e tiles do mundo), copiado por `HMMM`. |
| Página 1, linhas 416–465 | **Atlas de tiles** do mundo (montanha, árvores, cidade, masmorra, castelo), 50×50 cada. |
| Página 1, linhas 470–502 | Imagem do **minimapa da masmorra** (34×33), atualizada incrementalmente. |

### Decisões de projeto importantes

- **Chamadas à BIOS:** em MSX-DOS a página 0 é RAM, então qualquer chamada direta à BIOS trava. Tudo passa por `CALSLT` (`bios_call` em `ui.c`).
- **`VDP_UNIT_U16`** no `msxgl_config.h`: com 8 bits em `UY` o `Draw_Line` truncava `y + 256`.
- **Som na interrupção:** gancho em `H.TIMI` (0FD9Fh); só a rotina de interrupção escreve no PSG. Andamento compensado em 60 Hz (pula 1 em cada 6 ticks).
- **Texto é caro:** ~5 ms por caractere. O HUD só reescreve o que mudou; o mundo só reescreve X/Y/nome quando mudam.
- **Salvar/carregar** só em MSX-DOS 2 (handles). Em MSX-DOS 1 o jogo avisa.
- **Mapas ≠ GW-BASIC:** o gerador aleatório é outro; o número da sorte reproduz o **próprio** mapa deste porte.

### Desvios do listing original (intencionais)

1. Laço infinito do ladrão sem item (linha 4620): corrigido.
2. "Sorte" (HP ao sair) e conclusão da missão só **quando o monstro morre**; o BASIC aplicava a cada golpe (linha 1667).
3. `A` aceita minúsculas; `S` (status) não gasta turno.
4. A espera de 0,5 s por passo só ocorre quando há algo para ler.

---

## 2. Onde estamos (v0.2.0)

### Pronto e testado no emulador (openMSX, Philips NMS 8250, MSX-DOS 2)

- [x] Título renderizado com o próprio motor 3D, criação de personagem, loja.
- [x] Mundo colorido com tiles, minimapa incremental, HUD incremental.
- [x] Masmorra 3D preenchida (paredes, portas, portas laterais, escadas, buracos, baús), minimapa da masmorra com neblina.
- [x] 10 monstros em vetores coloridos (do BASIC), IA, combate corpo a corpo, arco, machado arremessado, escudo.
- [x] Amuleto mágico: escada acima/abaixo, KILL, BAD?? (toad, lizard man, backfire).
- [x] Armadilha, buraco, escada, baú, mímico, ladrão, gremlin.
- [x] Castelo, missões, cavaleiro (Lord), tela de morte.
- [x] Salvar/carregar (`W`/`L`, título com `L`); avisos "NO SAVE" e "NEEDS DOS2".
- [x] Música (4 temas, 3 vozes) e 14 efeitos pelo PSG; `M` liga/desliga.
- [x] Saída para o DOS (`ESC`) restaurando texto e paleta; gancho de interrupção restaurado.
- [x] Disco de 720 KB (`msxdisk.exe`) que inicia sozinho.

### Medidas de desempenho (quadros de 20 ms por passo)

| Ação | v0.1 | v0.2.0 |
| --- | --- | --- |
| Passo no mundo | 60–67 | **19–20** |
| Passo na masmorra | 62–70 | **26–34** |
| Redesenho 3D (sem espera/HUD) | 19–27 | 19–27 |

### Não testado ainda

- [ ] **Joystick** (portas 1 e 2): código existe, nunca exercitado.
- [ ] **Máquina de 60 Hz** com o jogo rodando (o andamento da música a 60 Hz é só deduzido).
- [ ] **Hardware real** e outros MSX2 (VRAM de 64 KB, outras BIOS). Só C-BIOS/NMS 8250 no emulador.
- [ ] Opção 3 do amuleto matando um monstro (a rotina de morte foi vista com o rato, mas não com o feitiço).

---

## 3. Problemas conhecidos

| # | Problema | Notas |
| --- | --- | --- |
| 1 | **A música às vezes "pica" (engasga).** | Reportado pelo usuário em 10/out/2026 (não reproduzido em medições). Ver hipóteses abaixo. |
| 2 | Passo na masmorra ainda leva ~0,6 s. | Cerca de 6 quadros são a cópia da página fora da tela. |
| 3 | Texto lento (5 ms/caractere) deixa as telas cheias (loja, castelo) com ~1 s de desenho. | `DrawChar_4B` do MSXgl. |
| 4 | `build.bat` do MSXgl falha na etapa do disco (`msxtar --size=720K`). | Inofensivo; use `tools/mkdisk.bat`. |
| 5 | Os arquivos do MSX-DOS 2 no `.dsk` vêm das ferramentas do MSXgl. | Conferir licença antes de distribuir o disco. |
| 6 | Aplicam-se float → inteiro: HP/2 no backfire arredonda para baixo. | Diferenças mínimas do BASIC. |

### Música picotando — hipóteses (investigar na próxima sessão)

1. **Ticks perdidos pela flag `g_psgBusy`.** `ui_pollkey` liga a flag durante as chamadas BIOS ao joystick (`GTSTCK`/`GTTRIG`, 4 `CALSLT` por poll). Se a interrupção cai nessa janela, `snd_tick` retorna sem avançar a música: o andamento oscila. **Correção provável:** ler o joystick (porta 14/15 do PSG) dentro da própria interrupção, ou adiar só a escrita (não o avanço da música).
2. **Seções longas com interrupções desabilitadas** (`DI`) em rotinas de VDP/BIOS (comandos do VDP, CALSLT). Medir quantas interrupções são atendidas atrasadas (`g_sndTicks` vs. `g_JIFFY`).
3. **Pausas do DOS** durante acesso a disco (salvar/carregar) e telas muito pesadas (várias interrupções seguidas atendidas de uma vez).
4. **Compensação de 60 Hz** (`s_skip`) só deve atuar em NTSC; conferir `VDP_GetFrequency()` na máquina do usuário.
5. **Envelope curto** (`left <= 1` reduz 3 de volume) pode soar como corte em notas de 1 passo. Testar sem a "release".

Instrumentar primeiro: contador de ticks pulados por `g_psgBusy`, duração máxima da interrupção e atraso `g_JIFFY − g_sndTicks`.

---

## 4. Roadmap sugerido

**Próxima sessão (curto prazo)**
1. Diagnosticar e corrigir a **música picotando** (seção 3).
2. **Troca de página** em vez de cópia (`HMMM`) no 3D da masmorra e no mundo: tira ~6 quadros por passo. Exige desenhar o HUD nas duas páginas.
3. Testar **joystick** e **60 Hz**; testar em MSX2 real.
4. Voltar ao desempenho do texto: bitmap font próprio mais rápido, ou cache de linhas.

**Médio prazo**
5. Usar a **RAM mapeada** (256 KB) do DOS 2 para guardar recursos (abertura, tiles, música maior) e carregar do disco.
6. Música mais longa e com percussão (canal de ruído); efeitos extras (portas, magia).
7. Opção de salvar a preferência de música/volume junto do jogo.
8. Tela de abertura/encerramento, telas de ajuda (`H`), rolagem de mensagens.

**Longo prazo**
9. **Porte para PC em Go** reaproveitando regras e dados: separar o núcleo de regras (já isolado em `akalabeth.c`) e tornar `gen_data.js`/`gen_music.js` geradores de dados neutros (JSON).
10. Licenciamento: decidir a licença do código deste porte e confirmar a do listing de Nanochess e dos arquivos do MSX-DOS 2.

---

## 5. Como compilar, testar e gerar release

```
# compilar (MSXgl/projects/akalabeth)
build.bat                         -> out/akalabeth.com   (ignore o erro da etapa de disco)
tools/mkdisk.bat                  -> dist/akalabeth.dsk + dist/AKALABET.COM

# regenerar dados (só se mudar o basic.bas ou a música)
node tools/gen_data.js ../../../basic.bas content/data_gen.h
node tools/gen_music.js content/music_gen.h

# rodar
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
