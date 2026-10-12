// Monta content/AKALABET.DAT a partir de content/art.seg + art.json (telas RLE) e content/music.seg + music.json.
// Formato: "AKD2", n, n x u16 (bytes usados por segmento), depois os bytes usados de cada segmento (o ultimo e o de musica).
// Uso: node tools/gen_dat.js content/AKALABET.DAT
const fs = require('fs');
const C = 'content/';
const SEG = 16384;
const art = fs.readFileSync(C + 'art.seg'), artJ = JSON.parse(fs.readFileSync(C + 'art.json'));
const mus = fs.readFileSync(C + 'music.seg'), musJ = JSON.parse(fs.readFileSync(C + 'music.json'));
const parts = [], lens = [];
artJ.lens.forEach((l, i) => { parts.push(art.slice(i * SEG, i * SEG + l)); lens.push(l); });
parts.push(mus.slice(0, musJ.len)); lens.push(musJ.len);
const hdr = Buffer.alloc(5 + 2 * lens.length);
hdr.write('AKD2', 0, 'latin1'); hdr[4] = lens.length;
lens.forEach((l, i) => hdr.writeUInt16LE(l, 5 + 2 * i));
fs.writeFileSync(process.argv[2], Buffer.concat([hdr, ...parts]));
console.log(`${process.argv[2]}: ${lens.length} segmentos, ${lens.join('+')} = ${lens.reduce((a, b) => a + b)} bytes`);
