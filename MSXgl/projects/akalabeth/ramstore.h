//=============================================================================
// AKALABETH MSX2 - RAM store (MSX-DOS 2 memory mapper segments)
//=============================================================================
#pragma once
#include "core.h"

#define RS_SEGS			16					// max segments held (256 KB mapper = 16)
#define RS_SEG_BYTES	16384

extern bool g_rsOk;							// TRUE when the segments are allocated and loaded

bool rs_init(u8 count);						// allocates count segments; FALSE: no mapper / no memory
void rs_free(void);
void rs_put(u8 si, u16 off, const u8* src, u16 len);			// RAM -> segment si
void rs_get(u8 si, u16 off, u8* dst, u16 len);					// segment si -> RAM
void rs_to_vram(u8 si, u16 off, u16 len, u32 va);				// segment si -> VRAM (len multiple of 256)
