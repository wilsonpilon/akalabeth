// Gera content/AKALABET.DAT: telas pixel art (SCREEN 5, 256x212, paleta do jogo) para a RAM mapeada.
// Cada tela = 2 segmentos de 16 KB (linhas 0..127, linhas 128..211 + preenchimento). Ordem: INTRO, HELP, OUTRO.
// Uso: node tools/gen_art.js content/AKALABET.DAT [prefixo.png-dir-opcional]
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
const out = process.argv[2];
if (!out) { console.error('uso: node gen_art.js saida.dat [dir-ppm]'); process.exit(1); }
const imgs = [['intro', intro()], ['help', help()], ['outro', outro()]];
const bufs = [];
for (const [name, im] of imgs) {
  const raw = im.pack();                                         // 27136 bytes
  const a = Buffer.alloc(16384), b = Buffer.alloc(16384);
  raw.copy(a, 0, 0, 16384); raw.copy(b, 0, 16384, raw.length);
  bufs.push(a, b);
  
}
fs.writeFileSync(out, Buffer.concat(bufs));
console.log(`${out}: ${bufs.length} segmentos, ${bufs.length * 16} KB`);

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
