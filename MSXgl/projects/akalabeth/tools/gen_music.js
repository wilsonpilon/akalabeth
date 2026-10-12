// Gera content/music_gen.h (versoes curtas de reserva, no codigo) e o segmento de musica completo (content/music.seg).
// Temas originais para o PSG (3 vozes: melodia, baixo, harmonia).
// Notacao: "A4:3" = nota A, oitava 4, 3 passos; "C#4:1" sustenido; "A#1" = si bemol; "R:2" = pausa.
// Passo = colcheia; velocidade em ticks (1/50 s) por passo. Todas as vozes de uma musica devem ter o mesmo tamanho.
// Uso: node tools/gen_music.js content/music_gen.h content/music.seg
const fs = require('fs');

// Cada secao tem 8 compassos de 8 passos. "code" = so a 1a secao (reserva no codigo, sem RAM mapeada);
// "full" = musica completa (segmento de musica de AKALABET.DAT).
const S = {
  title_A: {
    mel: ['A4:3 C5:1 E5:2 C5:2', 'B4:3 D5:1 G5:2 D5:2', 'A4:3 C5:1 F5:2 C5:2', 'G#4:3 B4:1 E5:4',
          'E5:3 D5:1 C5:2 A4:2', 'D5:3 C5:1 B4:2 G4:2', 'F5:2 E5:2 D5:2 C5:2', 'B4:3 G#4:1 A4:4'],
    bas: ['A2:4 E3:4', 'G2:4 D3:4', 'F2:4 C3:4', 'E2:4 B2:4', 'A2:4 E3:4', 'G2:4 D3:4', 'F2:4 C3:4', 'E2:4 B2:4'],
    arp: ['A3:1 C4:1 E4:1 C4:1 A3:1 C4:1 E4:1 C4:1', 'G3:1 B3:1 D4:1 B3:1 G3:1 B3:1 D4:1 B3:1',
          'F3:1 A3:1 C4:1 A3:1 F3:1 A3:1 C4:1 A3:1', 'E3:1 G#3:1 B3:1 G#3:1 E3:1 G#3:1 B3:1 G#3:1',
          'A3:1 C4:1 E4:1 C4:1 A3:1 C4:1 E4:1 C4:1', 'G3:1 B3:1 D4:1 B3:1 G3:1 B3:1 D4:1 B3:1',
          'F3:1 A3:1 C4:1 A3:1 F3:1 A3:1 C4:1 A3:1', 'E3:1 G#3:1 B3:1 G#3:1 E3:1 G#3:1 B3:1 G#3:1'] },
  title_B: {
    mel: ['E5:2 G5:2 C6:2 G5:2', 'D5:2 G5:2 B5:2 G5:2', 'C5:2 E5:2 A5:2 E5:2', 'A5:3 G5:1 F5:2 E5:2',
          'E5:2 G5:2 C6:2 E6:2', 'D6:3 B5:1 G5:2 D5:2', 'E5:2 G#5:2 B5:2 G#5:2', 'A5:6 R:2'],
    bas: ['C2:4 G2:4', 'G1:4 D2:4', 'A1:4 E2:4', 'F1:4 C2:4', 'C2:4 G2:4', 'G1:4 D2:4', 'E1:4 B1:4', 'A1:4 E2:4'],
    arp: ['C4:1 E4:1 G4:1 E4:1 C4:1 E4:1 G4:1 E4:1', 'B3:1 D4:1 G4:1 D4:1 B3:1 D4:1 G4:1 D4:1',
          'A3:1 C4:1 E4:1 C4:1 A3:1 C4:1 E4:1 C4:1', 'A3:1 C4:1 F4:1 C4:1 A3:1 C4:1 F4:1 C4:1',
          'C4:1 E4:1 G4:1 E4:1 C4:1 E4:1 G4:1 E4:1', 'B3:1 D4:1 G4:1 D4:1 B3:1 D4:1 G4:1 D4:1',
          'G#3:1 B3:1 E4:1 B3:1 G#3:1 B3:1 E4:1 B3:1', 'A3:1 C4:1 E4:1 C4:1 A3:1 C4:1 E4:1 C4:1'] },
  world_A: {
    mel: ['E5:2 G5:1 E5:1 C5:2 E5:2', 'D5:2 G5:1 D5:1 B4:2 D5:2', 'C5:2 E5:1 C5:1 A4:2 C5:2', 'A4:2 C5:2 F5:2 C5:2',
          'G5:2 E5:1 C5:1 E5:2 G5:2', 'B5:2 G5:1 D5:1 B4:2 D5:2', 'A5:2 C5:1 F5:1 E5:2 C5:2', 'D5:2 B4:2 G4:4'],
    bas: ['C2:2 G2:2 C2:2 G2:2', 'G1:2 D2:2 G1:2 D2:2', 'A1:2 E2:2 A1:2 E2:2', 'F1:2 C2:2 F1:2 C2:2',
          'C2:2 G2:2 C2:2 G2:2', 'G1:2 D2:2 G1:2 D2:2', 'F1:2 C2:2 F1:2 C2:2', 'G1:2 D2:2 G1:2 D2:2'],
    arp: ['E4:4 G4:4', 'D4:4 B3:4', 'C4:4 E4:4', 'A3:4 C4:4', 'E4:4 G4:4', 'D4:4 B3:4', 'A3:4 C4:4', 'B3:4 D4:4'] },
  world_B: {
    mel: ['A4:2 C5:2 E5:2 A5:2', 'G5:2 E5:2 B4:2 E5:2', 'F5:2 A5:2 C6:2 A5:2', 'B5:3 A5:1 G5:2 D5:2',
          'C6:2 B5:1 A5:1 E5:2 A5:2', 'A5:2 F5:1 C5:1 F5:2 A5:2', 'G5:2 B5:2 D6:2 B5:2', 'C6:6 R:2'],
    bas: ['A1:2 E2:2 A1:2 E2:2', 'E1:2 B1:2 E1:2 B1:2', 'F1:2 C2:2 F1:2 C2:2', 'G1:2 D2:2 G1:2 D2:2',
          'A1:2 E2:2 A1:2 E2:2', 'F1:2 C2:2 F1:2 C2:2', 'G1:2 D2:2 G1:2 D2:2', 'C2:2 G2:2 C2:2 G2:2'],
    arp: ['C4:4 E4:4', 'B3:4 G3:4', 'A3:4 C4:4', 'B3:4 D4:4', 'C4:4 E4:4', 'A3:4 C4:4', 'B3:4 D4:4', 'E4:4 G4:4'] },
  dungeon_A: {
    mel: ['R:6 D4:2', 'F4:3 E4:1 D4:2 R:2', 'R:4 A3:2 C#4:2', 'D4:4 R:4',
          'R:6 A4:2', 'G4:3 F4:1 E4:2 R:2', 'C#4:2 D4:2 E4:2 F4:2', 'E4:6 R:2'],
    bas: ['D2:2 R:2 D2:2 R:2', 'A#1:2 R:2 A#1:2 R:2', 'A1:2 R:2 A1:2 R:2', 'D2:2 R:2 A1:2 R:2',
          'D2:2 R:2 D2:2 R:2', 'A#1:2 R:2 A#1:2 R:2', 'A1:2 R:2 A1:2 R:2', 'E2:2 R:2 A1:2 R:2'],
    arp: ['A3:8', 'A#3:8', 'A3:8', 'D3:8', 'A3:8', 'A#3:8', 'A3:8', 'E3:8'] },
  dungeon_B: {
    mel: ['R:2 D4:2 F4:2 A4:2', 'A#4:4 R:2 F4:2', 'G4:3 A#4:1 D5:2 R:2', 'C#5:4 E5:2 R:2',
          'D5:2 C5:2 A4:2 F4:2', 'D5:3 C5:1 A#4:2 F4:2', 'E4:2 A4:2 C#5:2 A4:2', 'D4:8'],
    bas: ['D2:2 R:2 D2:2 R:2', 'A#1:2 R:2 A#1:2 R:2', 'G1:2 R:2 G1:2 R:2', 'A1:2 R:2 A1:2 R:2',
          'D2:2 R:2 D2:2 R:2', 'A#1:2 R:2 A#1:2 R:2', 'A1:2 R:2 A1:2 R:2', 'D2:4 A1:4'],
    arp: ['F3:8', 'D3:8', 'D3:8', 'E3:8', 'F3:8', 'D3:8', 'E3:8', 'A3:8'] },
  court_A: {
    mel: ['F5:2 A5:1 F5:1 C5:2 F5:2', 'D5:2 F5:1 D5:1 A#4:2 D5:2', 'E5:2 G5:1 E5:1 C5:2 E5:2', 'A4:2 C5:2 F5:4',
          'D5:2 F5:1 D5:1 A4:2 D5:2', 'A#4:2 D5:2 F5:4', 'G5:2 E5:2 C5:2 A#4:2', 'A4:4 F4:4'],
    bas: ['F2:2 C3:2 F2:2 C3:2', 'A#1:2 F2:2 A#1:2 F2:2', 'C2:2 G2:2 C2:2 G2:2', 'F2:2 C3:2 F2:2 C3:2',
          'D2:2 A2:2 D2:2 A2:2', 'A#1:2 F2:2 A#1:2 F2:2', 'C2:2 G2:2 C2:2 G2:2', 'F2:4 C2:4'],
    arp: ['R:1 C4:1 R:1 C4:1 R:1 C4:1 R:1 C4:1', 'R:1 D4:1 R:1 D4:1 R:1 D4:1 R:1 D4:1',
          'R:1 E4:1 R:1 E4:1 R:1 E4:1 R:1 E4:1', 'R:1 C4:1 R:1 C4:1 R:1 C4:1 R:1 C4:1',
          'R:1 A3:1 R:1 A3:1 R:1 A3:1 R:1 A3:1', 'R:1 D4:1 R:1 D4:1 R:1 D4:1 R:1 D4:1',
          'R:1 E4:1 R:1 E4:1 R:1 E4:1 R:1 E4:1', 'R:1 C4:1 R:1 C4:1 R:1 C4:1 R:1 C4:1'] },
  court_B: {
    mel: ['D5:2 F5:2 A5:2 F5:2', 'G5:2 A#5:2 D6:2 A#5:2', 'E5:2 G5:2 C6:2 G5:2', 'A5:3 G5:1 F5:4',
          'F5:2 A5:2 D6:2 A5:2', 'D6:3 C6:1 A#5:2 F5:2', 'G5:2 E5:2 C5:2 E5:2', 'F5:6 R:2'],
    bas: ['D2:2 A2:2 D2:2 A2:2', 'G1:2 D2:2 G1:2 D2:2', 'C2:2 G2:2 C2:2 G2:2', 'F2:4 C2:4',
          'D2:2 A2:2 D2:2 A2:2', 'A#1:2 F2:2 A#1:2 F2:2', 'C2:2 G2:2 C2:2 G2:2', 'F2:4 C2:4'],
    arp: ['R:1 A3:1 R:1 A3:1 R:1 A3:1 R:1 A3:1', 'R:1 D4:1 R:1 D4:1 R:1 D4:1 R:1 D4:1',
          'R:1 E4:1 R:1 E4:1 R:1 E4:1 R:1 E4:1', 'R:1 C4:1 R:1 C4:1 R:1 C4:1 R:1 C4:1',
          'R:1 A3:1 R:1 A3:1 R:1 A3:1 R:1 A3:1', 'R:1 D4:1 R:1 D4:1 R:1 D4:1 R:1 D4:1',
          'R:1 E4:1 R:1 E4:1 R:1 E4:1 R:1 E4:1', 'R:1 C4:1 R:1 C4:1 R:1 C4:1 R:1 C4:1'] },
  victory_A: {
    mel: ['C5:1 R:1 C5:1 R:1 E5:2 G5:2', 'D5:1 R:1 D5:1 R:1 F5:2 B5:2', 'E5:1 R:1 E5:1 R:1 G5:2 C6:2', 'A5:2 G5:2 F5:2 E5:2',
          'G5:3 E5:1 C5:2 E5:2', 'G5:3 D5:1 B4:2 D5:2', 'A5:2 F5:2 D5:2 G5:2', 'C6:6 R:2'],
    bas: ['C2:2 G2:2 C2:2 G2:2', 'G1:2 D2:2 G1:2 D2:2', 'C2:2 G2:2 C2:2 G2:2', 'F1:2 C2:2 F1:2 C2:2',
          'C2:2 G2:2 C2:2 G2:2', 'G1:2 D2:2 G1:2 D2:2', 'F1:2 G1:2 F1:2 G1:2', 'C2:4 C1:4'],
    arp: ['E4:2 G4:2 E4:2 G4:2', 'F4:2 B3:2 F4:2 B3:2', 'G4:2 C4:2 G4:2 C4:2', 'A3:2 C4:2 A3:2 C4:2',
          'E4:2 G4:2 E4:2 G4:2', 'D4:2 B3:2 D4:2 B3:2', 'D4:2 B3:2 D4:2 B3:2', 'E4:4 G4:4'] },
  victory_B: {
    mel: ['F5:2 A5:2 C6:2 A5:2', 'E5:2 G5:2 C6:2 G5:2', 'D5:2 G5:2 B5:2 G5:2', 'C6:4 G5:2 E5:2',
          'A5:2 C6:2 F6:2 C6:2', 'B5:2 D6:2 G6:2 D6:2', 'E6:2 D6:2 C6:2 G5:2', 'C6:8'],
    bas: ['F1:2 C2:2 F1:2 C2:2', 'C2:2 G2:2 C2:2 G2:2', 'G1:2 D2:2 G1:2 D2:2', 'C2:2 G2:2 C2:4',
          'F1:2 C2:2 F1:2 C2:2', 'G1:2 D2:2 G1:2 D2:2', 'C2:2 G2:2 C2:2 G2:2', 'C2:8'],
    arp: ['A3:2 C4:2 A3:2 C4:2', 'E4:2 G4:2 E4:2 G4:2', 'D4:2 B3:2 D4:2 B3:2', 'E4:2 G4:2 E4:4',
          'A3:2 C4:2 A3:2 C4:2', 'D4:2 B3:2 D4:2 B3:2', 'E4:2 G4:2 E4:2 G4:2', 'E4:8'] },
};
const songs = [
  { id: 'TITLE',   speed: 11, parts: ['title_A', 'title_B', 'title_A'] },
  { id: 'WORLD',   speed: 8,  parts: ['world_A', 'world_B', 'world_A'] },
  { id: 'DUNGEON', speed: 13, parts: ['dungeon_A', 'dungeon_B', 'dungeon_A'] },
  { id: 'COURT',   speed: 9,  parts: ['court_A', 'court_B', 'court_A'] },
  { id: 'VICTORY', speed: 7,  parts: ['victory_A', 'victory_B'] },
];
const cat = (parts, k) => parts.reduce((r, p) => r.concat(S[p][k]), []);
const full = songs.map(s => ({ id: s.id, speed: s.speed, mel: cat(s.parts, 'mel'), bas: cat(s.parts, 'bas'), arp: cat(s.parts, 'arp') }));
const code = songs.map(s => ({ id: s.id, speed: s.speed, mel: S[s.parts[0]].mel, bas: S[s.parts[0]].bas, arp: S[s.parts[0]].arp }));

const SEMI = { C: 0, D: 2, E: 4, F: 5, G: 7, A: 9, B: 11 };
function note(tok) {
  const m = tok.match(/^([A-G])(#?)(\d):(\d+)$/);
  if (tok.startsWith('R:')) return [0, +tok.slice(2)];
  if (!m) throw new Error('nota? ' + tok);
  const n = (+m[3] - 1) * 12 + SEMI[m[1]] + (m[2] ? 1 : 0) + 1;     // C1 = 1
  return [n, +m[4]];
}
function track(bars) {
  const out = []; let total = 0;
  for (const b of bars) for (const t of b.split(/\s+/)) { const [n, l] = note(t); out.push(n, l); total += l; }
  out.push(0xFF, 0);
  return { bytes: out, total };
}

let h = '// GERADO por tools/gen_music.js -- NAO EDITAR\n#pragma once\n#include "core.h"\n\n';
// periodos do PSG: f = 111861 / periodo ; C1 = 32.703 Hz
h += '#define NOTE_COUNT 73\nconst u16 g_NotePeriod[NOTE_COUNT] = { 0';
let row = [];
for (let i = 0; i < 72; i++) {
  const f = 32.7032 * Math.pow(2, i / 12);
  row.push(Math.round(111861 / f));
}
h += ', ' + row.join(', ') + ' };\n\n';
const names = [];
for (const s of code) {
  const t = ['mel', 'bas', 'arp'].map(k => track(s[k]));
  if (!(t[0].total === t[1].total && t[1].total === t[2].total)) throw new Error(`${s.id}: vozes com tamanhos diferentes ${t.map(x => x.total)}`);
  names.push(s.id);
  ['a', 'b', 'c'].forEach((v, i) => { h += `const u8 mus_${s.id.toLowerCase()}_${v}[] = { ${t[i].bytes.join(',')} };\n`; });
}
h += '\ntypedef struct { const u8* tr[3]; u8 speed; } MusicSong;\n';
h += '// reserva no codigo (so a 1a secao); order = MUS_* ids defined in akalabeth.h: ' + names.join(', ') + '\n#define MUS_COUNT ' + names.length + '\n';
h += 'const MusicSong g_Songs[MUS_COUNT] = {\n';
for (const s of code) { const l = s.id.toLowerCase(); h += `\t{ { mus_${l}_a, mus_${l}_b, mus_${l}_c }, ${s.speed} },\n`; }
h += '};\n';
fs.writeFileSync(process.argv[2], h);

// segmento de musica: [n][ (speed, offA, lenA, offB, lenB, offC, lenC) x n ] + trilhas (sem o terminador)
const HS = 13;
const hdr = 1 + HS * full.length;
const body = [];
const hb = Buffer.alloc(hdr);
hb[0] = full.length;
let off = hdr;
full.forEach((s, i) => {
  const t = ['mel', 'bas', 'arp'].map(k => track(s[k]));
  if (!(t[0].total === t[1].total && t[1].total === t[2].total)) throw new Error(`${s.id}: vozes com tamanhos diferentes ${t.map(x => x.total)}`);
  hb[1 + i * HS] = s.speed;
  t.forEach((tr, v) => {
    const bytes = tr.bytes.slice(0, tr.bytes.length - 2);          // sem o terminador FF,0
    hb.writeUInt16LE(off, 2 + i * HS + v * 4); hb.writeUInt16LE(bytes.length, 4 + i * HS + v * 4);
    body.push(Buffer.from(bytes)); off += bytes.length;
  });
  console.log(s.id.padEnd(8), 'compassos:', t[0].total / 8, ' duracao:', (t[0].total * s.speed / 50).toFixed(1) + 's', ' bytes:', t.reduce((a, x) => a + x.bytes.length, 0));
});
if (process.argv[3]) {
  const seg = Buffer.alloc(16384);
  const all = Buffer.concat([hb, ...body]);
  if (all.length > 16384) throw new Error('musica excede 16 KB');
  all.copy(seg);
  fs.writeFileSync(process.argv[3], seg);
  fs.writeFileSync(process.argv[3].replace(/\.seg$/, '.json'), JSON.stringify({ len: all.length }));
  console.log(process.argv[3] + ':', all.length, 'bytes de 16384');
}
