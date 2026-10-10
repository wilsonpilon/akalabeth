// Gera content/data_gen.h a partir do basic.bas do Akalabeth (porte GW-BASIC de Nanochess)
//  - polilinhas dos 10 monstros (linhas 300-400 e 3087-3089)
//  - tabela de divisao 8.8 (a/DI) para projecao dos monstros
//  - tabelas de perspectiva (linhas 50-62) reparametrizadas para 256 px de largura
// Uso: node tools/gen_data.js ../../../basic.bas content/data_gen.h
const fs = require('fs');
const src = fs.readFileSync(process.argv[2], 'latin1').split(/\r?\n/);
const outPath = process.argv[3];

const lines = {};
for (const l of src) {
  const m = l.match(/^(\d+)\s(.*)$/);
  if (m) lines[+m[1]] = m[2];
}

// ---- 1. monstros --------------------------------------------------------
// ON MC GOTO 300,310,320,330,340,350,360,370,380,390
const starts = [300, 310, 320, 330, 340, 350, 360, 370, 380, 390];
const names  = ['SKELETON','THIEF','GIANT_RAT','ORC','VIPER','CARRION_CRAWLER','GREMLIN','MIMIC','DAEMON','BALROG'];
// o corpo de cada monstro: linhas starts[i]..(proximo start-1), exceto o 'GOTO 490' final
// Daemon continua em 3087..3089 (via GOTO 3087 na linha 388)
function bodyLines(i) {
  const a = starts[i];
  const b = i + 1 < starts.length ? starts[i + 1] : 401;
  const r = [];
  for (let n = a; n < b; n++) if (lines[n] !== undefined) r.push(lines[n]);
  if (i === 8) for (let n = 3087; n <= 3089; n++) r.push(lines[n]);
  return r;
}

// expressao -> {ax, half, anchor}
function parseX(e) {
  e = e.replace(/\s/g, '');
  // C [+-] n /DI [+.5]    | C | C-7+DI (erro de digitacao do original -> C-7/DI)
  let m = e.match(/^C(?:([+-])(\d+)(?:\/DI|\+DI))?(\+\.5)?$/);
  if (!m) throw new Error('X? ' + e);
  const ax = m[1] ? (m[1] === '-' ? -1 : 1) * +m[2] : 0;
  return { a: ax, half: !!m[3] };
}
function parseY(e) {
  e = e.replace(/\s/g, '');
  if (e === '79-YY%(DI)') return { a: 0, top: true };   // ancorado no teto
  let m = e.match(/^B(?:-(\d+)\/DI)?$/);
  if (!m) throw new Error('Y? ' + e);
  return { a: m[1] ? +m[1] : 0, top: false };
}
function splitPt(s) { // "(x,y)" ou "x,y"
  const m = s.match(/^\((.+?),(.+)\)$/);
  if (!m) throw new Error('PT? ' + s);
  return [parseX(m[1]), parseY(m[2])];
}

const shapes = [];
for (let i = 0; i < 10; i++) {
  const ops = [];
  if (i === 7) { shapes.push(ops); continue; }   // mimico = bau (procedural)
  let body = bodyLines(i).join(':');
  body = body.replace(/GOTO \d+/g, '').replace(/ON MC.*/, '');
  for (const stmt of body.split(/:(?=LINE|PSET)/)) {
    const s = stmt.trim().replace(/:$/, '');
    if (!s) continue;
    let m;
    if ((m = s.match(/^LINE\s*(\(.+?\))-(\(.+\))$/))) {            // LINE (a)-(b)
      const p = splitPt(m[1]), q = splitPt(m[2]);
      ops.push(['M', p]); ops.push(['L', q]);
    } else if ((m = s.match(/^LINE\s*-(\(.+\))$/))) {                // LINE -(b)
      ops.push(['L', splitPt(m[1])]);
    } else if ((m = s.match(/^PSET\s*(\(.+\))$/))) {
      ops.push(['D', splitPt(m[1])]);
    } else if (s) {
      throw new Error('stmt? ' + s);
    }
  }
  shapes.push(ops);
}

// ---- 2. emitir ----------------------------------------------------------
let o = '// GERADO por tools/gen_data.js a partir de basic.bas -- NAO EDITAR\n';
o += '// Dados derivados do AKALABETH (Richard Garriott, 1979) via porte GW-BASIC de Nanochess (2019)\n#pragma once\n#include "core.h"\n\n';

// formato: 3 bytes por ponto: [op|flags] [x] [y]
//   op   : 0=move 1=line 2=dot ; bit2 (4)=+0.5 em X ; bit3 (8)=Y ancorado no teto
o += 'enum { SH_MOVE = 0, SH_LINE = 1, SH_DOT = 2, SH_HALF = 4, SH_TOP = 8 };\n';
o += 'typedef struct { i8 op; i8 x; i8 y; } ShapePt;\n';
const flat = [];
const idx = [];
for (let i = 0; i < 10; i++) {
  idx.push(flat.length);
  for (const [k, [px, py]] of shapes[i]) {
    const op = (k === 'M' ? 0 : k === 'L' ? 1 : 2) | (px.half ? 4 : 0) | (py.top ? 8 : 0);
    flat.push([op, px.a, py.a]);
  }
}
idx.push(flat.length);
o += `#define SHAPE_PTS ${flat.length}\n`;
o += 'const ShapePt g_Shapes[SHAPE_PTS] = {\n';
for (let i = 0; i < flat.length; i += 6)
  o += '\t' + flat.slice(i, i + 6).map(p => `{${p[0]},${p[1]},${p[2]}}`).join(',') + ',\n';
o += '};\n';
o += 'const u16 g_ShapeStart[11] = { ' + idx.join(', ') + ' };\n\n';

// divisao 8.8: DIV88[DI-1][a+96] = round(a*256/DI)
o += '// a*256/DI para a em [-96,95] e DI em [1,9]\n';
o += 'const i16 g_Div88[9][192] = {\n';
for (let d = 1; d <= 9; d++) {
  const row = [];
  for (let a = -96; a < 96; a++) row.push(Math.round(a * 256 / d));
  o += '\t{' + row.join(',') + '},\n';
}
o += '};\n\n';

// ---- 3. perspectiva (linhas 50-62), reparametrizada p/ 256 px ------------
const W = 256, CX = 127, CY = 79;
const rnd = v => Math.floor(v + 0.5);
const XX = [CX], YY = [CY], XXo = [139];
for (let x = 2; x <= 20; x += 2) {
  const xo = rnd(Math.atan(1 / x) / Math.atan(1) * 140);       // original (280 px)
  XXo[x / 2] = xo;
  XX[x / 2] = rnd(Math.atan(1 / x) / Math.atan(1) * 128);      // 256 px
  YY[x / 2] = rnd(xo * 4 / 7);                                  // altura mantida
}
const PE = [], CD = [], LD = [], FT = [], LA = [];
for (let k = 0; k <= 10; k++) {
  PE[k] = k === 0 ? [0, W - 1, 0, 159]
                  : [CX - XX[k], CX + XX[k], CY - YY[k], CY + YY[k]];
}
for (let k = 1; k <= 10; k++)
  CD[k] = [rnd(CX - XX[k] / 3), rnd(CX + XX[k] / 3), rnd(CY - YY[k] * .7), CY + YY[k]];
CD[0] = [0, 0, 0, 0];
for (let k = 0; k <= 9; k++) {
  const l0 = rnd((PE[k][0] * 2 + PE[k + 1][0]) / 3), l1 = rnd((PE[k][0] + 2 * PE[k + 1][0]) / 3);
  const w = l0 - PE[k][0];
  let t2 = rnd(PE[k][2] + w * 4 / 7), t3 = rnd(PE[k][2] + 2 * w * 4 / 7);
  const b4 = rnd((PE[k][3] * 2 + PE[k + 1][3]) / 3), b5 = rnd((PE[k][3] + 2 * PE[k + 1][3]) / 3);
  t2 = rnd(b4 - (b4 - t2) * .8); t3 = rnd(b5 - (b5 - t3) * .8);
  if (t3 === b4) t3--;
  LD[k] = [l0, l1, t2, t3, b4, b5];
}
LD[10] = [0, 0, 0, 0, 0, 0];
for (let k = 0; k <= 9; k++) {
  FT[k] = [rnd(CX - XX[k] / 3), rnd(CX + XX[k] / 3), rnd(CX - XX[k + 1] / 3), rnd(CX + XX[k + 1] / 3),
           rnd(CY + (YY[k] * 2 + YY[k + 1]) / 3), rnd(CY + (YY[k] + 2 * YY[k + 1]) / 3)];
}
FT[10] = [0, 0, 0, 0, 0, 0];
for (let k = 0; k <= 9; k++) {
  const l0 = rnd((FT[k][0] * 2 + FT[k][1]) / 3), l1 = rnd((FT[k][0] + 2 * FT[k][1]) / 3);
  const b = FT[k][4];
  LA[k] = [l0, l1, 159 - b, b];
}
LA[10] = [0, 0, 0, 0];
const tab = (name, t, n, c) => {
  let s = `const i16 ${name}[11][${n}] = {\n`;
  for (const r of t) s += '\t{' + r.join(',') + '},\n';
  return s + '};\n';
};
o += '// perspectiva: PE=[L,R,T,B] CD=[L,R,T,B] LD=[x0,x1,y2,y3,y4,y5] FT=[x0,x1,x2,x3,y4,y5] LA=[lx,rx,top,bot]\n';
o += tab('g_PE', PE, 4) + tab('g_CD', CD, 4) + tab('g_LD', LD, 6) + tab('g_FT', FT, 6) + tab('g_LA', LA, 4);
o += `#define VIEW_CX ${CX}\n#define VIEW_CY ${CY}\n`;
o += 'const i8 g_YY[11] = { ' + YY.join(', ') + ' };\n';
fs.writeFileSync(outPath, o);
console.log('shapes:', shapes.map(s => s.length).join(' '), ' total pts:', flat.length);
console.log('XX', XX.join(','), '\nYY', YY.join(','));
