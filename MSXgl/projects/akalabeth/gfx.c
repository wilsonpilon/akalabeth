//=============================================================================
// AKALABETH MSX2 - graphics: palette, primitives, 3D dungeon view, overworld
//=============================================================================
#include "akalabeth.h"
#include "draw.h"
#include "font/font_mgl_sample6.h"
#include "content/data_gen.h"

//-----------------------------------------------------------------------------
// Palette
static const u16 g_Pal[16] =
{
	RGB16(0, 0, 0),		// 0  black
	RGB16(0, 0, 2),		// 1  navy
	RGB16(2, 1, 1),		// 2  stone far
	RGB16(3, 2, 2),		// 3
	RGB16(4, 3, 2),		// 4
	RGB16(5, 4, 3),		// 5  stone near
	RGB16(2, 3, 7),		// 6  blue
	RGB16(1, 1, 3),		// 7  floor
	RGB16(1, 0, 1),		// 8  ceiling
	RGB16(7, 6, 3),		// 9  cream
	RGB16(6, 4, 1),		// 10 wood
	RGB16(7, 1, 1),		// 11 red
	RGB16(1, 5, 1),		// 12 grass
	RGB16(0, 3, 1),		// 13 forest
	RGB16(7, 5, 0),		// 14 gold
	RGB16(7, 7, 7),		// 15 white
};

// Drawing target (added to every y): 0 = page 0, 256 = page 1
static u16 g_oy = 0;
static u8 s_front = 0;				// page being displayed (R#2)

// Show page p (0/1). The caller must have finished drawing on it.
static void show_page(u8 p)
{
	if (p == s_front) return;
	VDP_CommandWait();
	VDP_RegWrite(2, 0x1F | (p << 5));
	s_front = p;
}

void gfx_set_palette_fade(u8 level)
{
	for (u8 i = 0; i < 16; i++)
	{
		u16 c = g_Pal[i];
		u8 g = (c >> 8) & 7, r = (c >> 4) & 7, b = c & 7;
		g = (g * level + 3) / 7; r = (r * level + 3) / 7; b = (b * level + 3) / 7;
		VDP_SetPaletteEntry(i, RGB16(r, g, b));
	}
}

void gfx_init(void)
{
	VDP_SetMode(VDP_MODE_SCREEN5);
	VDP_SetColor(C_BLACK);
	VDP_EnableDisplay(FALSE);
	VDP_DisableSprite();			// the game has no sprites: their tables (page 0, rows 212..255) hold the tile atlas
	VDP_ClearVRAM();
	gfx_set_palette_fade(7);
	VDP_EnableDisplay(TRUE);
}

// Palette "torch flicker" on the near stone tones
void gfx_flicker(u8 phase)
{
	static const u16 f[4] = { RGB16(5, 4, 3), RGB16(5, 4, 2), RGB16(6, 4, 3), RGB16(5, 3, 2) };
	VDP_SetPaletteEntry(C_ST4, f[phase & 3]);
	VDP_SetPaletteEntry(C_CREAM, (phase & 1) ? RGB16(7, 6, 3) : RGB16(7, 6, 2));
}

//-----------------------------------------------------------------------------
// Primitives (all clipped to 256x212)
static bool s_hot;

void gfx_rect(u16 x, u16 y, u16 w, u16 h, u8 c)
{
	if (w == 0 || h == 0) return;
	if (!g_oy) { s_hot = TRUE; if (y >= VIEW_H) { g_oy = 256; gfx_rect(x, y, w, h, c); g_oy = 0; } }
	if (((x | w) & 1) == 0) VDP_CommandHMMV(x, y + g_oy, w, h, c | (c << 4));	// byte aligned: fast path
	else VDP_CommandLMMV(x, y + g_oy, w, h, c, VDP_OP_IMP);
}

void gfx_hline(i16 x0, i16 x1, i16 y, u8 c)
{
	if (!g_oy) { s_hot = TRUE; if (y >= VIEW_H) { g_oy = 256; gfx_hline(x0, x1, y, c); g_oy = 0; } }
	if (x0 > x1) { i16 t = x0; x0 = x1; x1 = t; }
	if (x1 < 0 || x0 > 255 || y < 0 || y > 211) return;
	if (x0 < 0) x0 = 0;
	if (x1 > 255) x1 = 255;
	VDP_CommandLMMV(x0, y + g_oy, x1 - x0 + 1, 1, c, VDP_OP_IMP);
}

void gfx_vline(i16 x, i16 y0, i16 y1, u8 c)
{
	if (!g_oy) s_hot = TRUE;
	if (y0 > y1) { i16 t = y0; y0 = y1; y1 = t; }
	if (!g_oy && y0 >= VIEW_H) { g_oy = 256; gfx_vline(x, y0, y1, c); g_oy = 0; }
	if (x < 0 || x > 255 || y1 < 0 || y0 > 211) return;
	if (y0 < 0) y0 = 0;
	if (y1 > 211) y1 = 211;
	VDP_CommandLMMV(x, y0 + g_oy, 1, y1 - y0 + 1, c, VDP_OP_IMP);
}

static i16 clampx(i16 v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }
static i16 clampy(i16 v) { return v < 0 ? 0 : (v > 211 ? 211 : v); }

void gfx_line(i16 x0, i16 y0, i16 x1, i16 y1, u8 c)
{
	if (!g_oy) s_hot = TRUE;
	if (y0 == y1) { gfx_hline(x0, x1, y0, c); return; }
	if (x0 == x1) { gfx_vline(x0, y0, y1, c); return; }
	Draw_Line(clampx(x0), clampy(y0) + g_oy, clampx(x1), clampy(y1) + g_oy, c, VDP_OP_IMP);
}

void gfx_box(i16 x, i16 y, i16 w, i16 h, u8 c)
{
	gfx_hline(x, x + w - 1, y, c);
	gfx_hline(x, x + w - 1, y + h - 1, c);
	gfx_vline(x, y, y + h - 1, c);
	gfx_vline(x + w - 1, y, y + h - 1, c);
}

void gfx_clear(u8 c)
{
	show_page(0);
	s_hot = TRUE;
	VDP_CommandLMMV(0, 0, 256, 212, c, VDP_OP_IMP);
}

// Full-screen picture (256x212) from the RAM store into page 0; FALSE when the store is not loaded
bool gfx_art(u8 idx)
{
	if (!g_rsOk) return FALSE;
	g_oy = 0;
	show_page(0);
	VDP_CommandWait();
	s_hot = TRUE;
	rs_to_vram(idx, 0, 16384, 0);
	rs_to_vram(idx + 1, 0, 10752, 16384);
	return TRUE;
}

//-----------------------------------------------------------------------------
// Fast text: glyph rows are written by the CPU directly into VRAM (6x8 font, 2 pixels per byte).
// Text always starts on an even x, so every glyph covers exactly 3 whole bytes per row.
static u8 s_tlbuf[8];
static u8* s_tlp;								// 4-byte aligned lookup table (2 pixels -> 1 byte)
static const u8* s_tg;							// glyph (8 bytes)
static u16 s_ta;								// VRAM address of the glyph's first row (y*128 + x/2)

static void blit_glyph(void) __naked
{
	__asm
		push	ix
		ld		ix, (_s_tg)
		ld		de, (_s_ta)
		ld		b, #8
	blit_row:
		di
		ld		a, d							; R#14 = A16..A14
		rlca
		rlca
		and		a, #3
		out		(#0x99), a
		ld		a, #0x8E
		out		(#0x99), a
		ld		a, e							; A7..A0
		out		(#0x99), a
		ld		a, d							; 01 + A13..A8 (write)
		and		a, #0x3F
		or		a, #0x40
		out		(#0x99), a
		ld		c, 0(ix)						; glyph row, pixels in bits 7..2
		inc		ix
		ld		a, c							; pixels 0,1
		rlca
		rlca
		and		a, #3
		ld		hl, (_s_tlp)
		add		a, l
		ld		l, a
		ld		a, (hl)
		out		(#0x98), a
		ld		a, c							; pixels 2,3
		rrca
		rrca
		rrca
		rrca
		and		a, #3
		ld		hl, (_s_tlp)
		add		a, l
		ld		l, a
		ld		a, (hl)
		out		(#0x98), a
		ld		a, c							; pixels 4,5
		rrca
		rrca
		and		a, #3
		ld		hl, (_s_tlp)
		add		a, l
		ld		l, a
		ld		a, (hl)
		out		(#0x98), a
		ei
		ld		hl, #128
		add		hl, de
		ex		de, hl
		djnz	blit_row
		pop		ix
		ret
	__endasm;
}

void gfx_text(u16 x, u16 y, const c8* s, u8 fg, u8 bg)
{
	if (s_hot) { VDP_CommandWait(); s_hot = FALSE; }	// do not race with a fill that is still running
	s_tlp = (u8*)(((u16)s_tlbuf + 3) & 0xFFFC);
	s_tlp[0] = (bg << 4) | bg;
	s_tlp[1] = (bg << 4) | fg;
	s_tlp[2] = (fg << 4) | bg;
	s_tlp[3] = (fg << 4) | fg;
	for (u8 pass = 0; pass < 2; pass++)
	{
		u16 addr = (y + g_oy + (pass ? 256 : 0)) * 128 + ((x & 0xFFFE) >> 1);
		for (const c8* p = s; *p; p++, addr += 3)
		{
			s_tg = g_Font_MGL_Sample6 + 4 + (u8)*p * 8;
			s_ta = addr;
			blit_glyph();
		}
		if (y < VIEW_H || g_oy) break;					// HUD rows only: both pages hold the HUD
	}
}

// Drawing target: the page being displayed (on = TRUE) or the default (page 0, HUD on both pages)
void gfx_target_front(bool on)
{
	g_oy = (on && s_front) ? 256 : 0;
}

static u8 slen(const c8* s) { u8 n = 0; while (s[n]) n++; return n; }

void gfx_text_c(u16 y, const c8* s, u8 fg, u8 bg)
{
	gfx_text((256 - slen(s) * 6) / 2, y, s, fg, bg);
}

// Framed panel
void gfx_panel(u16 x, u16 y, u16 w, u16 h)
{
	gfx_rect(x, y, w, h, C_NAVY);
	gfx_box(x, y, w, h, C_WOOD);
	gfx_box(x + 1, y + 1, w - 2, h - 2, C_ST2);
}

// Decorated full screen page with centred title
void gfx_page(const c8* title)
{
	g_oy = 0;
	gfx_clear(C_BLACK);
	gfx_box(1, 1, 254, 210, C_WOOD);
	gfx_box(3, 3, 250, 206, C_GOLD);
	gfx_box(5, 5, 246, 202, C_WOOD);
	// corner studs
	gfx_rect(2, 2, 3, 3, C_CREAM); gfx_rect(251, 2, 3, 3, C_CREAM);
	gfx_rect(2, 207, 3, 3, C_CREAM); gfx_rect(251, 207, 3, 3, C_CREAM);
	if (title)
	{
		gfx_text_c(10, title, C_GOLD, C_BLACK);
		gfx_hline(20, 235, 20, C_WOOD);
		gfx_hline(20, 235, 22, C_ST2);
	}
}


// Scaled text (uses the 6x8 font bitmaps directly)
void gfx_text_big(u16 x, u16 y, const c8* s, u8 scale, u8 color)
{
	for (; *s; s++, x += 6 * scale + scale)
	{
		if (*s == ' ') continue;
		const u8* gl = g_Font_MGL_Sample6 + 4 + (u8)*s * 8;
		for (u8 r = 0; r < 8; r++)
		{
			u8 bits = gl[r];
			u8 c = 0;
			while (c < 6)
			{
				if (bits & (0x80 >> c))
				{
					u8 e = c;
					while (e < 6 && (bits & (0x80 >> e))) e++;
					gfx_rect(x + c * scale, y + r * scale, (e - c) * scale, scale, color);
					c = e;
				}
				else c++;
			}
		}
	}
}

// Default MSX2 palette (used when going back to MSX-DOS)
void gfx_restore_palette(void)
{
	static const u16 pal[16] =
	{
		RGB16(0, 0, 0), RGB16(0, 0, 0), RGB16(1, 6, 1), RGB16(3, 7, 3), RGB16(1, 1, 7), RGB16(2, 3, 7),
		RGB16(5, 1, 1), RGB16(2, 6, 7), RGB16(7, 1, 1), RGB16(7, 3, 3), RGB16(6, 6, 1), RGB16(6, 6, 4),
		RGB16(1, 4, 1), RGB16(6, 2, 5), RGB16(5, 5, 5), RGB16(7, 7, 7)
	};
	for (u8 i = 0; i < 16; i++) VDP_SetPaletteEntry(i, pal[i]);
}

//-----------------------------------------------------------------------------
// Fills
static void fill_tri(i16 cx, i16 ytop, i16 h, i16 halfw, u8 cl, u8 cr)
{
	for (i16 r = 0; r < h; r++)
	{
		i16 w = ((r + 1) * halfw) / h;
		gfx_hline(cx - w, cx, ytop + r, cl);
		gfx_hline(cx + 1, cx + w, ytop + r, cr);
	}
}

// Column-wise fill of a quad: x0..x1 with linear top (t0->t1) and bottom (b0->b1).
// Uses byte-aligned 2-pixel wide HMMV columns (the outline is drawn on top by the caller).
static void fill_cols(i16 x0, i16 x1, i16 t0, i16 t1, i16 b0, i16 b1, u8 c)
{
	if (x1 < x0) return;
	i16 w = x1 - x0;
	if (w == 0) { gfx_vline(x0, t0, b0, c); return; }
	i16 st = ((t1 - t0) * 256) / w, sb = ((b1 - b0) * 256) / w;
	i16 at = 0, ab = 0;
	u8 cc = c | (c << 4);
	i16 ast = st < 0 ? -st : st, asb = sb < 0 ? -sb : sb;
	u8 sw = ((ast > asb ? ast : asb) < 100) ? 8 : 4;		// column width: wider when the edges are flat
	for (i16 x = x0; x <= x1; )
	{
		i16 ta = t0 + (at >> 8), ba = b0 + (ab >> 8);
		if (((x & (sw - 1)) == 0) && x + sw - 1 <= x1 && x >= 0 && x + sw <= 256)
		{
			i16 sa = sw * st, sbb = sw * sb;
			i16 tb = t0 + ((at + sa - st) >> 8), bb = b0 + ((ab + sbb - sb) >> 8);
			i16 top = ta > tb ? ta : tb, bot = ba < bb ? ba : bb;
			if (bot >= top) VDP_CommandHMMV(x, top + g_oy, sw, bot - top + 1, cc);
			at += sa; ab += sbb; x += sw;
		}
		else
		{
			gfx_vline(x, ta, ba, c);
			at += st; ab += sb; x++;
		}
	}
}

// Row-wise fill of a quad: y0..y1 with x range interpolated
static void fill_rows(i16 y0, i16 y1, i16 xl0, i16 xr0, i16 xl1, i16 xr1, u8 c)
{
	i16 h = y1 - y0;
	if (h <= 0) { gfx_hline(xl0, xr0, y0, c); return; }
	for (i16 y = y0; y <= y1; y++)
	{
		i16 d = y - y0;
		gfx_hline(xl0 + ((xl1 - xl0) * d) / h, xr0 + ((xr1 - xr0) * d) / h, y, c);
	}
}

//-----------------------------------------------------------------------------
// 3D DUNGEON
//-----------------------------------------------------------------------------
static u8 cell(i8 x, i8 y)
{
	if (x < 0 || x > 10 || y < 0 || y > 10) return 1;
	return g_dn[x][y];
}

static const u8 s_Side[10]  = { C_ST3, C_ST3, C_ST2, C_ST2, C_ST1, C_ST1, C_ST1, C_ST1, C_ST1, C_ST1 };
static const u8 s_Front[10] = { C_ST4, C_ST4, C_ST3, C_ST3, C_ST2, C_ST2, C_ST1, C_ST1, C_ST1, C_ST1 };
static const u8 s_MonCol[11] = { 0, C_WHITE, C_BLUE, C_CREAM, C_GRASS, C_GOLD, C_CREAM, C_GRASS, C_GOLD, C_RED, C_RED };

static u8 darker(u8 c) { return (c > C_ST1) ? c - 1 : C_BLACK; }
static u8 edge_col(u8 k) { return k <= 2 ? C_CREAM : (k <= 5 ? C_ST4 : C_ST3); }

// Left side wall of cell k (mirrored when right = true)
static void side_wall(u8 k, bool right, bool door, bool open)
{
	i16 L1 = g_PE[k][0], L2 = g_PE[k + 1][0];
	i16 T1 = g_PE[k][2], T2 = g_PE[k + 1][2];
	i16 B1 = g_PE[k][3], B2 = g_PE[k + 1][3];
	if (right) { L1 = 255 - L1; L2 = 255 - L2; }
	i16 xa = right ? L2 : L1, xb = right ? L1 : L2;
	if (!open)
	{
		u8 tone = s_Side[k];
		if (!right) fill_cols(xa, xb, T1, T2, B1, B2, tone);
		else        fill_cols(xa, xb, T2, T1, B2, B1, tone);
		// mortar courses
		if (k < 2)
			for (u8 i = 1; i < 4; i++)
			{
				i16 ya = T1 + ((B1 - T1) * i) / 4, yb = T2 + ((B2 - T2) * i) / 4;
				if (!right) gfx_line(xa, ya, xb, yb, darker(tone));
				else        gfx_line(xa, yb, xb, ya, darker(tone));
			}
		if (door)
		{
			// door in the side wall (LD table)
			i16 x0 = g_LD[k][0], x1 = g_LD[k][1], y2 = g_LD[k][2], y3 = g_LD[k][3], y4 = g_LD[k][4], y5 = g_LD[k][5];
			if (k == 0) { y2 -= 3; x0 = 0; }
			if (right) { x0 = 255 - x0; x1 = 255 - x1; }
			if (!right) fill_cols(x0, x1, y2, y3, y4, y5, C_WOOD);
			else        fill_cols(x1, x0, y3, y2, y5, y4, C_WOOD);
			gfx_line(x0, y4, x0, y2, C_CREAM);
			gfx_line(x0, y2, x1, y3, C_CREAM);
			gfx_line(x1, y3, x1, y5, C_CREAM);
		}
		u8 e = edge_col(k);
		if (k < 3)
		{
			gfx_line(L1, T1, L2, T2, e);
			gfx_line(L1, B1, L2, B2, e);
		}
		if (k > 0) gfx_vline(L1, T1, B1, e);
	}
	else
	{
		// side passage: dark recess plus outline
		i16 x0 = L1 < L2 ? L1 : L2, x1 = L1 < L2 ? L2 : L1;
		gfx_rect(x0, T2, x1 - x0 + 1, B2 - T2 + 1, C_BLACK);
		u8 e = edge_col(k);
		if (k > 0) gfx_vline(L1, T1, B1, e);
		gfx_hline(L1, L2, T2, e);
		gfx_vline(L2, T2, B2, e);
		gfx_hline(L1, L2, B2, e);
	}
}

static void front_wall(u8 k, u8 ce)
{
	i16 L = g_PE[k][0], R = g_PE[k][1], T = g_PE[k][2], B = g_PE[k][3];
	u8 tone = s_Front[k];
	gfx_rect(L, T, R - L + 1, B - T + 1, tone);
	for (u8 i = 1; i < 4; i++)
		gfx_hline(L, R, T + ((B - T) * i) / 4, darker(tone));
	u8 e = edge_col(k);
	gfx_box(L, T, R - L + 1, B - T + 1, e);
	if (ce == 4)
	{
		i16 dl = g_CD[k][0], dr = g_CD[k][1], dt = g_CD[k][2], db = g_CD[k][3];
		gfx_rect(dl, dt, dr - dl + 1, db - dt + 1, C_WOOD);
		gfx_hline(dl, dr, dt, C_CREAM);
		gfx_vline(dl, dt, db, C_CREAM);
		gfx_vline(dr, dt, db, C_CREAM);
		i16 my = (dt + db) / 2;
		gfx_rect(dr - 2 - (k < 3 ? 2 : 0), my, k < 3 ? 3 : 2, k < 3 ? 3 : 1, C_GOLD);	// knob
		for (u8 i = 1; i < 3; i++)
			gfx_vline(dl + ((dr - dl) * i) / 3, dt, db, darker(C_WOOD));
	}
}

// Floor (or ceiling) hole + ladder
static void floor_feature(u8 k, u8 ce)
{
	bool ceil = (ce == 8);
	i16 x0 = g_FT[k][0], x1 = g_FT[k][1], x2 = g_FT[k][2], x3 = g_FT[k][3], y4 = g_FT[k][4], y5 = g_FT[k][5];
	if (ceil) { y4 = 158 - y4; y5 = 158 - y5; }
	if (y4 > y5) fill_rows(y5, y4, x2, x3, x0, x1, C_BLACK);
	else         fill_rows(y4, y5, x0, x1, x2, x3, C_BLACK);
	gfx_line(x0, y4, x2, y5, C_CREAM);
	gfx_line(x2, y5, x3, y5, C_CREAM);
	gfx_line(x3, y5, x1, y4, C_CREAM);
	gfx_line(x1, y4, x0, y4, C_CREAM);
	if (ce != 9)
	{
		i16 lx = g_LA[k][0], rx = g_LA[k][1], tp = g_LA[k][2], ba = g_LA[k][3];
		gfx_vline(lx, tp, ba, C_WOOD);
		gfx_vline(rx, tp, ba, C_WOOD);
		if (k < 4) { gfx_vline(lx + 1, tp, ba, C_WOOD); gfx_vline(rx - 1, tp, ba, C_WOOD); }
		for (u8 i = 1; i < 5; i++)
		{
			i16 y = (ba * (5 - i) + tp * i) / 5;
			gfx_hline(lx, rx, y, C_GOLD);
		}
	}
}

static void draw_chest(u8 k, u8 col)
{
	i16 yb = g_PE[k][3], d = 10 / k, c = VIEW_CX;
	// front
	gfx_rect(c - d, yb - d, 2 * d + 1, d + 1, C_WOOD);
	gfx_box(c - d, yb - d, 2 * d + 1, d + 1, col);
	// lid / top
	i16 d5 = 5 / k, d15 = 15 / k;
	gfx_line(c - d, yb - d, c - d5, yb - d15, col);
	gfx_hline(c - d5, c + d15, yb - d15, col);
	gfx_vline(c + d15, yb - d15, yb - d5, col);
	gfx_line(c + d15, yb - d5, c + d, yb, col);
	gfx_line(c + d, yb - d, c + d15, yb - d15, col);
	if (k < 4) gfx_rect(c - 1, yb - d - 1, 3, 3, C_GOLD);		// lock
}

static void draw_shape(u8 mc, u8 k, u8 col, i8 offs)
{
	const i16* div = g_Div88[k - 1];
	i16 C = VIEW_CX + offs, B = VIEW_CY + g_YY[k] + offs, ytop = VIEW_CY - g_YY[k] + offs;
	i16 cx = 0, cy = 0;
	u16 a = g_ShapeStart[mc - 1], e = g_ShapeStart[mc];
	for (; a < e; a++)
	{
		const ShapePt* p = &g_Shapes[a];
		i16 vx = div[p->x + 96];
		if (p->op & SH_HALF) vx += 128;
		i16 x = C + ((vx + 128) >> 8);
		i16 y = (p->op & SH_TOP) ? ytop : B - ((div[p->y + 96] + 128) >> 8);
		u8 op = p->op & 3;
		if (op == SH_LINE) gfx_line(cx, cy, x, y, col);
		else if (op == SH_DOT) gfx_rect(x, y, 1, 1, col);
		cx = x; cy = y;
	}
}

static void draw_monster(u8 mc, u8 k)
{
	if (mc == M_MIMIC) { draw_chest(k, C_GOLD); return; }
	if (k < 4) draw_shape(mc, k, C_BLACK, 1);		// drop shadow
	draw_shape(mc, k, s_MonCol[mc], 0);
	if (k < 2) draw_shape(mc, k, s_MonCol[mc], -1);	// bolder strokes when close
}

//-----------------------------------------------------------------------------
// Dungeon automap: the image lives in page 1 (rows 420..452) and is updated incrementally
u8 g_dseen[DUNW][DUNW];
bool g_dmapon = true;
static u8 s_dtype[DUNW][DUNW];
#define DMAP_IY		470

static void dmap_cell(u8 x, u8 y, u8 t)
{
	u8 c;
	switch (t)
	{
	case 1: case 3: c = C_ST1; break;			// walls (secret doors look like walls)
	case 4:         c = C_WOOD; break;			// door
	case 5:         c = C_GOLD; break;			// chest
	case 7: case 8: c = C_GRASS; break;			// ladders
	case 9:         c = C_BLUE; break;			// hole
	default:        c = C_ST4; break;			// open floor (traps stay hidden)
	}
	VDP_CommandLMMV(x * 3, DMAP_IY + y * 3, 3, 3, c, VDP_OP_IMP);
}

static void dmap_mark(i8 x, i8 y)
{
	if (x < 0 || x > 10 || y < 0 || y > 10) return;
	u8 t = g_dn[x][y] % 10;
	if (!g_dseen[x][y] || s_dtype[x][y] != t)
	{
		g_dseen[x][y] = 1; s_dtype[x][y] = t;
		dmap_cell(x, y, t);
	}
}

void gfx_dmap_reset(void)
{
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) { g_dseen[x][y] = 0; s_dtype[x][y] = 0; }
	VDP_CommandLMMV(0, DMAP_IY, 34, 33, 0, VDP_OP_IMP);
}

void gfx_dmap_rebuild(void)
{
	VDP_CommandLMMV(0, DMAP_IY, 34, 33, 0, VDP_OP_IMP);
	for (u8 x = 0; x < DUNW; x++)
		for (u8 y = 0; y < DUNW; y++)
			if (g_dseen[x][y]) { s_dtype[x][y] = g_dn[x][y] % 10; dmap_cell(x, y, s_dtype[x][y]); }
}

static i8 s_monx[10], s_mony[10];

volatile u16 g_prof[6];			// profiling: frames per phase of the last 3D draw

void gfx_draw_dungeon(void)
{
	gfx_ow_invalidate();
	g_visn = 0; g_visChest = false;
	u8 back = s_front ^ 1;
	g_oy = back ? 256 : 0;
	u16 tp = g_JIFFY;
	for (u8 q = 0; q < 6; q++) g_prof[q] = 0;

	// depth of the first front wall
	u8 n;
	for (n = 1; n < 10; n++)
	{
		u8 t = cell(g_px + g_dx * n, g_py + g_dy * n) % 10;
		if (t == 1 || t == 3 || t == 4) break;
	}

	// background
	gfx_rect(0, 0, 256, VIEW_H, C_BLACK);
	g_prof[0] = g_JIFFY - tp; tp = g_JIFFY;

	for (i8 k = n; k >= 0; k--)
	{
		i8 cx = g_px + g_dx * k, cy = g_py + g_dy * k;
		u8 v = cell(cx, cy), ce = v % 10, mc = v / 10;
		dmap_mark(cx, cy);
		if (k != (i8)n) { dmap_mark(cx + g_dy, cy - g_dx); dmap_mark(cx - g_dy, cy + g_dx); }
		if (k == (i8)n)
		{
			front_wall(k, ce);
		}
		else
		{
			u8 le = cell(cx + g_dy, cy - g_dx) % 10, ri = cell(cx - g_dy, cy + g_dx) % 10;
			bool par = ((cx + cy) & 1);
			i16 L = g_PE[k][0], R = g_PE[k][1];
			i16 T1 = g_PE[k][2], T2 = g_PE[k + 1][2], B1 = g_PE[k][3], B2 = g_PE[k + 1][3];
			// floor & ceiling strips (checkerboard along the corridor)
			u8 fa = k < 5 ? C_FLOOR : C_NAVY, fb = k < 5 ? C_NAVY : C_BLACK;
			u8 ca = k < 5 ? C_CEIL : C_BLACK;
			gfx_rect(L, B2, R - L + 1, B1 - B2 + 1, par ? fa : fb);
			gfx_rect(L, T1, R - L + 1, T2 - T1 + 1, par ? ca : C_BLACK);
			g_prof[1] += g_JIFFY - tp; tp = g_JIFFY;
			bool lw = (le == 1 || le == 3 || le == 4), rw = (ri == 1 || ri == 3 || ri == 4);
			side_wall(k, false, le == 4, !lw);
			side_wall(k, true,  ri == 4, !rw);
			g_prof[2] += g_JIFFY - tp; tp = g_JIFFY;
			if (ce == 7 || ce == 8 || ce == 9) floor_feature(k, ce);
			if (ce == 5 && k > 0) { draw_chest(k, C_GOLD); g_visChest = true; }
		}
		if (mc > 0)
		{
			if (k > 0) draw_monster(mc, k);
			if (g_visn < 10) { s_monx[g_visn] = cx; s_mony[g_visn] = cy; g_vis[g_visn++] = mc; }
		}
		g_prof[3] += g_JIFFY - tp; tp = g_JIFFY;
	}
	// automap overlay (top right corner)
	if (g_dmapon)
	{
		gfx_box(217, 2, 36, 36, C_WOOD);
		VDP_CommandHMMM(0, DMAP_IY, 218, g_oy + 3, 34, 33);
		for (u8 i = 0; i < g_visn; i++)
			gfx_rect(218 + s_monx[i] * 3, 3 + s_mony[i] * 3, 3, 3, C_RED);
		gfx_rect(218 + g_px * 3, 3 + g_py * 3, 3, 3, C_WHITE);
		gfx_rect(219 + g_px * 3 + g_dx, 4 + g_py * 3 + g_dy, 1, 1, C_RED);
	}
	// flip: the page just drawn becomes the visible one (no copy)
	g_oy = 0;
	show_page(back);
	g_prof[4] = g_JIFFY - tp;
}

//-----------------------------------------------------------------------------
// OVERWORLD
//-----------------------------------------------------------------------------
static u8 hash8(u8 a, u8 b, u8 i)
{
	u16 h = a * 131 + b * 31 + i * 17 + 7;
	h ^= h >> 5; h *= 109; h ^= h >> 7;
	return (u8)h;
}

static void tile_grass(i16 x, i16 y, u8 tx, u8 ty)
{
	gfx_rect(x, y, OW_TS, OW_TS, C_GRASS);
	for (u8 i = 0; i < 5; i++)
	{
		i16 px = x + 3 + hash8(tx, ty, i) % 44, py = y + 4 + hash8(tx, ty, i + 9) % 42;
		gfx_rect(px, py, 2, 1, C_FOREST);
	}
}

static void tile_mountain(i16 x, i16 y)
{
	fill_tri(x + 15, y + 22, 22, 13, C_ST2, C_ST1);
	fill_tri(x + 15, y + 22, 8, 4, C_WHITE, C_ST4);
	fill_tri(x + 29, y + 6, 38, 21, C_ST4, C_ST2);
	fill_tri(x + 29, y + 6, 13, 7, C_WHITE, C_ST4);
	gfx_line(x + 29, y + 6, x + 29, y + 44, C_ST3);
}

static void pine(i16 cx, i16 by)
{
	gfx_rect(cx - 1, by - 4, 3, 5, C_WOOD);
	fill_tri(cx, by - 20, 11, 6, C_FOREST, C_NAVY);
	fill_tri(cx, by - 13, 11, 9, C_FOREST, C_NAVY);
	gfx_rect(cx, by - 20, 1, 1, C_GRASS);
}

static void tile_trees(i16 x, i16 y)
{
	pine(x + 13, y + 28);
	pine(x + 34, y + 24);
	pine(x + 24, y + 43);
}

static void house(i16 x, i16 y, i16 w, i16 h)
{
	gfx_rect(x, y, w, h, C_ST4);
	gfx_hline(x, x + w - 1, y + h - 1, C_ST2);
	fill_tri(x + w / 2, y - 9, 10, w / 2 + 2, C_RED, C_WOOD);
	gfx_rect(x + w / 2 - 2, y + h - 7, 4, 7, C_WOOD);
	gfx_rect(x + 2, y + 3, 3, 3, C_GOLD);
}

static void tile_town(i16 x, i16 y)
{
	gfx_rect(x + 3, y + 36, 44, 8, C_ST3);			// cobbled square
	gfx_hline(x + 3, x + 46, y + 40, C_ST2);
	house(x + 5, y + 21, 16, 15);
	house(x + 27, y + 17, 18, 19);
	gfx_vline(x + 24, y + 8, y + 20, C_CREAM);		// banner pole
	gfx_rect(x + 25, y + 8, 6, 4, C_RED);
}

static void tile_dungeon(i16 x, i16 y)
{
	static const u8 ell[15] = { 7, 12, 15, 17, 18, 19, 19, 19, 19, 19, 18, 17, 15, 12, 7 };
	i16 cx = x + 25;
	for (u8 r = 0; r < 15; r++)
		gfx_rect(cx - ell[r] - 2, y + 13 + r * 2, 2 * ell[r] + 5, 2, C_ST3);
	for (u8 r = 0; r < 15; r++)
		gfx_rect(cx - ell[r], y + 13 + r * 2, 2 * ell[r] + 1, 2, C_BLACK);
	// stairs going down
	for (u8 s = 0; s < 5; s++)
		gfx_rect(cx - 12 + s * 2, y + 22 + s * 3, 25 - s * 4, 2, (s & 1) ? C_ST1 : C_ST2);
	gfx_hline(cx - 14, cx + 14, y + 18, C_ST4);
}

static void tile_castle(i16 x, i16 y)
{
	gfx_rect(x + 11, y + 24, 29, 20, C_ST4);						// keep
	gfx_rect(x + 6, y + 15, 10, 29, C_ST3);							// towers
	gfx_rect(x + 35, y + 15, 10, 29, C_ST3);
	for (u8 i = 0; i < 3; i++)										// battlements
	{
		gfx_rect(x + 6 + i * 4, y + 12, 2, 4, C_ST3);
		gfx_rect(x + 35 + i * 4, y + 12, 2, 4, C_ST3);
		gfx_rect(x + 12 + i * 6, y + 21, 3, 4, C_ST4);
	}
	gfx_vline(x + 11, y + 24, y + 44, C_ST2);
	gfx_vline(x + 16, y + 15, y + 44, C_ST2);
	gfx_vline(x + 35, y + 15, y + 44, C_ST2);
	gfx_rect(x + 22, y + 31, 8, 13, C_WOOD);						// gate
	gfx_rect(x + 23, y + 29, 6, 3, C_WOOD);
	gfx_vline(x + 26, y + 29, y + 44, darker(C_WOOD));
	gfx_rect(x + 9, y + 20, 2, 4, C_BLACK);
	gfx_rect(x + 39, y + 20, 2, 4, C_BLACK);
	gfx_vline(x + 11, y + 3, y + 12, C_CREAM);						// flags
	gfx_rect(x + 12, y + 3, 6, 4, C_RED);
	gfx_vline(x + 40, y + 3, y + 12, C_CREAM);
	gfx_rect(x + 41, y + 3, 6, 4, C_GOLD);
}

static void tile_draw(u8 t, i16 x, i16 y, u8 tx, u8 ty)
{
	tile_grass(x, y, tx, ty);
	switch (t)
	{
	case T_MOUNTAIN: tile_mountain(x, y); break;
	case T_TREES:    tile_trees(x, y);    break;
	case T_TOWN:     tile_town(x, y);     break;
	case T_DUNGEON:  tile_dungeon(x, y);  break;
	case T_CASTLE:   tile_castle(x, y);   break;
	}
}

static void put_num2(c8* s, u8 v)
{
	s[0] = v >= 10 ? '0' + v / 10 : ' ';
	s[1] = '0' + v % 10;
	s[2] = 0;
}

// Overworld: the side panels and the minimap are persistent on the visible page; only the tile
// area is rebuilt off-screen and copied, and only the values that changed are re-printed.
#define ATLAS_Y		212				// atlas rows in page 0 (below the visible screen); tiles are cropped to 44 rows
static bool s_atlas = FALSE;

// Each special tile is rendered once into the atlas and then only copied (HMMM)
static void atlas_build(void)
{
	g_oy = 256;
	for (u8 t = T_MOUNTAIN; t <= T_CASTLE; t++)
		tile_draw(t, (t - 1) * OW_TS, 0, 7, 7);
	VDP_CommandHMMM(0, 256, 0, ATLAS_Y, 5 * OW_TS, 44);
	VDP_CommandWait();
	s_atlas = TRUE;
}

static bool s_owStatic = FALSE;
static u8 s_lx = 255, s_ly = 255, s_lt = 255;
static u8 s_mapDone[MAPW][MAPW];
static u8 s_pmx = 255, s_pmy = 255;

void gfx_ow_invalidate(void) { s_owStatic = FALSE; }

static u8 map_color(u8 x, u8 y)
{
	switch (g_te[x][y])
	{
	case T_MOUNTAIN: return C_ST3;
	case T_TREES:    return C_FOREST;
	case T_TOWN:     return C_GOLD;
	case T_DUNGEON:  return C_RED;
	case T_CASTLE:   return C_WHITE;
	}
	return C_GRASS;
}

static void ow_static(void)
{
	g_oy = 0;
	gfx_rect(0, 0, 256, VIEW_H, C_BLACK);
	gfx_box(OW_X0 - 2, OW_Y0 - 2, 3 * OW_TS + 4, 3 * OW_TS + 4, C_WOOD);
	gfx_box(OW_X0 - 1, OW_Y0 - 1, 3 * OW_TS + 2, 3 * OW_TS + 2, C_GOLD);
	gfx_text(5, 8, "WORLD", C_GOLD, C_BLACK);
	gfx_text(5, 22, "X", C_ST4, C_BLACK);
	gfx_text(5, 32, "Y", C_ST4, C_BLACK);
	gfx_text(22, 90, "N", C_CREAM, C_BLACK);					// compass
	gfx_vline(25, 106, 124, C_ST4);
	gfx_line(25, 100, 22, 106, C_CREAM);
	gfx_line(25, 100, 28, 106, C_CREAM);
	gfx_hline(16, 34, 118, C_ST2);
	gfx_text(214, 8, "MAP", C_GOLD, C_BLACK);					// minimap frame
	gfx_box(208, 20, 46, 46, C_WOOD);
	gfx_rect(209, 21, 44, 44, C_NAVY);
	for (u8 y = 0; y < MAPW; y++) for (u8 x = 0; x < MAPW; x++) s_mapDone[x][y] = 0;
	s_lx = s_ly = s_lt = 255;
	s_pmx = 255;
	s_owStatic = TRUE;
}

void gfx_draw_overworld(void)
{
	g_visn = 0; g_visChest = false;
	show_page(0);
	if (!s_owStatic) ow_static();
	for (i8 iy = -1; iy <= 1; iy++)
		for (i8 ix = -1; ix <= 1; ix++)
			g_seen[g_tx + ix][g_ty + iy] = 1;

	// tiles + player, built off-screen then copied
	if (!s_atlas) atlas_build();
	g_oy = 256;
	for (i8 iy = -1; iy <= 1; iy++)
		for (i8 ix = -1; ix <= 1; ix++)
		{
			u8 tx = g_tx + ix, ty = g_ty + iy, t = g_te[tx][ty];
			i16 x = OW_X0 + (ix + 1) * OW_TS, y = OW_Y0 + (iy + 1) * OW_TS;
			if (t == T_GRASS) tile_grass(x, y, tx, ty);
			else
			{
				VDP_CommandHMMM((t - 1) * OW_TS, ATLAS_Y, x, 256 + y, OW_TS, 44);
				gfx_rect(x, y + 44, OW_TS, OW_TS - 44, C_GRASS);		// the cropped rows are plain grass
			}
		}
	i16 px = OW_X0 + OW_TS + 25, py = OW_Y0 + OW_TS + 24;
	gfx_rect(px - 5, py - 10, 11, 21, C_BLACK);
	gfx_rect(px - 2, py - 9, 5, 5, C_CREAM);			// head
	gfx_rect(px - 3, py - 10, 7, 2, C_ST4);				// helmet
	gfx_rect(px - 4, py - 4, 9, 8, C_RED);				// tunic
	gfx_hline(px - 4, px + 4, py, C_GOLD);				// belt
	gfx_rect(px - 3, py + 4, 3, 5, C_BLUE);				// legs
	gfx_rect(px + 1, py + 4, 3, 5, C_BLUE);
	gfx_vline(px + 6, py - 8, py + 2, C_WHITE);			// sword
	gfx_hline(px + 5, px + 7, py + 1, C_GOLD);
	VDP_CommandHMMM(OW_X0, 256 + OW_Y0, OW_X0, OW_Y0, 3 * OW_TS, 3 * OW_TS);
	g_oy = 0;

	// values that changed
	c8 num[3];
	if (s_lx != g_tx) { put_num2(num, g_tx); gfx_text(17, 22, num, C_CREAM, C_BLACK); s_lx = g_tx; }
	if (s_ly != g_ty) { put_num2(num, g_ty); gfx_text(17, 32, num, C_CREAM, C_BLACK); s_ly = g_ty; }
	u8 t = g_te[g_tx][g_ty];
	if (s_lt != t)
	{
		const c8* nm = "WILDS  ";
		if (t == T_TREES) nm = "FOREST ";
		else if (t == T_TOWN) nm = "TOWN   ";
		else if (t == T_DUNGEON) nm = "DUNGEON";
		else if (t == T_CASTLE) nm = "CASTLE ";
		gfx_text(5, 56, nm, C_GRASS, C_BLACK);
		gfx_text(5, 66, t >= T_TOWN ? "ENTER" : "     ", C_GOLD, C_BLACK);
		s_lt = t;
	}

	// minimap: only the newly explored cells, then move the marker
	for (u8 y = 0; y < MAPW; y++)
		for (u8 x = 0; x < MAPW; x++)
			if (g_seen[x][y] && !s_mapDone[x][y])
			{
				s_mapDone[x][y] = 1;
				gfx_rect(210 + x * 2, 22 + y * 2, 2, 2, map_color(x, y));
			}
	if (s_pmx != 255)
		gfx_rect(210 + s_pmx * 2, 22 + s_pmy * 2, 2, 2, s_mapDone[s_pmx][s_pmy] ? map_color(s_pmx, s_pmy) : C_NAVY);
	gfx_rect(210 + g_tx * 2, 22 + g_ty * 2, 2, 2, C_WHITE);
	s_pmx = g_tx; s_pmy = g_ty;
}
