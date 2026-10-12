// Gera content/art.seg (+ art.json): telas pixel art (SCREEN 5, 256x212, paleta do jogo) comprimidas com RLE, em segmentos de 16 KB.
// Ordem: INTRO, HELP, OUTRO, STORY1..3 (ver ART_* em akalabeth.h). Uso: node tools/gen_art.js content/art.seg [dir-png-opcional]
const fs = require('fs');
const W = 256, H = 212;
// indices da paleta (ver akalabeth.h)
const K = 0, NAVY = 1, ST1 = 2, ST2 = 3, ST3 = 4, ST4 = 5, BLUE = 6, FLOOR = 7, CEIL = 8, CREAM = 9, WOOD = 10, RED = 11, GRASS = 12, FOREST = 13, GOLD = 14, WHITE = 15;
const PAL = [[0,0,0],[0,0,2],[2,1,1],[3,2,2],[4,3,2],[5,4,3],[2,3,7],[1,1,3],[1,0,1],[7,6,3],[6,4,1],[7,1,1],[1,5,1],[0,3,1],[7,5,0],[7,7,7]];

let seed = 1234;
function rnd(n) { seed = (seed * 1103515245 + 12345) & 0x7fffffff; return (seed >> 8) % n; }

class Img {
  constructor() { this.p = new Uint8Array(W * H); }
  px(x, y, c) { x |= 0; y |= 0; if (x >= 0 && x < W && y >= 0 && y < H) this.p[y * W + x] = c; }
  get(x, y) { return this.p[y * W + x]; }
  rect(x, y, w, h, c) { for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) this.px(x + i, y + j, c); }
  box(x, y, w, h, c) { this.rect(x, y, w, 1, c); this.rect(x, y + h - 1, w, 1, c); this.rect(x, y, 1, h, c); this.rect(x + w - 1, y, 1, h, c); }
  disc(cx, cy, r, c) { for (let j = -r; j <= r; j++) for (let i = -r; i <= r; i++) if (i * i + j * j <= r * r + r / 2) this.px(cx + i, cy + j, c); }
  line(x0, y0, x1, y1, c) {
    x0 = Math.round(x0); y0 = Math.round(y0); x1 = Math.round(x1); y1 = Math.round(y1);
    const dx = Math.abs(x1 - x0), dy = -Math.abs(y1 - y0), sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    let e = dx + dy;
    for (;;) { this.px(x0, y0, c); if (x0 === x1 && y0 === y1) break; const e2 = 2 * e; if (e2 >= dy) { e += dy; x0 += sx; } if (e2 <= dx) { e += dx; y0 += sy; } }
  }
  poly(pts, c) { // preenchimento scanline (par-impar)
    let y0 = Math.min(...pts.map(p => p[1])), y1 = Math.max(...pts.map(p => p[1]));
    for (let y = y0; y <= y1; y++) {
      const xs = [];
      for (let i = 0; i < pts.length; i++) {
        const a = pts[i], b = pts[(i + 1) % pts.length];
        if ((a[1] <= y && b[1] > y) || (b[1] <= y && a[1] > y)) xs.push(a[0] + (y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]));
      }
      xs.sort((u, v) => u - v);
      for (let i = 0; i + 1 < xs.length; i += 2) for (let x = Math.round(xs[i]); x < Math.round(xs[i + 1]); x++) this.px(x, y, c);
    }
  }
  // degrade vertical com dithering ordenado entre duas cores
  grad(y0, y1, ca, cb) {
    const bay = [[0,8,2,10],[12,4,14,6],[3,11,1,9],[15,7,13,5]];
    for (let y = y0; y < y1; y++) for (let x = 0; x < W; x++) {
      const t = (y - y0) / (y1 - y0) * 16;
      this.px(x, y, bay[y & 3][x & 3] < t ? cb : ca);
    }
  }
  pack() {
    const o = Buffer.alloc(W * H / 2);
    for (let i = 0; i < W * H; i += 2) o[i >> 1] = (this.p[i] << 4) | this.p[i + 1];
    return o;
  }
  // ppm opcional para conferir
  ppm() {
    const b = Buffer.alloc(W * H * 3);
    for (let i = 0; i < W * H; i++) { const c = PAL[this.p[i]]; b[i * 3] = c[1] * 36; b[i * 3 + 1] = c[0] * 36; b[i * 3 + 2] = c[2] * 36; }
    return Buffer.concat([Buffer.from(`P6\n${W} ${H}\n255\n`), b]);
  }
}
// OBS: RGB16(r,g,b) do jogo = (r,g,b); PAL acima esta (g?)... ver conversao abaixo
// PAL guarda [r,g,b]; ppm usa r=c[0]
Img.prototype.ppm = function () {
  const b = Buffer.alloc(W * H * 3);
  for (let i = 0; i < W * H; i++) { const c = PAL[this.p[i]]; b[i * 3] = Math.min(255, c[0] * 36); b[i * 3 + 1] = Math.min(255, c[1] * 36); b[i * 3 + 2] = Math.min(255, c[2] * 36); }
  return Buffer.concat([Buffer.from(`P6\n${W} ${H}\n255\n`), b]);
};

//-----------------------------------------------------------------------------
function stars(im, y0, y1, n) {
  for (let i = 0; i < n; i++) { const x = rnd(W), y = y0 + rnd(y1 - y0); im.px(x, y, rnd(4) ? ST4 : (rnd(2) ? CREAM : WHITE)); }
}
function mountains(im, baseY, h, c1, c2, step) {
  let x = -10, y = baseY - h / 2;
  const pts = [[-10, baseY + 40]];
  while (x < W + 20) { pts.push([x, baseY - (h / 3 + rnd(h * 2 / 3))]); x += step / 2 + rnd(step); }
  pts.push([W + 20, baseY + 40]);
  im.poly(pts, c1);
  // luz na face esquerda
  for (let i = 1; i < pts.length - 1; i++) { const p = pts[i], q = pts[i + 1]; if (q[1] > p[1]) im.line(p[0], p[1], q[0], q[1], c2); }
}
function tower(im, x, y, w, h, c, hi, roof) {
  im.rect(x, y, w, h, c); im.rect(x, y, 2, h, hi);
  for (let i = 0; i < w; i += 4) im.rect(x + i, y - 3, 2, 3, c);              // ameias
  if (roof) im.poly([[x - 2, y - 3], [x + w / 2, y - 3 - roof], [x + w + 2, y - 3]], RED);
}
function castle(im, cx, by, s) {
  // s = 1 normal
  const u = v => Math.round(v * s);
  im.rect(cx - u(46), by - u(30), u(92), u(30), ST3);
  im.rect(cx - u(46), by - u(30), u(92), 2, ST4);
  for (let i = -46; i < 46; i += 6) im.rect(cx + u(i), by - u(34), u(3), u(4), ST3);
  tower(im, cx - u(54), by - u(52), u(16), u(52), ST3, ST4, u(14));
  tower(im, cx + u(38), by - u(52), u(16), u(52), ST3, ST4, u(14));
  tower(im, cx - u(10), by - u(64), u(20), u(64), ST3, ST4, u(18));
  // bandeiras
  for (const [fx, fy] of [[cx - u(46), by - u(66)], [cx + u(46), by - u(66)], [cx, by - u(86)]]) {
    im.rect(fx, fy, 1, u(10), CREAM); im.poly([[fx + 1, fy], [fx + 8, fy + 2], [fx + 1, fy + 5]], GOLD);
  }
  // portao e janelas
  im.rect(cx - u(7), by - u(18), u(14), u(18), K); im.disc(cx, by - u(18), u(7), K);
  for (let i = -6; i <= 6; i += 3) im.rect(cx + u(i), by - u(20), 1, u(20), WOOD);
  for (const [wx, wy] of [[cx - u(48), by - u(40)], [cx + u(44), by - u(40)], [cx - u(4), by - u(52)], [cx - u(30), by - u(18)], [cx + u(26), by - u(18)]]) {
    im.rect(wx, wy, 3, 5, GOLD); im.px(wx + 1, wy - 1, GOLD);
  }
}
function knight(im, x, y, face) { // x,y = pes; 14x32 aprox.; face = +1 direita
  im.rect(x - 5, y - 10, 4, 10, ST2); im.rect(x + 1, y - 10, 4, 10, ST2);           // pernas
  im.rect(x - 6, y - 2, 5, 2, ST1); im.rect(x + 1, y - 2, 5, 2, ST1);
  im.rect(x - 6, y - 22, 12, 13, BLUE); im.rect(x - 6, y - 22, 12, 2, CREAM);       // tunica
  im.rect(x - 6, y - 12, 12, 2, GOLD);                                              // cinto
  im.rect(x - 4, y - 31, 8, 9, ST4);                                                // elmo
  im.rect(x - 4, y - 28, 8, 2, K); im.rect(x - 1, y - 33, 2, 3, RED);               // viseira, pluma
  im.rect(x - 4 - face * 5, y - 21, 4, 9, WOOD); im.rect(x - 3 - face * 5, y - 20, 2, 7, GOLD); // escudo
  // braco e espada erguida
  const sx = x + face * 6;
  im.rect(sx - 1, y - 28, 3, 8, BLUE);
  im.rect(sx - 1, y - 52, 2, 24, WHITE); im.rect(sx, y - 52, 1, 24, CREAM);          // lamina
  im.rect(sx - 4, y - 29, 9, 2, GOLD); im.rect(sx - 1, y - 27, 3, 3, WOOD);
}
function bands(im) { // faixas pretas para texto sobreposto (o texto do jogo tem fundo opaco)
  im.rect(0, 0, W, 48, K); im.rect(0, 48, W, 1, WOOD); im.rect(0, 49, W, 1, GOLD); im.rect(0, 50, W, 1, WOOD);
  im.rect(0, 195, W, 1, WOOD); im.rect(0, 196, W, 1, GOLD); im.rect(0, 197, W, 16, K);
}
function frame(im) {
  im.box(1, 1, 254, 210, WOOD); im.box(3, 3, 250, 206, GOLD); im.box(5, 5, 246, 202, WOOD);
  for (const [x, y] of [[2, 2], [251, 2], [2, 207], [251, 207]]) im.rect(x, y, 3, 3, CREAM);
}

//-----------------------------------------------------------------------------
function intro() {
  const im = new Img(); seed = 77;
  im.grad(0, 120, K, NAVY); im.grad(120, 150, NAVY, FLOOR);
  stars(im, 52, 110, 70);
  // lua
  for (let j = -15; j <= 15; j++) for (let i = -15; i <= 15; i++) if (i * i + j * j <= 225 && (i - 7) * (i - 7) + (j + 5) * (j + 5) > 144) im.px(206 + i, 72 + j, CREAM);
  mountains(im, 150, 46, ST1, ST2, 30);
  mountains(im, 164, 30, CEIL, ST1, 24);
  // colina
  im.poly([[-10, 212], [-10, 186], [60, 172], [128, 160], [196, 172], [270, 186], [270, 212]], FOREST);
  for (let i = 0; i < 150; i++) { const x = rnd(W), y = 165 + rnd(45); if (im.get(x, y) === FOREST) im.px(x, y, rnd(3) ? GRASS : K); }
  castle(im, 128, 162, 1);
  // estrada
  im.poly([[122, 162], [134, 162], [176, 212], [86, 212]], ST2);
  im.poly([[126, 162], [130, 162], [140, 212], [122, 212]], ST3);
  // arvores
  for (const [x, y] of [[24, 196], [48, 188], [214, 192], [236, 200], [196, 186]]) {
    im.rect(x - 1, y - 2, 3, 8, WOOD); im.poly([[x - 9, y], [x, y - 22], [x + 9, y]], K); im.poly([[x - 7, y - 3], [x, y - 20], [x + 7, y - 3]], FOREST);
  }
  bands(im);
  return im;
}
function help() {
  const im = new Img(); seed = 5;
  im.grad(0, 212, NAVY, K);
  frame(im);
  im.rect(6, 6, 244, 200, K);
  // cantos com icones
  const sword = (x, y) => { im.rect(x + 3, y, 2, 14, WHITE); im.rect(x, y + 14, 8, 2, GOLD); im.rect(x + 3, y + 16, 2, 5, WOOD); im.rect(x + 2, y + 21, 4, 2, GOLD); };
  const shield = (x, y) => { im.poly([[x, y], [x + 16, y], [x + 16, y + 10], [x + 8, y + 20], [x, y + 10]], BLUE); im.box(x, y, 17, 8, GOLD); im.rect(x + 7, y + 2, 3, 14, GOLD); };
  const chest = (x, y) => { im.rect(x, y + 6, 18, 12, WOOD); im.rect(x, y, 18, 7, ST4); im.rect(x, y + 6, 18, 2, GOLD); im.rect(x + 7, y + 8, 4, 4, GOLD); };
  const amulet = (x, y) => { im.box(x + 2, y, 10, 8, GOLD); im.line(x + 2, y + 8, x + 7, y + 14, GOLD); im.line(x + 12, y + 8, x + 7, y + 14, GOLD); im.disc(x + 7, y + 18, 5, RED); im.disc(x + 6, y + 17, 1, WHITE); };
  sword(14, 14); shield(226, 14); chest(14, 176); amulet(226, 172);
  // separadores
  im.rect(36, 30, 184, 1, WOOD); im.rect(36, 32, 184, 1, ST2);
  im.rect(36, 186, 184, 1, WOOD); im.rect(36, 188, 184, 1, ST2);
  return im;
}
function outro() {
  const im = new Img(); seed = 9;
  // aurora: sol nascendo atras do castelo
  im.grad(0, 70, FLOOR, BLUE); im.grad(70, 120, BLUE, CREAM); im.grad(120, 150, CREAM, GOLD);
  for (let a = 0; a < 16; a++) {                                                    // raios
    const ang = Math.PI * (a / 15), cx = 128, cy = 150;
    im.line(cx, cy, Math.round(cx + Math.cos(ang) * 260), Math.round(cy - Math.sin(ang) * 260), a & 1 ? GOLD : CREAM);
  }
  im.grad(0, 70, FLOOR, BLUE); im.grad(70, 100, BLUE, CREAM);
  im.disc(128, 150, 24, WHITE); im.disc(128, 150, 20, GOLD);
  mountains(im, 156, 40, ST2, ST3, 28);
  im.poly([[-10, 212], [-10, 186], [60, 176], [128, 168], [196, 176], [270, 186], [270, 212]], GRASS);
  for (let i = 0; i < 200; i++) { const x = rnd(W), y = 172 + rnd(40); if (im.get(x, y) === GRASS) im.px(x, y, rnd(3) ? FOREST : GOLD); }
  castle(im, 128, 170, 1);
  knight(im, 54, 192, 1);
  knight(im, 202, 192, -1);
  bands(im);
  return im;
}

//-----------------------------------------------------------------------------
// Telas de historia (abertura): faixa de titulo (0..22), figura (24..142), faixa de texto (143..211)
function storyFrame(im) {
  im.rect(0, 0, W, 23, K); im.rect(0, 23, W, 1, GOLD);
  im.rect(0, 143, W, 69, K); im.rect(0, 142, W, 1, WOOD); im.rect(0, 143, W, 1, GOLD);
}
function story1() { // a Idade de Ouro
  const im = new Img(); seed = 21;
  im.grad(24, 96, BLUE, CREAM);
  im.disc(196, 56, 15, GOLD); im.disc(196, 56, 11, WHITE);
  for (let i = 0; i < 12; i++) { const a = i * Math.PI / 6; im.line(196 + Math.cos(a) * 19, 56 + Math.sin(a) * 19, 196 + Math.cos(a) * 27, 56 + Math.sin(a) * 27, GOLD); }
  for (const [x, y] of [[40, 48], [120, 40], [84, 62]]) { im.disc(x, y, 7, WHITE); im.disc(x + 9, y + 2, 6, WHITE); im.disc(x - 9, y + 2, 5, CREAM); }
  mountains(im, 108, 26, ST2, ST3, 40);
  im.poly([[-10, 143], [-10, 112], [60, 100], [128, 108], [200, 98], [270, 112], [270, 143]], GRASS);
  im.poly([[-10, 143], [-10, 126], [80, 118], [170, 124], [270, 116], [270, 143]], FOREST);
  for (let i = 0; i < 90; i++) { const x = rnd(W), y = 104 + rnd(38); if (im.get(x, y) === GRASS) im.px(x, y, rnd(3) ? FOREST : GOLD); }
  castle(im, 128, 112, 0.5);
  for (const [x, y] of [[40, 128], [64, 124], [200, 124], [226, 130]]) {
    im.rect(x, y, 12, 9, ST4); im.poly([[x - 2, y], [x + 6, y - 7], [x + 14, y]], RED); im.rect(x + 5, y + 3, 3, 6, WOOD);
  }
  for (const [x, y] of [[20, 134], [100, 132], [160, 134], [244, 136]]) { im.rect(x, y - 2, 2, 5, WOOD); im.poly([[x - 6, y], [x + 1, y - 14], [x + 8, y]], FOREST); }
  for (const [x, y] of [[100, 76], [112, 70], [90, 82]]) { im.line(x, y, x + 3, y - 2, WHITE); im.line(x + 3, y - 2, x + 6, y, WHITE); }
  storyFrame(im);
  return im;
}
function story2() { // a Era Sombria
  const im = new Img(); seed = 33;
  im.grad(24, 100, K, CEIL);
  im.disc(214, 52, 14, RED); im.disc(218, 49, 11, CEIL);
  stars(im, 26, 80, 40);
  mountains(im, 118, 40, ST1, CEIL, 34);
  im.poly([[-10, 143], [-10, 122], [70, 114], [150, 120], [270, 112], [270, 143]], FLOOR);
  for (let i = 0; i < 90; i++) { const x = rnd(W), y = 120 + rnd(22); if (im.get(x, y) === FLOOR) im.px(x, y, rnd(2) ? K : CEIL); }
  im.poly([[80, 143], [84, 96], [100, 80], [128, 74], [156, 80], [172, 96], [176, 143]], ST1);
  im.poly([[92, 143], [94, 102], [106, 90], [128, 86], [150, 90], [162, 102], [164, 143]], K);
  im.disc(128, 62, 17, CREAM); im.rect(118, 74, 20, 10, CREAM);
  im.rect(118, 58, 7, 8, K); im.rect(131, 58, 7, 8, K); im.poly([[125, 70], [128, 64], [131, 70]], K);
  for (let i = 0; i < 4; i++) im.rect(119 + i * 5, 76, 3, 7, K);
  im.rect(120, 61, 3, 3, RED); im.rect(133, 61, 3, 3, RED);
  im.rect(104, 112, 4, 4, RED); im.rect(116, 112, 4, 4, RED); im.rect(138, 112, 4, 4, RED); im.rect(150, 112, 4, 4, RED);
  for (const [x, y] of [[36, 60], [60, 78], [226, 84], [200, 66]]) {
    im.poly([[x, y], [x + 6, y - 5], [x + 12, y], [x + 6, y - 2]], K); im.line(x, y, x + 12, y, CREAM);
  }
  im.line(40, 26, 52, 52, WHITE); im.line(52, 52, 46, 54, WHITE); im.line(46, 54, 58, 80, WHITE);
  for (const [x, y] of [[26, 136], [50, 134], [206, 136], [232, 138]]) { im.rect(x, y - 12, 4, 12, CREAM); im.disc(x + 2, y - 15, 3, CREAM); im.rect(x - 3, y - 10, 10, 2, CREAM); }
  storyFrame(im);
  return im;
}
function story3() { // Lord British
  const im = new Img(); seed = 45;
  im.grad(24, 142, CREAM, GOLD);
  for (let a = 0; a < 18; a++) { const ang = Math.PI * (a / 17); im.line(128, 142, Math.round(128 + Math.cos(ang) * 200), Math.round(142 - Math.sin(ang) * 200), a & 1 ? WHITE : CREAM); }
  im.disc(128, 142, 34, GOLD); im.disc(128, 142, 28, WHITE);
  im.poly([[44, 143], [48, 118], [78, 106], [178, 106], [208, 118], [212, 143]], BLUE);
  im.poly([[100, 106], [128, 143], [156, 106]], ST4);
  for (let r = 0; r < 4; r++) for (let c = 0; c < 6; c++) im.disc(112 + c * 6 + (r & 1) * 3, 114 + r * 6, 1, ST2);
  im.poly([[86, 52], [84, 92], [96, 110], [160, 110], [172, 92], [170, 52]], ST4);
  im.disc(128, 74, 25, CREAM);
  im.poly([[104, 82], [108, 100], [128, 108], [148, 100], [152, 82], [140, 92], [116, 92]], WOOD);
  im.rect(112, 66, 10, 3, WOOD); im.rect(134, 66, 10, 3, WOOD); im.rect(115, 70, 4, 3, K); im.rect(137, 70, 4, 3, K);
  im.rect(126, 72, 3, 10, ST4); im.rect(122, 86, 12, 2, RED);
  im.poly([[100, 52], [104, 34], [114, 44], [128, 28], [142, 44], [152, 34], [156, 52]], GOLD); im.rect(100, 50, 56, 4, GOLD);
  im.rect(126, 32, 4, 4, RED); im.rect(108, 42, 3, 3, RED); im.rect(145, 42, 3, 3, RED);
  im.rect(190, 52, 3, 92, WOOD); im.poly([[184, 52], [191, 30], [199, 52]], GOLD);
  for (const x of [28, 224]) { im.rect(x, 30, 2, 40, ST3); im.poly([[x + 2, 30], [x + 22, 38], [x + 2, 48]], x < 100 ? RED : BLUE); }
  storyFrame(im);
  return im;
}

//-----------------------------------------------------------------------------
// RLE por byte: 0..127 = literal (n+1 bytes); 128..255 = repeticao ((b&127)+1 vezes o proximo byte)
function rle(b) {
  const out = []; let i = 0; const n = b.length;
  while (i < n) {
    let j = i; while (j + 1 < n && b[j + 1] === b[i] && j - i < 127) j++;
    const run = j - i + 1;
    if (run >= 3) { out.push(0x80 | (run - 1), b[i]); i = j + 1; continue; }
    const lit = []; let k = i;
    while (k < n && lit.length < 128) { if (k + 2 < n && b[k] === b[k + 1] && b[k + 1] === b[k + 2]) break; lit.push(b[k]); k++; }
    if (!lit.length) { lit.push(b[i]); k = i + 1; }
    out.push(lit.length - 1, ...lit); i = k;
  }
  return Buffer.from(out);
}

const out = process.argv[2];
if (!out) { console.error('uso: node gen_art.js art.seg [dir-png]'); process.exit(1); }
// ordem = ART_* em akalabeth.h
const imgs = [['intro', intro()], ['help', help()], ['outro', outro()], ['story1', story1()], ['story2', story2()], ['story3', story3()]];
const comp = imgs.map(([n, im]) => ({ name: n, data: rle(im.pack()) }));
// segmentos de 16 KB: diretorio ([n] + n x {seg, off, len}) no inicio do segmento 0; imagens empacotadas sem atravessar segmentos
const SEG = 16384;
const segs = [Buffer.alloc(SEG)]; const used = [1 + 5 * comp.length];
const dir = [comp.length];
for (const c of comp) {
  if (c.data.length > SEG) throw new Error('imagem maior que um segmento');
  let s = segs.length - 1;
  if (used[s] + c.data.length > SEG) { segs.push(Buffer.alloc(SEG)); used.push(0); s++; }
  c.data.copy(segs[s], used[s]);
  dir.push(s, used[s] & 255, used[s] >> 8, c.data.length & 255, c.data.length >> 8);
  used[s] += c.data.length;
}
Buffer.from(dir).copy(segs[0], 0);
fs.writeFileSync(out, Buffer.concat(segs));
fs.writeFileSync(out.replace(/\.seg$/, '.json'), JSON.stringify({ lens: used }));
console.log(`${out}: ${comp.map(c => c.name + ' ' + c.data.length).join(', ')} -> ${segs.length} segmentos (usado: ${used.join(', ')})`);

// previa em PNG (x3 horizontal nao: 1:1) para conferir
if (process.argv[3]) {
  const zlib = require('zlib');
  const crcT = Array.from({ length: 256 }, (_, n) => { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1; return c >>> 0; });
  const crc = b => { let c = 0xffffffff; for (const x of b) c = crcT[(c ^ x) & 255] ^ (c >>> 8); return (c ^ 0xffffffff) >>> 0; };
  const chunk = (t, d) => { const l = Buffer.alloc(4); l.writeUInt32BE(d.length); const td = Buffer.concat([Buffer.from(t), d]); const c = Buffer.alloc(4); c.writeUInt32BE(crc(td)); return Buffer.concat([l, td, c]); };
  for (const [name, im] of imgs) {
    const S = 3, w = W * 2, h = H * 2;   // 2x
    const raw = Buffer.alloc((w * 3 + 1) * h);
    for (let y = 0; y < h; y++) { raw[y * (w * 3 + 1)] = 0; for (let x = 0; x < w; x++) { const c = PAL[im.p[(y >> 1) * W + (x >> 1)]]; const o = y * (w * 3 + 1) + 1 + x * 3; raw[o] = Math.min(255, c[0] * 36); raw[o + 1] = Math.min(255, c[1] * 36); raw[o + 2] = Math.min(255, c[2] * 36); } }
    const ih = Buffer.alloc(13); ih.writeUInt32BE(w, 0); ih.writeUInt32BE(h, 4); ih[8] = 8; ih[9] = 2;
    fs.writeFileSync(`${process.argv[3]}/${name}.png`, Buffer.concat([Buffer.from([137,80,78,71,13,10,26,10]), chunk('IHDR', ih), chunk('IDAT', zlib.deflateSync(raw)), chunk('IEND', Buffer.alloc(0))]));
  }
}
