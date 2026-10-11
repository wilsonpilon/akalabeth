# Ilustracoes vetoriais (SVG) do livreto: tracos em uma cor, inspiradas nos livretos antigos de RPG.
# Cada funcao devolve uma string <svg>. Cores via variaveis CSS (--ink, --acc, --gold, --paper).

def svg(w, h, body, extra=''):
    return f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" {extra}>{body}</svg>'

S = 'stroke="var(--ink)" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"'
F = 'fill="var(--ink)"'
N = 'fill="none"'


def skull(cx, cy, r=30, fill='var(--paper)', stroke='var(--ink)'):
    return f'''<g stroke="{stroke}" stroke-width="2.4" stroke-linejoin="round" stroke-linecap="round">
<path d="M{cx-r},{cy} C{cx-r},{cy-r*1.35} {cx+r},{cy-r*1.35} {cx+r},{cy} C{cx+r},{cy+r*0.55} {cx+r*0.6},{cy+r*0.75} {cx+r*0.55},{cy+r*0.95} L{cx-r*0.55},{cy+r*0.95} C{cx-r*0.6},{cy+r*0.75} {cx-r},{cy+r*0.55} {cx-r},{cy} Z" fill="{fill}"/>
<ellipse cx="{cx-r*0.42}" cy="{cy+r*0.05}" rx="{r*0.28}" ry="{r*0.34}" fill="{stroke}"/>
<ellipse cx="{cx+r*0.42}" cy="{cy+r*0.05}" rx="{r*0.28}" ry="{r*0.34}" fill="{stroke}"/>
<path d="M{cx},{cy+r*0.3} l{-r*0.12},{r*0.28} l{r*0.24},0 z" fill="{stroke}"/>
<path d="M{cx-r*0.4},{cy+r*0.95} v{r*0.3} M{cx-r*0.13},{cy+r*0.95} v{r*0.3} M{cx+r*0.13},{cy+r*0.95} v{r*0.3} M{cx+r*0.4},{cy+r*0.95} v{r*0.3}" fill="none"/>
<path d="M{cx-r*0.55},{cy+r*1.25} h{r*1.1}" fill="none"/>
</g>'''


def ribs(cx, top, w=60, n=5, gap=13):
    out = [f'<path d="M{cx},{top-8} V{top+n*gap+10}" {S} {N}/>']
    for i in range(n):
        y = top + i * gap
        ww = w * (1 - abs(i - 1.6) * 0.09)
        out.append(f'<path d="M{cx},{y} C{cx-ww*0.5},{y-8} {cx-ww},{y+2} {cx-ww*0.95},{y+12}" {S} {N}/>')
        out.append(f'<path d="M{cx},{y} C{cx+ww*0.5},{y-8} {cx+ww},{y+2} {cx+ww*0.95},{y+12}" {S} {N}/>')
    return ''.join(out)


def hatch(x, y, w, h, step=5, ang=1):
    out = []
    for i in range(0, int(w + h), step):
        x1 = x + i if i < w else x + w
        y1 = y if i < w else y + (i - w)
        x2 = x if i < h else x + (i - h)
        y2 = y + i if i < h else y + h
        out.append(f'M{x1},{y1} L{x2},{y2}')
    return f'<path d="{" ".join(out)}" stroke="var(--ink)" stroke-width="0.9" fill="none" opacity="0.55"/>'


# ---------------------------------------------------------------- capa: heroi contra esqueleto
def cover_art():
    b = []
    # estrelas
    import random
    random.seed(4)
    for _ in range(70):
        x, y = random.randint(8, 392), random.randint(8, 300)
        b.append(f'<circle cx="{x}" cy="{y}" r="{random.choice([0.8,1.2,1.8])}" fill="var(--star)"/>')
    # lua
    b.append('<circle cx="318" cy="70" r="30" fill="var(--star)"/><circle cx="332" cy="60" r="26" fill="var(--bg)"/>')
    # chao
    b.append('<path d="M0,420 C60,395 120,405 200,398 C280,392 340,402 400,392 V520 H0 Z" fill="var(--star)" opacity="0.18"/>')
    # esqueleto (direita)
    sk = '<g stroke="var(--star)" stroke-width="3" fill="none" stroke-linecap="round" stroke-linejoin="round">'
    sk += '<path d="M255,300 C230,305 215,330 218,360 M255,300 C280,305 295,330 292,360" />'
    for i in range(5):
        y = 318 + i * 15
        w = 38 - abs(i - 1.5) * 4
        sk += f'<path d="M255,{y} C{255-w*0.5},{y-9} {255-w},{y+3} {255-w*0.9},{y+14}"/><path d="M255,{y} C{255+w*0.5},{y-9} {255+w},{y+3} {255+w*0.9},{y+14}"/>'
    sk += '<path d="M255,296 V400"/><path d="M225,395 C240,405 270,405 285,395"/><path d="M255,400 L232,470 L248,520 M255,400 L282,470 L270,520"/>'
    sk += '<path d="M292,332 L330,300 L360,250"/><path d="M360,250 L372,210" stroke-width="5"/>'
    sk += '<path d="M218,336 L190,370 L176,410"/>'
    sk += '</g>'
    b.append(sk)
    b.append(skull(255, 262, 30, fill='var(--bg)', stroke='var(--star)'))
    # escudo do esqueleto
    b.append('<circle cx="176" cy="415" r="26" fill="var(--bg)" stroke="var(--star)" stroke-width="3"/><circle cx="176" cy="415" r="8" fill="var(--star)"/>')
    # heroi (esquerda)
    h = '<g stroke="var(--star)" stroke-width="3" stroke-linejoin="round" stroke-linecap="round">'
    h += '<path d="M70,230 C40,245 30,300 20,380 L10,520 L120,520 L112,400 C110,330 100,270 100,240 Z" fill="var(--bg)"/>'
    h += '<circle cx="86" cy="215" r="22" fill="var(--bg)"/>'
    h += '<path d="M64,205 C62,175 100,170 108,200 C100,190 80,188 64,205 Z" fill="var(--star)"/>'
    h += '<path d="M78,214 h6 M92,214 h6" />'
    h += '<path d="M30,330 C60,345 90,345 112,330" fill="none"/><path d="M50,420 L56,520 M80,430 L84,520" fill="none" stroke-width="2"/>'
    h += '<path d="M104,300 L160,285 L215,262" fill="none" stroke-width="4"/>'
    h += '<path d="M215,262 L270,238" stroke-width="2" /></g>'
    b.append(h)
    # lamina
    b.append('<path d="M150,292 L330,215" stroke="var(--star)" stroke-width="6" stroke-linecap="round"/>')
    b.append('<path d="M142,280 L158,305" stroke="var(--gold)" stroke-width="6" stroke-linecap="round"/>')
    # faiscas
    b.append('<g stroke="var(--gold)" stroke-width="3" stroke-linecap="round"><path d="M300,230 l12,-14 M310,242 l20,-4 M292,222 l2,-18 M322,226 l12,-14"/></g>')
    return svg(400, 520, ''.join(b), 'preserveAspectRatio="xMidYMax meet"')


# ---------------------------------------------------------------- cripta com escada e esqueleto (historia)
def crypt_art():
    b = []
    b.append(f'<path d="M20,300 C10,150 60,40 170,30 C260,25 310,110 300,300 Z" fill="var(--paper)" {S}/>')
    b.append(hatch(24, 40, 270, 260, 7))
    b.append(f'<path d="M40,300 C34,180 70,80 160,70 C240,66 280,140 276,300 Z" fill="var(--paper)" {S}/>')
    # escada
    b.append(f'<path d="M70,290 V130 M110,290 V130" {S} {N}/>')
    for i in range(7):
        y = 150 + i * 20
        b.append(f'<path d="M70,{y} H110" {S} {N}/>')
    b.append(skull(190, 190, 26))
    b.append(ribs(190, 232, 44, 4, 12))
    b.append(f'<path d="M190,232 V300" {S} {N}/><path d="M144,236 L112,270 M236,236 L270,262 L282,300" {S} {N}/>')
    # teias
    b.append(f'<path d="M232,76 L300,150 M252,72 C262,100 280,118 296,130 M270,76 C268,100 280,120 296,140" stroke="var(--ink)" stroke-width="1.2" fill="none"/>')
    b.append(f'<path d="M20,300 H300" {S} {N}/>')
    return svg(320, 310, ''.join(b))


# ---------------------------------------------------------------- rei Lord British
def king_art():
    b = []
    b.append(f'<path d="M20,330 C18,250 40,210 90,196 H200 C250,210 272,250 270,330 Z" fill="var(--paper)" {S}/>')
    b.append(hatch(28, 215, 235, 115, 6))
    # cota de malha
    b.append(f'<path d="M110,200 L145,330 L180,200" fill="var(--paper)" {S}/>')
    for r in range(6):
        for c in range(4):
            x = 124 + c * 10 + (r % 2) * 5 - r * 1
            b.append(f'<circle cx="{x+8}" cy="{232+r*14}" r="3" fill="none" stroke="var(--ink)" stroke-width="1"/>')
    # cabelo e rosto
    b.append(f'<path d="M80,60 C70,120 62,170 60,230 L100,215 L100,100 Z M210,60 C222,120 230,170 232,230 L192,215 L192,100 Z" fill="var(--paper)" {S}/>')
    b.append(f'<ellipse cx="145" cy="108" rx="52" ry="64" fill="var(--paper)" {S}/>')
    b.append(f'<path d="M97,128 C100,196 190,196 193,128 C185,160 105,160 97,128 Z" {F}/>')
    b.append(f'<path d="M115,96 h22 M155,96 h22" {S} {N}/><circle cx="127" cy="104" r="4" {F}/><circle cx="165" cy="104" r="4" {F}/>')
    b.append(f'<path d="M145,104 V130 M138,134 h14" {S} {N}/>')
    b.append(f'<path d="M127,150 C140,156 150,156 163,150" stroke="var(--paper)" stroke-width="3" fill="none"/>')
    # coroa
    b.append(f'<path d="M92,62 L100,26 L120,48 L145,14 L170,48 L190,26 L198,62 Z" fill="var(--gold)" {S}/>')
    b.append(f'<circle cx="145" cy="14" r="5" fill="var(--paper)" {S}/><path d="M92,62 H198" {S} {N}/>')
    # cetro
    b.append(f'<path d="M262,330 V90" stroke="var(--ink)" stroke-width="6" stroke-linecap="round"/><path d="M262,92 C246,70 250,48 262,30 C274,48 278,70 262,92 Z" fill="var(--gold)" {S}/>')
    return svg(300, 340, ''.join(b))


# ---------------------------------------------------------------- monstro (polvo na parede de tijolos)
def monster_art():
    b = []
    b.append(f'<path d="M30,300 C10,150 80,40 170,50 C260,60 300,180 270,300 Z" fill="var(--paper)" {S}/>')
    # tijolos
    for r in range(11):
        y = 66 + r * 20
        b.append(f'<path d="M{40+r%2*12},{y} H{270}" stroke="var(--ink)" stroke-width="1" opacity=".5"/>')
        for c in range(8):
            x = 52 + c * 26 + (r % 2) * 13
            b.append(f'<path d="M{x},{y} v20" stroke="var(--ink)" stroke-width="1" opacity=".5"/>')
    b.append(f'<ellipse cx="155" cy="150" rx="62" ry="78" fill="var(--paper)" {S}/>')
    b.append(f'<path d="M110,120 l30,6 M200,120 l-30,6" {S} {N}/>')
    b.append(f'<path d="M112,128 h28 v10 h-28 Z M170,128 h28 v10 h-28 Z" fill="var(--ink)"/>')
    b.append(f'<path d="M140,176 C140,160 170,160 170,176 V204 C170,218 140,218 140,204 Z" fill="var(--ink)"/>')
    b.append(f'<path d="M147,176 v14 M155,176 v14 M163,176 v14" stroke="var(--paper)" stroke-width="2"/>')
    # tentaculos
    for sx, sgn in ((100, -1), (210, 1)):
        b.append(f'<path d="M{sx},220 C{sx+sgn*40},240 {sx+sgn*70},210 {sx+sgn*95},250 S{sx+sgn*70},290 {sx+sgn*110},290" {S} {N} stroke-width="4"/>')
    b.append(f'<path d="M150,228 C140,260 170,270 150,300 M165,228 C175,262 150,276 168,304" {S} {N} stroke-width="4"/>')
    return svg(320, 310, ''.join(b))


# ---------------------------------------------------------------- pergaminho (legenda do mapa) - so a moldura
def scroll_frame(w=300, h=420):
    b = f'''<path d="M30,40 C18,30 14,14 30,8 H{w-22} C{w-6},16 {w-4},34 {w-14},44 V{h-40} C{w-4},{h-26} {w-8},{h-8} {w-26},{h-6} H40 C24,{h-12} 18,{h-34} 30,{h-44} Z" fill="var(--paper)" {S}/>
<path d="M30,40 C44,52 44,26 30,22 C16,20 18,40 30,40 Z M{w-14},{h-44} C{w-28},{h-32} {w-26},{h-58} {w-14},{h-60}" {S} {N}/>
<path d="M44,24 H{w-34} M44,{h-22} H{w-40}" stroke="var(--ink)" stroke-width="1" stroke-dasharray="3 4" fill="none"/>'''
    return svg(w, h, b, 'preserveAspectRatio="none"')


# ---------------------------------------------------------------- pilares (teclas)
def pillar(flip=False):
    b = f'''<g transform="{'scale(-1,1) translate(-120,0)' if flip else ''}">
<path d="M26,30 C20,10 60,0 94,10 C110,16 104,30 96,36 L88,44 H34 Z" fill="var(--paper)" {S}/>
<path d="M34,44 L38,300 H84 L88,44 Z" fill="var(--paper)" {S}/>
<path d="M50,60 L56,120 L48,150 M70,200 L64,250" stroke="var(--ink)" stroke-width="1.6" fill="none"/>
<path d="M22,300 H100 V318 H22 Z" fill="var(--paper)" {S}/>
<path d="M90,60 C110,100 108,160 96,210" stroke="var(--ink)" stroke-width="1" fill="none" opacity=".6"/>
{hatch(40,50,42,240,8)}
</g>'''
    return svg(120, 330, b)


# ---------------------------------------------------------------- porta de masmorra + esqueleto (contracapa)
def door_art():
    b = []
    # arco de pedra
    b.append(f'<path d="M30,360 V150 C30,40 90,10 150,10 C210,10 270,40 270,150 V360 Z" fill="var(--paper)" {S}/>')
    b.append(hatch(34, 30, 232, 330, 9))
    b.append(f'<path d="M62,360 V160 C62,76 100,48 150,48 C200,48 238,76 238,160 V360 Z" fill="var(--paper)" {S}/>')
    # tabuas
    for x in (92, 122, 152, 182, 212):
        b.append(f'<path d="M{x},60 V360" stroke="var(--ink)" stroke-width="1.6" opacity=".8"/>')
    b.append(f'<path d="M62,150 H238 M62,270 H238" {S} {N} stroke-width="4"/>')
    for x in (74, 226):
        for y in (150, 270):
            b.append(f'<circle cx="{x}" cy="{y}" r="4" {F}/>')
    b.append(f'<path d="M62,160 L238,262 M62,262 L238,160" stroke="var(--ink)" stroke-width="7" stroke-linecap="round" opacity=".9"/>')
    # esqueleto espiando
    b.append(skull(40, 70, 22))
    b.append(f'<path d="M40,96 V150 M40,120 L72,140" {S} {N}/>')
    # teia
    b.append(f'<path d="M230,16 L270,70 M246,14 C250,34 260,52 270,60" stroke="var(--ink)" stroke-width="1.2" fill="none"/>')
    b.append(f'<path d="M0,364 H300" {S} {N}/>')
    return svg(300, 370, ''.join(b))


# ---------------------------------------------------------------- icones do mapa (como o jogo)
def icon_town():
    return svg(40, 40, f'<rect x="4" y="4" width="32" height="32" {N} {S}/><path d="M12,12 h6 v6 h-6 z M22,12 h6 v6 h-6 z M12,22 h6 v6 h-6 z M22,22 h6 v6 h-6 z" {F}/>')

def icon_castle():
    return svg(40, 40, f'<rect x="4" y="4" width="32" height="32" {N} {S}/><path d="M10,30 V16 h4 v-4 h4 v4 h4 v-4 h4 v4 h4 v14 Z" {F}/><path d="M17,30 v-8 h6 v8" fill="var(--paper)"/>')

def icon_tree():
    return svg(40, 40, f'<path d="M20,4 L32,24 H8 Z M20,14 L35,34 H5 Z" fill="var(--paper)" {S}/><path d="M20,34 v5" {S} {N}/>')

def icon_mount():
    return svg(40, 40, f'<path d="M3,35 L16,10 L24,22 L29,14 L37,35 Z" fill="var(--paper)" {S}/><path d="M16,10 l-4,8 l4,-2 l3,5" {S} {N} stroke-width="1.4"/>')

def icon_dungeon():
    return svg(40, 40, f'<ellipse cx="20" cy="20" rx="15" ry="10" {F}/><path d="M8,20 H32" stroke="var(--paper)" stroke-width="2"/><path d="M20,12 v16" stroke="var(--paper)" stroke-width="2"/>')

def icon_ladder():
    b = f'<path d="M12,4 V36 M28,4 V36" {S} {N}/>'
    for y in (10, 18, 26, 34):
        b += f'<path d="M12,{y} H28" {S} {N}/>'
    return svg(40, 40, b)

def icon_hole():
    return svg(40, 40, f'<path d="M4,12 H36 L30,30 H10 Z" {F}/>')

def icon_chest():
    return svg(40, 40, f'<rect x="5" y="16" width="30" height="18" fill="var(--paper)" {S}/><path d="M5,22 C5,6 35,6 35,22" fill="var(--paper)" {S}/><rect x="17" y="21" width="6" height="7" fill="var(--gold)" {S}/>')

def icon_amulet():
    return svg(40, 40, f'<path d="M11,4 C14,18 26,18 29,4" {S} {N}/><circle cx="20" cy="27" r="9" fill="var(--red)" {S}/><circle cx="17" cy="24" r="2.4" fill="var(--paper)"/>')

# ---------------------------------------------------------------- enfeites
def divider(w=220):
    return svg(w, 14, f'<path d="M0,7 H{w/2-14} M{w/2+14},7 H{w}" stroke="var(--ink)" stroke-width="1.4"/><path d="M{w/2-10},7 l10,-6 l10,6 l-10,6 z" fill="var(--gold)" stroke="var(--ink)" stroke-width="1.4"/>')

def sword_icon(w=34, h=120):
    return svg(34, 120, f'<path d="M17,4 L22,70 H12 Z" fill="var(--paper)" {S}/><path d="M4,70 H30 M17,70 V104" {S} {N}/><circle cx="17" cy="110" r="6" fill="var(--gold)" {S}/>')

def shield_icon():
    return svg(60, 70, f'<path d="M6,6 H54 V36 C54,52 40,62 30,66 C20,62 6,52 6,36 Z" fill="var(--paper)" {S}/><path d="M30,10 V62 M8,28 H52" stroke="var(--gold)" stroke-width="5"/>')
