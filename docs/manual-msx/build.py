# Gera o livreto "Akalabeth - World of Doom - Manual MSX2" (A5, 16 paginas) em dois PDFs A4 paisagem:
#   manual-msx.pdf          paginas em ordem de leitura (2 por folha)
#   manual-msx-impressao.pdf  imposicao para imprimir frente e verso e dobrar (livreto grampeado)
# Uso: python build.py   (requer o Microsoft Edge para converter o HTML em PDF)
import os, subprocess, sys, art

HERE = os.path.dirname(os.path.abspath(__file__))
IMG = '../images/'
EDGE = r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'

CSS = '''
@import url('https://fonts.googleapis.com/css2?family=Cinzel:wght@500;700;900&family=Source+Sans+3:ital,wght@0,400;0,600;0,700;1,400&family=VT323&display=swap');
:root { --ink:#14308a; --acc:#2f7fd8; --gold:#c68a00; --paper:#fffdf5; --bg:#0b1d6b; --star:#eaf2ff; --red:#c4372b; --soft:#e4edfb; }
@page { size: 297mm 210mm; margin: 0; }
* { box-sizing: border-box; }
html, body { margin: 0; padding: 0; background: #fff; }
body { font-family: 'Source Sans 3', 'Segoe UI', Verdana, sans-serif; color: var(--ink); font-size: 9.4pt; line-height: 1.36; }
.sheet { width: 297mm; height: 210mm; display: flex; page-break-after: always; overflow: hidden; }
.sheet:last-child { page-break-after: auto; }
.page { width: 148.5mm; height: 210mm; position: relative; background: var(--paper); padding: 15mm 13mm 15mm; overflow: hidden; }
.page.left { border-right: 0.2mm solid #cfd8ee; }
.frame { position: absolute; inset: 6mm; border: 0.5mm solid var(--ink); pointer-events: none; }
.frame::after { content: ''; position: absolute; inset: 1.2mm; border: 0.2mm solid var(--acc); }
.folio { position: absolute; bottom: 8.6mm; left: 0; right: 0; text-align: center; font-family: 'VT323', monospace; font-size: 13pt; color: var(--ink); letter-spacing: 1px; }
.folio span { background: var(--paper); padding: 0 3mm; }
.runhead { position: absolute; top: 8.4mm; left: 0; right: 0; text-align: center; font-family: 'Cinzel', Georgia, serif; font-size: 7pt; letter-spacing: 3px; color: var(--acc); font-weight: 700; }
.runhead span { background: var(--paper); padding: 0 3mm; }
h1 { font-family: 'Cinzel', Georgia, serif; font-weight: 900; font-size: 19pt; line-height: 1.05; margin: 0 0 2mm; letter-spacing: 1px; color: var(--ink); }
h2 { font-family: 'Cinzel', Georgia, serif; font-weight: 700; font-size: 11.6pt; margin: 4mm 0 1.2mm; letter-spacing: .5px; color: var(--ink); border-bottom: 0.3mm solid var(--acc); padding-bottom: .6mm; }
h3 { font-family: 'Cinzel', Georgia, serif; font-weight: 700; font-size: 9.2pt; margin: 2.6mm 0 .6mm; color: var(--acc); }
p { margin: 0 0 2mm; text-align: justify; hyphens: auto; }
.lede::first-letter { font-family: 'Cinzel', serif; font-weight: 900; font-size: 30pt; float: left; line-height: .85; margin: 1mm 1.6mm 0 0; color: var(--ink); }
.note { background: var(--soft); border-left: 1.2mm solid var(--acc); padding: 1.8mm 2.6mm; margin: 2.4mm 0; font-size: 8.8pt; }
.warn { background: #fff4d6; border-left: 1.2mm solid var(--gold); padding: 1.8mm 2.6mm; margin: 2.4mm 0; font-size: 8.8pt; }
.key { display: inline-block; font-family: 'VT323', monospace; font-size: 11pt; line-height: 1; background: var(--ink); color: #fff; border-radius: 1mm; padding: .6mm 1.6mm .3mm; margin: 0 .3mm; }
table { border-collapse: collapse; width: 100%; font-size: 8.6pt; margin: 1.6mm 0 2.4mm; }
th { text-align: left; font-family: 'Cinzel', serif; font-size: 7.6pt; letter-spacing: .6px; color: #fff; background: var(--ink); padding: 1mm 1.6mm; }
td { padding: .9mm 1.6mm; border-bottom: 0.2mm solid #c5d2ee; vertical-align: top; }
tr:nth-child(even) td { background: #f2f6fd; }
ul { margin: 0 0 2mm; padding-left: 4.2mm; } li { margin-bottom: .8mm; }
.shot { display: block; width: 100%; border: 0.5mm solid var(--ink); border-radius: 1mm; box-shadow: 1.4mm 1.4mm 0 var(--soft); }
.cap { text-align: center; font-size: 7.6pt; font-style: italic; color: var(--acc); margin: 1.4mm 0 2.4mm; }
.art svg { display: block; width: 100%; height: auto; }
.center { text-align: center; }
.badge { display: inline-block; font-family: 'VT323', monospace; background: var(--ink); color: #fff; padding: 0 2.4mm; border-radius: 1.4mm; font-size: 13pt; letter-spacing: 1px; }
.row { display: flex; gap: 4mm; align-items: flex-start; }
.col { flex: 1; }
/* capa */
.cover { background: var(--bg); color: var(--star); padding: 0; }
.cover .frame { border-color: var(--star); inset: 5mm; } .cover .frame::after { border-color: var(--acc); }
.cover .art { position: absolute; left: 10mm; right: 10mm; top: 54mm; bottom: 28mm; }
.cover .art svg { width: 100%; height: 100%; }
.cover .t1 { position: absolute; top: 14mm; left: 0; right: 0; text-align: center; font-family: 'Cinzel', serif; font-weight: 900; font-size: 30pt; letter-spacing: 3px; color: var(--star); text-shadow: 0.7mm 0.7mm 0 var(--acc); }
.cover .t2 { position: absolute; top: 34mm; left: 0; right: 0; text-align: center; font-family: 'Cinzel', serif; font-weight: 700; font-size: 13pt; letter-spacing: 6px; color: var(--gold); }
.cover .t3 { position: absolute; top: 44mm; left: 0; right: 0; text-align: center; font-family: 'VT323', monospace; font-size: 12pt; letter-spacing: 3px; color: var(--star); opacity: .85; }
.cover .msx { position: absolute; bottom: 10mm; left: 0; right: 0; text-align: center; }
.cover .msx .badge { background: var(--star); color: var(--bg); font-size: 15pt; padding: 0 4mm; }
.cover .band { position: absolute; bottom: 24mm; left: 12mm; right: 12mm; height: 3mm; background: repeating-linear-gradient(90deg, var(--acc) 0 3mm, var(--gold) 3mm 6mm, var(--star) 6mm 9mm, var(--bg) 9mm 12mm); opacity: .9; }
/* contracapa */
.page.back { --ink:#eaf2ff; --paper:#0b1d6b; --acc:#86b6ff; --gold:#f0b400; --red:#ff8a78; background:#0b1d6b; color:#eaf2ff; }
.page.back .frame { border-color: var(--ink); }
.page.back .small { color:#b9d0ff; }
.back .door { margin: 0 auto; }
.credits { font-size: 8.2pt; line-height: 1.34; }
.credits b { font-family: 'Cinzel', serif; font-size: 7.4pt; letter-spacing: .8px; color: var(--acc); display: block; margin-top: 1.4mm; }
.small { font-size: 7.4pt; color: #4d5f9e; }
.legend { display: grid; grid-template-columns: 11mm 1fr; gap: 1.2mm 2.4mm; align-items: center; font-size: 8.8pt; }
.legend .ic svg { width: 10mm; height: 10mm; display: block; }
.legend b { font-family: 'Cinzel', serif; font-size: 8.6pt; }
.keys { width: 100%; }
.keys td, .keys th { text-align: left; }
'''


def page(num, inner, cls='', head=True):
    side = 'left' if num % 2 == 0 else 'right'
    h = '<div class="runhead"><span>AKALABETH · WORLD OF DOOM · MSX2</span></div>' if head else ''
    f = f'<div class="folio"><span>{num}</span></div>' if head else ''
    fr = '<div class="frame"></div>'
    return f'<div class="page {side} {cls}">{fr}{h}{inner}{f}</div>'


def shot(name, cap=None, w=100):
    s = f'<img class="shot" style="width:{w}%;margin:0 auto" src="{IMG}{name}.png">'
    if cap:
        s += f'<div class="cap">{cap}</div>'
    return s


def k(t):
    return f'<span class="key">{t}</span>'


A = art
P = {}

# 1 -------------------------------------------------------------- capa
P[1] = f'''<div class="page right cover"><div class="frame"></div>
<div class="t1">AKALABETH</div><div class="t2">WORLD OF DOOM</div><div class="t3">MANUAL DO JOGADOR · EDIÇÃO MSX2</div>
<div class="art">{A.cover_art()}</div><div class="band"></div>
<div class="msx"><span class="badge">MSX2</span> <span class="badge" style="background:transparent;color:var(--star);">MSX-DOS 2 · SCREEN 5</span></div></div>'''

# 2 -------------------------------------------------------------- anais I
P[2] = page(2, f'''
<h1 style="color:var(--red);font-size:16pt;margin-bottom:0">CUIDADO, MORTAL!</h1>
<p style="font-family:'Cinzel',serif;font-weight:700;margin:0 0 3mm;color:var(--acc);font-size:9.4pt">Insensato, tu invades Akalabeth! Mundo da perdição…</p>
<h2>Os Anais de Akalabeth</h2>
<p class="lede">Dizem que, há muito tempo, a paz e a tranquilidade cobriam as terras. Comida e bebida corriam livres, homens e feras viviam em harmonia, ouro e prata abundavam. Era a <b>Idade de Ouro de Akalabeth</b>.</p>
<p>Mondain, segundo filho de Wolfgang, um grande rei de outrora, quis tomar a herança do irmão e usou para isso seus grandes poderes do mal. Por muitos anos Mondain percorreu as terras de Akalabeth espalhando o mal e a morte por onde passava. Criou masmorras tão fundas e extensas que seus andares mais baixos jamais foram explorados.</p>
<p>Nessas masmorras soltou ainda mais maldade: enviou ladrões, esqueletos e serpentes para morar perto da superfície, e demônios e balrogs para guardar as profundezas. Agora o sangue corria livre em Akalabeth e criaturas imundas logo passaram a rondar a superfície. Mondain lançou tamanha doença e peste sobre a terra que homens e feras viviam em constante medo. Assim foi a Era Sombria de Akalabeth.</p>
<div class="art" style="width:78%;margin:2mm auto 0">{A.crypt_art()}</div>
''')

# 3 -------------------------------------------------------------- anais II
P[3] = page(3, f'''
<div class="art" style="width:74%;margin:0 auto 1mm">{A.king_art()}</div>
<p>Então surgiu da terra um homem puro e justo para combater o Senhor das Trevas. <b>British</b>, Campeão da Luz Branca, travou batalha com Mondain nas profundezas do labirinto de masmorras e por fim o expulsou de Akalabeth para sempre. British, da Luz Branca, foi proclamado <b>Lord British</b>, Protetor de Akalabeth. Mas, ai de nós, grandes danos haviam sido causados às terras. O Renascimento de Akalabeth começou.</p>
<div class="center art" style="width:50%;margin:1mm auto">{A.divider(180)}</div>
<p style="font-family:'Cinzel',serif;font-weight:700;color:var(--ink);font-size:10.4pt;line-height:1.3;text-align:center;margin-top:3mm">É teu dever livrar Akalabeth das criaturas imundas que a infestam…<br><span style="color:var(--red)">…tentando continuar vivo!!</span></p>
''')

# 4 -------------------------------------------------------------- preparando
P[4] = page(4, f'''
<h1>Preparando o MSX</h1>
<p>Esta é a versão para <b>MSX2</b> do clássico <i>Akalabeth</i>. O jogo usa o modo gráfico <b>SCREEN 5</b>, a música e os efeitos do PSG, e guarda telas e músicas na <b>memória mapeada</b> do MSX-DOS 2.</p>
<h2>O que você precisa</h2>
<table><tr><th>Item</th><th>Requisito</th></tr>
<tr><td>Computador</td><td>MSX2 com <b>64 KB de VRAM</b> e drive de disquete</td></tr>
<tr><td>Sistema</td><td><b>MSX-DOS 2</b> (salvar/carregar e RAM mapeada). No MSX-DOS 1 o jogo roda, sem essas funções.</td></tr>
<tr><td>Memória</td><td><b>256 KB</b> recomendados: abertura, ajuda ilustrada, tela de vitória e músicas longas. Com 128 KB o jogo roda sem esses extras.</td></tr></table>
<h2>Como iniciar</h2>
<ul><li><b>Disco pronto:</b> insira <b>akalabeth.dsk</b> (720 KB) no drive A: e ligue o MSX2. O jogo inicia sozinho.</li>
<li><b>Em outro disco:</b> copie <b>AKALABET.COM</b> e <b>AKALABET.DAT</b> (juntos!) e digite <span class="key">AKALABET</span>.</li>
<li><b>No openMSX:</b><br><span style="font-family:'VT323',monospace;font-size:10.4pt">openmsx -machine Philips_NMS_8250 -ext msxdos2 -diska akalabeth.dsk</span></li></ul>
<div class="note">Ao iniciar, o jogo lê o <b>AKALABET.DAT</b> para a memória mapeada (<i>LOADING…</i>). O <b>AUTOEXEC.BAT</b> do disco executa <span class="key">RAMDISK /D</span> antes, pois o RAM disk do DOS 2 ocupa quase toda essa memória. Em outro disco, faça o mesmo.</div>
{shot('intro', 'A tela de abertura, pintada para esta versão.', 50)}
''')

# 5 -------------------------------------------------------------- primeiros passos
P[5] = page(5, f'''
<h1>Primeiros passos em Akalabeth</h1>
<div class="note"><b>Nota</b> — as informações seguem a ordem em que aparecem no jogo; podem ser lidas junto com a primeira partida. — <i>Lord British</i></div>
<h3>O número da sorte</h3>
<p>É a semente do gerador de números aleatórios. Cada vez que você usa o mesmo número, o jogo começa igual: mesmo mapa, mesmas masmorras, mesmo personagem e mesmos monstros.</p>
<h3>O nível de jogo (1 a 10)</h3>
<p>Ligado diretamente à força dos monstros: o nível 10 cria monstros dez vezes mais difíceis de destruir que o nível 1.</p>
<h2>O jogador</h2>
<table><tr><th>Atributo</th><th>Para que serve</th></tr>
<tr><td><b>Hit Points</b></td><td>Dano que você aguenta antes de morrer. Diminui a cada golpe sofrido e sobe ao sair de uma masmorra, conforme os monstros mortos.</td></tr>
<tr><td><b>Strength</b></td><td>Dano que você causa aos inimigos.</td></tr>
<tr><td><b>Dexterity</b></td><td>Chance de acertar.</td></tr>
<tr><td><b>Stamina</b></td><td>Defesa depois de um combate longo.</td></tr>
<tr><td><b>Wisdom</b></td><td>Usada nas missões do rei.</td></tr>
<tr><td><b>Gold</b></td><td>Dinheiro! Patrimônio!</td></tr></table>
<p class="small">O jogo sorteia os seis valores; responda <span class="key">Y</span> para aceitar ou qualquer outra tecla para sortear de novo.</p>
<div class="art" style="width:30%;margin:0 auto">{A.monster_art()}</div>
''')

# 6 -------------------------------------------------------------- tipo e loja
P[6] = page(6, f'''
<h1>O tipo de jogador</h1>
<p>Um aventureiro pode ser <b>Lutador</b> (<span class="key">F</span>) ou <b>Mago</b> (<span class="key">M</span>). As duas opções têm vantagens e desvantagens. A desvantagem do lutador é não saber controlar o <b>amuleto mágico</b>; o mago não pode usar <b>rapieiras</b> nem <b>arcos</b>.</p>
<h2>A loja de aventuras</h2>
<p>No início do jogo, e em toda cidade, o jogador pode comprar comida e armas. Para comprar basta digitar a <b>primeira letra</b> do item; o preço aparece na loja. <b>Compre comida suficiente para não morrer de fome!</b></p>
<table><tr><th>Item</th><th>Ouro</th><th>Dano</th><th>Observação</th></tr>
<tr><td><b>F</b>ood</td><td>1</td><td>—</td><td>Cada compra dá 10 de comida</td></tr>
<tr><td><b>R</b>apier</td><td>8</td><td>1–10</td><td>Não serve para magos</td></tr>
<tr><td><b>A</b>xe</td><td>5</td><td>1–5</td><td>Pode ser arremessado (e perdido)</td></tr>
<tr><td><b>S</b>hield</td><td>6</td><td>1</td><td>Defende e também bate</td></tr>
<tr><td><b>B</b>ow and arrows</td><td>3</td><td>1–4</td><td>Alcance de 5 células; não serve para magos</td></tr>
<tr><td><b>M</b>agic amulet</td><td>15</td><td>?</td><td>Veja a página 12</td></tr></table>
<p>Tecle <span class="key">Q</span> para sair da loja. O jogo começa numa cidade qualquer de um mapa de <b>21 × 21</b> quadrados. Você começa <b>sem comida</b>: a primeira parada é a loja.</p>
{shot('shop', 'A loja de aventuras.', 70)}
''')

# 7 -------------------------------------------------------------- mapa
P[7] = page(7, f'''
<h1>Legenda do mapa</h1>
<p>No mundo você vê os nove quadrados ao redor; à esquerda ficam suas coordenadas, o tipo do lugar e a bússola; à direita, o <b>minimapa</b> das áreas já visitadas.</p>
<div class="legend">
<div class="ic">{A.icon_town()}</div><div><b>As cidades</b> — a loja de aventuras: compra de equipamento. <span class="key">ENTER</span> para entrar.</div>
<div class="ic">{A.icon_castle()}</div><div><b>O castelo</b> — para vencer é preciso visitá-lo. Daqui parte-se em missão. Casa de Lord British.</div>
<div class="ic">{A.icon_tree()}</div><div><b>Árvores</b> — marcos do caminho. Dá para atravessar.</div>
<div class="ic">{A.icon_mount()}</div><div><b>Montanhas</b> — obstáculos intransponíveis. O mapa é cercado por elas.</div>
<div class="ic">{A.icon_dungeon()}</div><div><b>Masmorras</b> — onde o mal espreita. <span class="key">ENTER</span> para descer.</div>
</div>
<div class="note">Cada passo no mundo gasta <b>1 de comida</b>; na masmorra, 0,1.</div>
{shot('world', 'O mundo: tiles, bússola e minimapa.', 78)}
''')

# 8 -------------------------------------------------------------- movimento
P[8] = page(8, f'''
<h1 class="center">Movimento</h1>
<div class="row" style="align-items:center;margin-top:1mm">
<div class="art" style="width:15mm;flex:none">{A.pillar()}</div>
<div class="col"><table class="keys"><tr><th>Tecla</th><th>Fora</th><th>Masmorra</th></tr>
<tr><td>{k('↑')}</td><td>norte</td><td>em frente</td></tr>
<tr><td>{k('←')}</td><td>oeste</td><td>vira à esquerda</td></tr>
<tr><td>{k('→')}</td><td>leste</td><td>vira à direita</td></tr>
<tr><td>{k('↓')}</td><td>sul</td><td>meia-volta</td></tr>
<tr><td>{k('S')}</td><td colspan="2">estatísticas</td></tr>
<tr><td>{k('A')}</td><td>—</td><td>ataque</td></tr>
<tr><td>{k('ESPAÇO')}</td><td colspan="2">passa o turno</td></tr></table></div>
<div class="art" style="width:15mm;flex:none">{A.pillar(True)}</div></div>
<h2>As outras teclas</h2>
<table><tr><th>Tecla</th><th>Efeito</th></tr>
<tr><td>{k('ENTER')}</td><td>Entra em cidades, masmorra e castelo; sobe e desce escadas, buracos e alçapões.</td></tr>
<tr><td>{k('P')}</td><td><b>Pausa</b> liga/desliga: com ela ligada o jogo para depois de cada sequência de combate e espera uma tecla, para você ler a tela. Deixa o jogo um pouco mais lento; iniciantes podem gostar.</td></tr>
<tr><td>{k('H')} ou {k('?')}</td><td>Tela de <b>ajuda</b> (também no título).</td></tr>
<tr><td>{k('M')}</td><td>Liga/desliga a música.</td></tr>
<tr><td>{k('W')} / {k('L')}</td><td>Salva / carrega o jogo (AKALABET.SAV).</td></tr>
<tr><td>{k('ESC')}</td><td>Sai para o DOS (pergunta antes).</td></tr></table>
''')

# 9 -------------------------------------------------------------- masmorra
P[9] = page(9, f'''
<h1>A masmorra</h1>
<p>Vista em primeira pessoa. No canto superior direito o <b>minimapa</b> mostra só o que você já viu. O painel de baixo traz as mensagens e, à direita, <b>FOOD</b>, <b>H.P.</b>, <b>GOLD</b>, <b>LEVEL</b> (andar) e <b>FACING</b> (direção).</p>
{shot('dungeon', 'Corredor da masmorra, com o minimapa no canto.', 76)}
<div class="legend" style="margin-top:2mm">
<div class="ic">{A.icon_ladder()}</div><div><b>Escada</b> — <span class="key">ENTER</span> sobe ou desce. No andar 1, a escada para cima <i>sai</i> da masmorra.</div>
<div class="ic">{A.icon_hole()}</div><div><b>Buraco</b> no chão — <span class="key">ENTER</span> desce um andar.</div>
<div class="ic">{A.icon_chest()}</div><div><b>Baú</b> — ouro e um item aleatório. Cuidado: o <b>mímico</b> também parece um baú!</div>
</div>
<ul style="margin-top:1.6mm"><li><b>Portas</b> e <b>portas secretas</b> (parecem parede) podem ser atravessadas.</li>
<li><b>Armadilhas</b> são invisíveis: você cai um andar e perde 3 + sorteio(andar) de HP.</li>
<li>A cada troca de andar o desenho é o mesmo (para o mesmo número da sorte), mas monstros e baús <b>voltam</b>.</li></ul>
''')

# 10 ------------------------------------------------------------- combate
P[10] = page(10, f'''
<h1>Combate</h1>
<p>Tecle {k('A')} e depois a letra da arma ({k('R')} {k('A')} {k('S')} {k('B')} {k('M')}). Qualquer outra tecla significa lutar com as mãos. Se a arma não for sua, o jogo avisa <i>NOT OWNED</i>.</p>
<div class="row"><div class="col">
<ul><li><b>Corpo a corpo</b> (rapieira, escudo, mãos, machado de perto): atinge a célula à sua frente.</li>
<li><b>Arco</b> e <b>machado arremessado</b>: atingem o primeiro monstro na linha de visão, até 5 células.</li></ul></div>
<div class="art" style="width:9mm;flex:none">{A.sword_icon()}</div></div>
<h2>As contas do combate</h2>
<table><tr><th>O quê</th><th>Fórmula</th></tr>
<tr><td>Seu golpe acerta</td><td>(DEX − nº do monstro − andar) / 25</td></tr>
<tr><td>Dano seu</td><td>sorteio de 0 até o dano da arma + STR / 5</td></tr>
<tr><td>Monstro acerta</td><td>1 − (STA + escudo − nº do monstro − andar) / 20</td></tr>
<tr><td>Dano dele</td><td>sorteio(nº do monstro) + andar</td></tr></table>
<p>Matar um monstro dá <b>nº do monstro + andar</b> de ouro e soma “sorte”, que vira <b>pontos de vida</b> quando você sai da masmorra. Monstros feridos <b>fogem</b> e se recuperam, mas atacam se ficarem encurralados.</p>
{shot('mimic', 'Um baú que ataca: é o mímico.', 60)}
''')

# 11 ------------------------------------------------------------- monstros
P[11] = page(11, f'''
<h1>Os monstros</h1>
<p>O monstro de número <i>n</i> só aparece a partir do andar <i>n − 2</i>, e cada um tem 40% de chance de estar em cada andar.</p>
<table><tr><th>Nº</th><th>Monstro</th><th>Andar</th><th>Notas</th></tr>
<tr><td>1</td><td><b>Skeleton</b></td><td>1</td><td></td></tr>
<tr><td>2</td><td><b>Thief</b></td><td>1</td><td>50% das vezes <b>rouba</b> um item em vez de atacar</td></tr>
<tr><td>3</td><td><b>Giant Rat</b></td><td>1</td><td></td></tr>
<tr><td>4</td><td><b>Orc</b></td><td>2</td><td></td></tr>
<tr><td>5</td><td><b>Viper</b></td><td>3</td><td></td></tr>
<tr><td>6</td><td><b>Carrion Crawler</b></td><td>4</td><td>Pendurado no teto</td></tr>
<tr><td>7</td><td><b>Gremlin</b></td><td>5</td><td>50% das vezes come <b>metade da sua comida</b></td></tr>
<tr><td>8</td><td><b>Mimic</b></td><td>6</td><td>Disfarçado de baú</td></tr>
<tr><td>9</td><td><b>Daemon</b></td><td>7</td><td></td></tr>
<tr><td>10</td><td><b>Balrog</b></td><td>8</td><td>O último</td></tr></table>
<div class="art" style="width:70%;margin:2mm auto 0">{A.crypt_art()}</div>
<p class="small center">Quanto mais fundo, mais forte o inimigo — e mais ouro ele carrega.</p>
''')

# 12 ------------------------------------------------------------- amuleto
P[12] = page(12, f'''
<h1>O amuleto mágico</h1>
<div class="row"><div class="col"><p>Para <b>lutadores</b> cada uso sorteia um dos quatro efeitos abaixo. O <b>mago</b> escolhe o efeito, mas a cada uso há 25% de chance de o amuleto perder uma carga (<i>LAST CHARGE ON THIS AMULET!</i>).</p></div>
<div class="art" style="width:16mm;flex:none">{A.icon_amulet()}</div></div>
<table><tr><th>Opção</th><th>Efeito</th></tr>
<tr><td><b>1</b> LADDER UP</td><td>Cria uma escada para <b>cima</b> onde você está.</td></tr>
<tr><td><b>2</b> LADDER DN</td><td>Cria uma escada para <b>baixo</b> onde você está.</td></tr>
<tr><td><b>3</b> KILL</td><td>Ataque mágico em linha reta (dano até 10 + andar).</td></tr>
<tr><td><b>4</b> BAD??</td><td>Arriscado! <b>Toad</b> (FOR, DES, STA e SAB viram 3), <b>Lizard Man</b> (todos os atributos ×2,5; o ouro não) ou <b>Backfire</b> (perde metade do HP).</td></tr></table>
<div class="row"><div class="col">{shot('amulet-menu', 'O menu do amuleto.')}</div><div class="col">{shot('combat-magic', 'O feitiço KILL em ação.')}</div></div>
<div class="art" style="width:38%;margin:3mm auto 0">{A.monster_art()}</div>
<p class="small center">O feitiço BAD?? transforma… ou não.</p>
''')

# 13 ------------------------------------------------------------- missoes
P[13] = page(13, f'''
<h1>As missões de Lord British</h1>
<ol style="margin:0 0 2mm;padding-left:4.6mm"><li>Vá ao <b>castelo</b> (<span class="key">ENTER</span>). Na primeira vez, diga seu nome e aceite o desafio.</li>
<li>O primeiro monstro é o de número <b>sabedoria / 3</b> (mínimo 1). Mate <b>um</b> deles na masmorra e volte.</li>
<li>Voltando com a missão cumprida, o rei <b>soma 1 a todos os seus atributos</b> e pede o monstro seguinte.</li>
<li>Se voltar sem ter matado o monstro, ele manda você de volta à missão.</li>
<li>Ao matar o <b>Balrog</b> e retornar, você é sagrado <b>Lord</b> — e pode continuar jogando num nível mais alto.</li></ol>
<div class="row"><div class="col">{shot('castle-quest', 'O rei dá a missão.')}</div><div class="col">{shot('outro', 'A tela de vitória.')}</div></div>
<div class="art" style="width:48%;margin:4mm auto 0">{A.king_art()}</div>
<p class="small center">Lord British, Protetor de Akalabeth.</p>
''')

# 14 ------------------------------------------------------------- salvar musica ajuda
P[14] = page(14, f'''
<h1>Salvar, música e ajuda</h1>
<h2>Salvar e carregar</h2>
<p>{k('W')} salva em <b>AKALABET.SAV</b> (1248 bytes, no disco atual). {k('L')} carrega, a qualquer momento ou na tela de título. O arquivo guarda personagem, mundo, andar, monstros, mapa explorado e posição. Só funciona com <b>MSX-DOS 2</b>.</p>
<h2>Música e efeitos</h2>
<p>Cinco temas originais de 3 vozes (título, mundo, masmorra, castelo/loja e vitória) e 14 efeitos pelo PSG. Com a RAM mapeada, cada tema dura de 30 a 50 s antes de repetir; sem ela tocam versões curtas. {k('M')} liga e desliga a música.</p>
<div class="warn"><b>Atenção:</b> a música ainda precisa de refinamento. Pode haver cortes ou engasgos em alguns momentos, principalmente ao andar na masmorra.</div>
<h2>Tela de ajuda</h2>
<p>{k('H')} abre a ajuda com todas as teclas, sem gastar turno.</p>
{shot('help', 'A tela de ajuda.', 58)}
<h2>Morte e fome</h2>
<p>Sem comida você morre de fome. Ao morrer aparece a lápide: {k('ESC')} ou {k('ENTER')} volta ao título.</p>
''')

# 15 ------------------------------------------------------------- dicas
P[15] = page(15, f'''
<h1>Dicas de sobrevivência</h1>
<ol style="margin:0 0 2mm;padding-left:4.6mm"><li>Gaste ouro em <b>comida</b> antes de tudo; depois compre uma arma.</li>
<li>O <b>machado</b> serve a quem tem pouca destreza: atirar acerta de longe, mas ele se perde.</li>
<li>Confie no <b>minimapa</b>: mostra por onde você já passou e onde estão as escadas.</li>
<li>Um <b>baú que se mexe</b> não é baú. Desconfie dos baús nos andares fundos.</li>
<li>Antes de ir fundo, <b>salve</b> ({k('W')}).</li>
<li>Para sair de uma masmorra sem escada por perto, o <b>amuleto</b> cria uma.</li>
<li>Sair da masmorra devolve <b>HP</b> proporcional aos monstros que você matou.</li></ol>
<h2>Limites desta versão</h2>
<ul><li>A <b>música</b> ainda precisa de refinamento (veja a página 14).</li>
<li>O jogo é só de <b>teclado</b>. Máquinas de <b>60 Hz</b> e <b>hardware real</b> ainda não foram testados.</li>
<li>Um passo na masmorra leva cerca de 0,55 s; no mundo, cerca de 0,3 s.</li>
<li>O mapa de um número da sorte é o <b>deste</b> porte; não coincide com o do original em BASIC.</li></ul>
<h2>Versão</h2>
<p>Este manual acompanha a versão <b>0.4.0</b> (outubro de 2026). O projeto é de fãs, sem fins comerciais; o código está em <b>github.com/wilsonpilon/akalabeth</b>.</p>
<div class="row" style="margin-top:2mm"><div class="col">{shot('death', 'A lápide.')}</div><div class="col">{shot('status-toad', 'Status (virou sapo!).')}</div></div>
<p class="center" style="font-family:'Cinzel',serif;color:var(--acc);font-weight:700;font-size:9pt;margin-top:2mm">Que a Luz Branca guie teus passos.</p>
''')

# 16 ------------------------------------------------------------- contracapa
P[16] = f'''<div class="page left back"><div class="frame"></div>
<div class="art door" style="margin-top:2mm;width:60%">{A.door_art()}</div>
<p style="font-family:'Cinzel',serif;font-weight:700;text-align:center;font-size:9.6pt;margin:2mm 4mm 1mm;line-height:1.3">Agora conheces o básico do jogo. Experimenta os comandos: há muito por dizer e por descobrir.<br><span style="color:var(--red)">Vai a Akalabeth e busca a aventura onde quiseres!!!</span></p>
<h2 style="text-align:center;border:0;margin:2mm 0 0">CRÉDITOS</h2>
<div class="credits center">
<b>JOGO ORIGINAL</b>Design e programação: Richard Garriott (Lord British)<br>
Arte da capa do jogo e do livreto: Denis Loubet<br>
Design do livreto e arte interna: Robin West Design<br>
<b>VERSÃO EM MSX BASIC</b>Oscar Toledo G. (Nanochess), 2019<br>
<b>VERSÃO MSX2 EM SDCC / MSXGL</b>Wilson Pilon, 2026<br>
<span class="small">Biblioteca MSXgl: Guillaume “Aoineko” Blanchard</span><br>
<span class="small">Este livreto foi inspirado no manual original de 1980; as ilustrações são novas.</span>
<div class="small" style="margin-top:2mm">© 1980, 1981 California Pacific Computer Co. (jogo e livreto originais)<br>Projeto de fãs, sem fins comerciais · v0.4.0</div></div></div>'''


def sheets_reading():
    blank = '<div class="page left" style="background:var(--paper)"></div>'
    out = [f'<div class="sheet">{blank}{P[1]}</div>']
    for i in range(2, 16, 2):
        out.append(f'<div class="sheet">{P[i]}{P[i+1]}</div>')
    out.append(f'<div class="sheet">{P[16]}{blank.replace("left", "right")}</div>')
    return ''.join(out)


def sheets_print():
    order = [(16, 1), (2, 15), (14, 3), (4, 13), (12, 5), (6, 11), (10, 7), (8, 9)]
    out = []
    for a, b in order:
        out.append(f'<div class="sheet">{P[a]}{P[b]}</div>')
    return ''.join(out)


def write(name, body):
    html = f'<!doctype html><html><head><meta charset="utf-8"><title>Akalabeth MSX2 - Manual</title><style>{CSS}</style></head><body>{body}</body></html>'
    p = os.path.join(HERE, name + '.html')
    open(p, 'w', encoding='utf8').write(html)
    return p


def to_pdf(html, pdf):
    cmd = [EDGE, '--headless=new', '--disable-gpu', '--no-pdf-header-footer', '--virtual-time-budget=20000',
           f'--print-to-pdf={pdf}', 'file:///' + html.replace('\\', '/')]
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=180)


if __name__ == '__main__':
    h1 = write('manual-msx', sheets_reading())
    h2 = write('manual-msx-impressao', sheets_print())
    out = os.path.join(HERE, '..')
    to_pdf(h1, os.path.abspath(os.path.join(out, 'manual-msx.pdf')))
    to_pdf(h2, os.path.abspath(os.path.join(out, 'manual-msx-impressao.pdf')))
    print('ok')
