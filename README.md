# AKALABETH — World of Doom (MSX2 / MSX-DOS 2)

**v0.4.0** · porte do clássico RPG de masmorras de 1979 para **MSX2**, em C com **SDCC + MSXgl**,
como programa `.COM` do MSX-DOS 2 (SCREEN 5, 256×212, paleta própria de 16 cores, música e efeitos pelo PSG).

*English: a faithful but colourful MSX2 port of Richard Garriott's 1979 Akalabeth (3D wireframe dungeons rebuilt as filled, shaded views),
with an overworld map, dungeon automap, save/load (MSX-DOS 2), PSG music and sound effects. Portuguese docs below.*

![Título](docs/images/title.png)

| | | |
| --- | --- | --- |
| ![Mundo](docs/images/world.png) | ![Masmorra](docs/images/dungeon.png) | ![Loja](docs/images/shop.png) |

## Jogar

> ⚠️ **Aviso:** a música ainda precisa de refinamento. Pode haver cortes ou engasgos em alguns momentos (principalmente ao andar na masmorra), a mixagem das 3 vozes é simples e os temas novos ainda não foram afinados. Relatos são bem-vindos.

Arquivos de cada release (`release/akalabeth-vX.Y.Z.zip`, anexado à release do GitHub):

| Arquivo | Descrição |
| --- | --- |
| `akalabeth.dsk` | Disco de 720 KB com MSX-DOS 2 e o jogo. Inicia sozinho. |
| `AKALABET.COM` | Só o programa, para qualquer disco com MSX-DOS. Sem o `AKALABET.DAT` ao lado, joga sem as telas novas e com músicas curtas. |
| `AKALABET.DAT` | Telas pixel art (abertura, ajuda, vitória) e músicas completas, carregadas na RAM mapeada. |

```
openmsx -machine Philips_NMS_8250 -ext msxdos2 -diska akalabeth.dsk
```

Requisitos: MSX2 com 64 KB de VRAM e drive de disco. **Salvar/carregar e a RAM mapeada exigem MSX-DOS 2**; a abertura, a tela de vitória e as músicas completas pedem **256 KB** de RAM (com 128 KB o jogo roda normalmente, sem esses extras).
Veja o **[manual do jogador](docs/MANUAL.md)**.

| Tecla | Efeito |
| --- | --- |
| Setas / joystick | Andar (mundo) ou frente/virar (masmorra) |
| ENTER | Entrar em cidade, masmorra, castelo; escadas e buracos |
| A | Atacar · ESPAÇO passa o turno |
| S | Status · **H ajuda** · M música · P pausa nas mensagens |
| W / L | Salvar / carregar (`AKALABET.SAV`) · ESC sai para o DOS |

## Novidades em relação ao original

- Vídeo **SCREEN 5**: paredes e pisos preenchidos e sombreados pela distância; portas, escadas, buracos e baús coloridos; monstros em linhas coloridas.
- Mundo com tiles desenhados e **minimapa**; masmorra com **minimapa de exploração**.
- **RAM mapeada de 256 KB** (MSX-DOS 2): telas pixel art de **abertura**, **ajuda** (`H`) e **vitória**, e **músicas de ~30–50 s** com 3 vozes (5 temas), lidas aos poucos da RAM mapeada.
- **14 efeitos** pelo PSG, tocados na interrupção do VDP.
- **Salvar e carregar** via MSX-DOS 2.
- Telas de loja, status, castelo, cavaleiro e morte redesenhadas, com fades de paleta.
- Mesmas regras, monstros e fórmulas do listing em BASIC (veja os desvios em [docs/OUTLINE.md](docs/OUTLINE.md)).

## Status e próximos passos

Veja **[docs/OUTLINE.md](docs/OUTLINE.md)** (arquitetura, o que foi testado, problemas conhecidos, roadmap) e o
[CHANGELOG](CHANGELOG.md). Resumo:

- ✅ Jogo completo e jogável de ponta a ponta no emulador (MSX-DOS 2).
- ⚠️ **Música ainda precisa de refinamento:** na v0.4.0 as músicas longas em fluxo às vezes engasgam ao andar na masmorra (a v0.4.0 já deixa de cortar a melodia nos efeitos de ruído, mas falta confirmar e ajustar).
- ⚡ **Desempenho (v0.3.0):** passo no mundo ≈ 0,3 s e na masmorra ≈ 0,55 s (era ≈ 1,3 s na v0.1); troca de páginas na masmorra e texto escrito direto na VRAM.
- ⏳ **Não testados:** joystick, 60 Hz com o jogo rodando, hardware real.
- 🔧 **Melhorias previstas:** perfil por função do 3D, memória do programa quase esgotada (veja o outline), percussão na música, porte para PC em Go.

## Compilar

Requer o [MSXgl](https://github.com/aoineko-fr/MSXgl) (que traz SDCC e Node) na pasta `MSXgl/`, com este projeto em `MSXgl/projects/akalabeth/`.

```
cd MSXgl/projects/akalabeth
build.bat            # gera out/akalabeth.com (ignore o erro da etapa de disco)
tools/mkdisk.bat     # gera dist/akalabeth.dsk, AKALABET.COM e AKALABET.DAT (usa msxdisk.exe na raiz)
```

`AK_DEBUG` em `akalabeth.h` deve ficar em `0` nas versões finais.

O `basic.bas` (porte GW-BASIC de Nanochess) **não** faz parte deste repositório, por causa da licença do autor. Os dados gerados (`content/data_gen.h`) já estão versionados; só é preciso ter o `basic.bas` na raiz para rodar `tools/gen_data.js` de novo.

| Arquivo | Conteúdo |
| --- | --- |
| `akalabeth.c` | Regras, mundo, masmorra, combate, loja, castelo, títulos |
| `gfx.c` | Paleta, primitivas, renderizador 3D, mundo, minimapas |
| `ui.c` | Console, HUD, entrada, motor de som (música em fluxo), aleatório |
| `ramstore.c` | RAM mapeada do DOS 2: segmentos, cópia segmento ↔ RAM/VRAM |
| `tools/gen_data.js` | `basic.bas` → vetores dos monstros e tabelas de perspectiva |
| `tools/gen_music.js` | Notação textual → trilhas do PSG (reserva no código + segmento de música) |
| `tools/gen_art.js` | Gera as telas pixel art (256×212, paleta do jogo) |
| `tools/gen_assets.sh` | Gera `content/AKALABET.DAT` (arte + música) |
| `tools/mkdisk.*` | Monta o disco de 720 KB com `msxdisk.exe` |
| `tools/test/` | Harness de testes automáticos no openMSX |

## Créditos

- **Jogo original:** Richard Garriott ("Lord British"), *Akalabeth: World of Doom*, Apple II, 1979.
- **Referência de regras e dados:** porte para GW-BASIC de **Oscar Toledo G. (Nanochess)**, jan–fev/2019. Regras, textos e vetores dos monstros foram extraídos dele.
- **Biblioteca:** [MSXgl](https://github.com/aoineko-fr/MSXgl) de Guillaume "Aoineko" Blanchard (CC BY-SA).
- **Porte MSX2:** este projeto, com os temas musicais originais compostos para ele.

Projeto de fã, sem fins comerciais. **Licença do código deste porte: a definir.** O listing de Nanochess (`basic.bas`) e os arquivos do MSX-DOS 2 incluídos no disco têm licenças próprias que devem ser confirmadas antes de redistribuir.
