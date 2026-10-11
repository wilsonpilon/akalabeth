# Harness de testes no openMSX

Scripts usados para testar o jogo de forma automática no **openMSX** (máquina `Philips_NMS_8250`, cartucho `msxdos2`).

- `lib.tcl` — utilitários (capturas, setas, ESC).
- `lib2.tcl` — executor de roteiros: `{key "texto"}`, `{arrow up}`, `{shot nome atraso}`, `{until 1}`, `{psg rotulo}`, `{tim rotulo}`...
  Sincroniza com o jogo pelo contador `g_keywait` (incrementado a cada espera de tecla).
- `mkkw.sh` — lê o `.map` do build e grava `kw.tcl` com os endereços de `g_keywait`, `g_drawtime`, `g_hudtime`, `g_turntime`, `g_sndTicks`...
- `run.ps1` — abre o openMSX com um roteiro (`-Scn scenX -Disk pasta -Ext msxdos2`).
- `scen*.tcl` — roteiros (masmorra de teste, armadilha, mímico, amuleto, salvar/carregar, medição de tempo).
- `runtim.sh`, `runall.sh` — compilam com `AK_DEBUG` apropriado e rodam as baterias.

**Atenção:** os caminhos estão fixos para a máquina onde foram escritos (pasta de rascunho da sessão e `D:\msx\openmsx`).
Ajuste `SP`/`$sp` nos scripts antes de usar. Os roteiros que dependem de `AK_DEBUG` > 0 precisam de uma build de depuração
(veja a tabela em `docs/OUTLINE.md`); volte para `0` antes de gerar uma release.

Armadilhas conhecidas: veja a seção "Roteiro de teste no openMSX" do `docs/OUTLINE.md`.

**v0.4.0 (RAM mapeada):** o `Philips_NMS_8250` tem só 128 KB (1 segmento livre no DOS 2). Para testar abertura/ajuda/vitória e as músicas longas use uma
cópia da máquina com `<size>256</size>` no `MemoryMapper` (salve como `NMS8250_256K.xml` em `openmsx/share/machines` e passe `-Machine NMS8250_256K` ao `run.ps1`).
Cenários novos: `scenR1` (abertura/título/ajuda), `scenR2` (ajuda no jogo + vitória; build com `AK_DEBUG 2` e `AK_DBG_TA (-10)`), `scenR4` (salvar/carregar), `scenR5` (masmorra, música, `M`, `H`; `AK_DEBUG 1`).
