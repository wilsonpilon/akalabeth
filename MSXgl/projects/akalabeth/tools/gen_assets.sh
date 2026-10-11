#!/bin/bash
# Gera content/AKALABET.DAT (telas pixel art + musica completa, em segmentos de 16 KB) e content/music_gen.h
cd "$(dirname "$0")/.." || exit 1
node tools/gen_art.js content/art.seg "$1" || exit 1
node tools/gen_music.js content/music_gen.h content/music.seg || exit 1
cat content/art.seg content/music.seg > content/AKALABET.DAT
ls -l content/AKALABET.DAT
