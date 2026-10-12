//=============================================================================
// AKALABETH MSX2 - RAM store: assets kept in MSX-DOS 2 mapper segments (16 KB each)
//
// The program occupies the four CPU pages, so a segment cannot simply be left mapped.
// rs_exec() switches page 2 only for the duration of one copy (interrupts off), and it
// must itself run from page 0/1: this module is linked first so it sits low in memory.
//=============================================================================
#include "akalabeth.h"
#include "dos.h"
#include "dos_mapper.h"
#include "ramstore.h"

enum { RS_PUT = 0, RS_GET = 1, RS_VRAM = 2, RS_RLE = 3 };

static u8  s_rsSeg[RS_SEGS];			// mapper segment numbers
static u8  s_rsSlot;					// slot of the mapper that supplied them
static u8  s_rsCount;
bool g_rsOk = FALSE;

// parameters of rs_exec (kept in page 3 RAM)
static u8  g_rsSeg, g_rsMode, g_rsOld, g_rsVh;
static u16 g_rsOff, g_rsLen, g_rsVa;
static u8* g_rsPtr;

static void rs_exec(void) __naked
{
	__asm
		push	ix
		di
		ld		hl, (_g_DOS_JumpTable)
		ld		de, #0x27					; GET_P2
		add		hl, de
		ld		de, #rs_r1
		push	de
		jp		(hl)
	rs_r1:
		di
		ld		(_g_rsOld), a
		ld		a, (_g_rsSeg)
		out		(#0xFE), a					; page 2 -> our segment (direct: the DOS routines may enable interrupts)
		ld		bc, (_g_rsLen)
		ld		hl, (_g_rsOff)
		ld		de, #0x8000
		add		hl, de
		ld		a, (_g_rsMode)
		or		a, a
		jr		z, rs_put
		dec		a
		jr		z, rs_get
		dec		a							; 0 = raw to VRAM, 1 = RLE to VRAM
		ld		e, a
		ld		a, (_g_rsVh)				; VRAM write address (17 bit)
		out		(#0x99), a
		ld		a, #0x8E
		out		(#0x99), a
		ld		a, (_g_rsVa)
		out		(#0x99), a
		ld		a, (_g_rsVa + 1)
		and		a, #0x3F
		or		a, #0x40
		out		(#0x99), a
		ld		a, e
		or		a, a
		jr		nz, rs_rle
		ld		a, b						; len is a multiple of 256
		ld		d, a
		ld		c, #0x98
		ld		b, #0
	rs_v1:
		otir
		dec		d
		jr		nz, rs_v1
		jr		rs_done
	rs_rle:									; RLE: 0..127 = n+1 literals, 128..255 = (n&127)+1 copies of the next byte
		ld		d, b
		ld		e, c						; DE = bytes left in the segment
		ld		c, #0x98
	rl_next:
		ld		a, d
		or		a, e
		jr		z, rs_done
		ld		a, (hl)
		inc		hl
		dec		de
		bit		7, a
		jr		nz, rl_run
		inc		a
		ld		b, a
		ld		a, e
		sub		a, b
		ld		e, a
		jr		nc, rl_lit
		dec		d
	rl_lit:
		otir
		jr		rl_next
	rl_run:
		and		a, #0x7F
		inc		a
		ld		b, a
		ld		a, (hl)
		inc		hl
		dec		de
	rl_out:
		out		(c), a
		djnz	rl_out
		jr		rl_next
	rs_put:
		ld		e, l
		ld		d, h
		ld		hl, (_g_rsPtr)
		ldir
		jr		rs_done
	rs_get:
		ld		de, (_g_rsPtr)
		ldir
	rs_done:
		ld		a, (_g_rsOld)
		out		(#0xFE), a					; page 2 back to the program
		ei
		pop		ix
		ret
	_rs_exec_end::
	__endasm;
}
extern u8 rs_exec_end;					// label (see above)
#define RS_EXEC_END		((u16)&rs_exec_end)

static void rs_call(u8 mode, u8 si, u16 off, u16 len)
{
	g_rsMode = mode; g_rsSeg = s_rsSeg[si]; g_rsOff = off; g_rsLen = len;
	rs_exec();
}

void rs_put(u8 si, u16 off, const u8* src, u16 len)
{
	g_rsPtr = (u8*)src;
	rs_call(RS_PUT, si, off, len);
}

void rs_get(u8 si, u16 off, u8* dst, u16 len)
{
	g_rsPtr = dst;
	rs_call(RS_GET, si, off, len);
}

// copy to VRAM (len multiple of 256) at 17-bit address va
void rs_to_vram(u8 si, u16 off, u16 len, u32 va)
{
	g_rsVh = (u8)(va >> 14);
	g_rsVa = (u16)(va & 0x3FFF);
	rs_call(RS_VRAM, si, off, len);
}

// RLE-compressed image (len bytes at segment si, offset off) decoded straight into VRAM
void rs_rle_vram(u8 si, u16 off, u16 len, u32 va)
{
	g_rsVh = (u8)(va >> 14);
	g_rsVa = (u16)(va & 0x3FFF);
	rs_call(RS_RLE, si, off, len);
}

bool rs_init(u8 count)
{
	g_rsOk = FALSE;
	s_rsCount = 0;
	if (count > RS_SEGS) return FALSE;
	if ((u16)&rs_exec >= 0x8000 || RS_EXEC_END >= 0x8000) return FALSE;	// would vanish with page 2
	if (!DOSMapper_Init()) return FALSE;
	DOS_VarTable* vt = DOSMapper_GetVarTable();
	if (vt->FreeSeg < count + 2) return FALSE;							// leave some for the system
	for (u8 i = 0; i < count; i++)
	{
		DOS_Segment sg;
		if (!DOSMapper_Alloc(DOS_ALLOC_USER, vt->Slot | DOS_SEGSLOT_THISFIRST, &sg))
		{
			rs_free();
			return FALSE;
		}
		s_rsSeg[i] = sg.Number;
		s_rsSlot = sg.Slot;
		s_rsCount++;
	}
	g_rsOk = TRUE;
	return TRUE;
}

void rs_free(void)
{
	for (u8 i = 0; i < s_rsCount; i++) DOSMapper_Free(s_rsSeg[i], s_rsSlot);
	s_rsCount = 0;
	g_rsOk = FALSE;
}

