#!/bin/bash
# Gera content/AKALABET.DAT (telas pixel art RLE + musica completa, em segmentos de 16 KB) e content/music_gen.h
cd "$(dirname "$0")/.." || exit 1
node tools/gen_art.js content/art.seg "$1" || exit 1
node tools/gen_music.js content/music_gen.h content/music.seg || exit 1
node tools/gen_dat.js content/AKALABET.DAT || exit 1
ls -l content/AKALABET.DAT
