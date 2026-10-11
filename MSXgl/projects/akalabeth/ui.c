//=============================================================================
// AKALABETH MSX2 - user interface: RNG, console, HUD, input, sound effects
//=============================================================================
#include "akalabeth.h"

//-----------------------------------------------------------------------------
// RNG (xorshift16, full period)
u16 g_rngState = 0xACE1;

u16 rnd16(void)
{
	u16 x = g_rngState;
	x ^= x << 7;
	x ^= x >> 9;
	x ^= x << 8;
	g_rngState = x;
	return x;
}

u16 rnd_n(u16 n)
{
	return (u16)(((u32)rnd16() * n) >> 16);
}

void rnd_seed(u16 s)
{
	g_rngState = s ? s : 0xACE1;
	for (u8 i = 0; i < 12; i++) rnd16();		// warm-up
}

u16 ui_frames(void) { return g_JIFFY; }

//-----------------------------------------------------------------------------
// Small helpers
static u8 i2s(i16 v, c8* o)
{
	c8 t[7]; u8 n = 0, m = 0;
	if (v < 0) { o[m++] = '-'; v = -v; }
	do { t[n++] = '0' + (u16)v % 10; v = (u16)v / 10; } while (v);
	while (n) o[m++] = t[--n];
	o[m] = 0;
	return m;
}

//-----------------------------------------------------------------------------
// Message console
static u8 s_line = 0, s_col = 0;

static void con_scroll(void)
{
	VDP_CommandHMMM(CON_X, CON_Y + 8, CON_X, CON_Y, CON_COLS * 6, (CON_LINES - 1) * 8);
	VDP_CommandHMMM(CON_X, CON_Y + 8 + 256, CON_X, CON_Y + 256, CON_COLS * 6, (CON_LINES - 1) * 8);	// the HUD also exists on page 1
	gfx_rect(CON_X, CON_Y + (CON_LINES - 1) * 8, CON_COLS * 6, 8, C_NAVY);
	s_line = CON_LINES - 1;
}

void con_clear(void)
{
	gfx_rect(CON_X, CON_Y, CON_COLS * 6, CON_LINES * 8, C_NAVY);
	s_line = 0; s_col = 0;
}

static u16 s_nlCount = 0;							// total number of console lines started (to detect extra messages in a turn)
u16 con_nlcount(void) { return s_nlCount; }

void con_nl(void)
{
	s_col = 0;
	s_line++;
	s_nlCount++;
}

// Print without newline; word wraps at the console width
void con_putsn(const c8* s, u8 color)
{
	c8 buf[CON_COLS + 1];
	while (*s)
	{
		u8 len = 0;
		while (s[len]) len++;
		u8 room = CON_COLS - s_col;
		u8 take = len;
		if (take > room)
		{
			take = room;
			if (s_col > 0 && room < 6) take = 0;				// start a fresh line for tight spaces
			else
			{
				u8 b = take;
				while (b > 0 && s[b] != ' ' && s[b - 1] != ' ') b--;	// back up to a space
				if (b > 0) take = b;
			}
		}
		if (take)
		{
			for (u8 i = 0; i < take; i++) buf[i] = s[i];
			buf[take] = 0;
			if (s_line >= CON_LINES) con_scroll();
			gfx_text(CON_X + s_col * 6, CON_Y + s_line * 8, buf, color, C_NAVY);
			s_col += take;
			s += take;
			while (*s == ' ' && s_col >= CON_COLS) s++;
		}
		if (*s) { con_nl(); if (s_line >= CON_LINES) con_scroll(); while (*s == ' ') s++; }
	}
}

void con_puts(const c8* s, u8 color)
{
	con_putsn(s, color);
	con_nl();
}

void con_putint(i16 v, u8 color)
{
	c8 b[8];
	i2s(v, b);
	con_putsn(b, color);
}

//-----------------------------------------------------------------------------
// HUD
static c8 s_hudLast[5][14];

void hud_invalidate(void)
{
	for (u8 r = 0; r < 5; r++) { s_hudLast[r][0] = 1; s_hudLast[r][13] = 0xFF; }
}

void hud_draw_static(void)
{
	gfx_rect(0, VIEW_H, 256, 212 - VIEW_H, C_BLACK);
	gfx_hline(0, 255, VIEW_H, C_GOLD);
	gfx_hline(0, 255, VIEW_H + 1, C_WOOD);
	gfx_panel(1, 164, 172, 47);
	gfx_panel(176, 164, 79, 47);
	con_clear();
	hud_invalidate();
	gfx_ow_invalidate();
}

// Draws only the part of the row that changed since the last update (text drawing is slow)
static void hud_row(u8 row, const c8* label, const c8* val, u8 col)
{
	c8 line[14];
	u8 n = 0;
	for (const c8* p = label; *p; p++) line[n++] = *p;
	for (const c8* p = val; *p && n < 12; p++) line[n++] = *p;
	while (n < 12) line[n++] = ' ';
	line[n] = 0;
	u8 i = 0;
	if (s_hudLast[row][13] == col) { while (i < 12 && line[i] == s_hudLast[row][i]) i++; }
	if (i >= 12) return;
	gfx_text(181 + i * 6, 167 + row * 8, line + i, col, C_NAVY);
	for (u8 j = 0; j < 12; j++) s_hudLast[row][j] = line[j];
	s_hudLast[row][13] = col;
}

void hud_update(void)
{
	c8 v[10];
	i16 f = g_inv[0];
	u8 n = i2s(f / 10, v);
	if (f % 10) { v[n++] = '.'; v[n++] = '0' + f % 10; v[n] = 0; }
	hud_row(0, "FOOD ", v, g_inv[0] < 50 ? C_RED : C_GRASS);
	i2s(g_stat[ST_HP], v);
	hud_row(1, "H.P. ", v, g_stat[ST_HP] < 6 ? C_RED : C_CREAM);
	i2s(g_stat[ST_GOLD], v);
	hud_row(2, "GOLD ", v, C_GOLD);
	if (g_in)
	{
		i2s(g_in, v);
		hud_row(3, "LEVEL ", v, C_BLUE);
		v[1] = 0;
		if (g_dx > 0) v[0] = 'E'; else if (g_dx < 0) v[0] = 'W'; else if (g_dy > 0) v[0] = 'S'; else v[0] = 'N';
		hud_row(4, "FACING ", v, C_CREAM);
	}
	else
	{
		hud_row(3, "LEVEL ", "-", C_ST3);
		hud_row(4, "WORLD ", "", C_ST3);
	}
}

//-----------------------------------------------------------------------------
// SOUND: music (3 voices) + sound effects, driven from the VDP interrupt (H.TIMI hook).
// Only the interrupt routine writes to the PSG. Channel A is shared: effects replace the melody.
#include "content/music_gen.h"


typedef struct { u16 tone; u8 noise; u8 vol; u8 len; } SfxStep;

static const SfxStep sfx_step[]  = { {0, 22, 5, 1}, {0, 0, 0, 0} };
static const SfxStep sfx_bump[]  = { {700, 0, 9, 2}, {900, 0, 9, 3}, {0, 0, 0, 0} };
static const SfxStep sfx_hit[]   = { {0, 6, 14, 3}, {300, 0, 13, 2}, {0, 18, 9, 4}, {0, 0, 0, 0} };
static const SfxStep sfx_miss[]  = { {0, 4, 7, 5}, {0, 12, 4, 3}, {0, 0, 0, 0} };
static const SfxStep sfx_hurt[]  = { {300, 0, 14, 3}, {380, 0, 14, 3}, {480, 0, 13, 3}, {640, 0, 12, 4}, {0, 0, 0, 0} };
static const SfxStep sfx_coin[]  = { {200, 0, 12, 3}, {142, 0, 12, 10}, {0, 0, 0, 0} };
static const SfxStep sfx_trap[]  = { {150, 0, 13, 2}, {200, 0, 13, 2}, {280, 0, 13, 2}, {400, 0, 13, 2}, {560, 0, 12, 2}, {800, 0, 12, 3}, {0, 14, 10, 10}, {0, 0, 0, 0} };
static const SfxStep sfx_down[]  = { {213, 0, 11, 4}, {284, 0, 11, 4}, {426, 0, 11, 6}, {0, 0, 0, 0} };
static const SfxStep sfx_up[]    = { {426, 0, 11, 4}, {284, 0, 11, 4}, {213, 0, 11, 6}, {0, 0, 0, 0} };
static const SfxStep sfx_death[] = { {300, 0, 14, 8}, {320, 0, 14, 8}, {360, 0, 14, 8}, {440, 0, 14, 14}, {0, 30, 10, 30}, {0, 0, 0, 0} };
static const SfxStep sfx_magic[] = { {400, 0, 10, 2}, {336, 0, 10, 2}, {284, 0, 10, 2}, {238, 0, 10, 2}, {200, 0, 10, 2}, {169, 0, 10, 2}, {142, 0, 10, 6}, {0, 0, 0, 0} };
static const SfxStep sfx_select[] = { {254, 0, 9, 2}, {0, 0, 0, 0} };
static const SfxStep sfx_buy[]   = { {190, 0, 12, 3}, {127, 0, 12, 8}, {0, 0, 0, 0} };
static const SfxStep sfx_fan[]   = { {213, 0, 13, 7}, {169, 0, 13, 7}, {142, 0, 13, 7}, {106, 0, 14, 20}, {0, 0, 0, 0} };

static const SfxStep* const sfx_table[14] =
{
	sfx_step, sfx_bump, sfx_hit, sfx_miss, sfx_hurt, sfx_coin, sfx_trap,
	sfx_down, sfx_up, sfx_death, sfx_magic, sfx_select, sfx_buy, sfx_fan
};

typedef struct { const u8* p; u8 left; u16 period; u8 age; u8 vol; u8 on; } Voice;

static const SfxStep* volatile s_sfx = 0;		// effect being played (0 = none)
static volatile u8 s_sfxLeft = 0;
static const MusicSong* s_song = 0;
static volatile bool s_musicOn = TRUE;
static volatile u8 s_newSong = 0xFF;			// song requested by the main program (0xFF = none, 0xFE = stop)
static u8 s_curSong = 0xFF;
static Voice s_voice[3];
static bool s_hz50 = TRUE;
static u8 s_skip = 0;
static volatile bool g_psgBusy = FALSE;			// set while the BIOS reads the joysticks (shares the PSG ports)
volatile u16 g_sndTicks;						// number of interrupts handled (diagnostics)
static u8 s_shadow[14];							// last value written to each PSG register

// PSG register write (A = register, L = value; sdcccall(1))
static void psg_w(u8 reg, u8 val) __naked
{
	reg; val;
	__asm
		out	(#0xA0), a
		ld	a, l
		out	(#0xA1), a
		ret
	__endasm;
}

// only touch the PSG when the value really changed (most ticks change nothing)
static void psg_set(u8 reg, u8 val)
{
	if (s_shadow[reg] != val) { s_shadow[reg] = val; psg_w(reg, val); }
}

// volume of the note: melody decays quickly, bass slowly, harmony is constant; short release at the end
static u8 env_vol(u8 i, u8 age, u8 left)
{
	u8 vv, d;
	if (i == 0)      { d = age >> 3; vv = (d < 6) ? 12 - d : 6; }
	else if (i == 1) { d = age >> 4; vv = (d < 4) ? 10 - d : 6; }
	else             vv = 5;
	if (left <= 1) vv = vv > 3 ? vv - 3 : 0;
	return vv;
}

// Interrupt service: called 50/60 times per second
void snd_tick(void)
{
	g_sndTicks++;
	if (g_psgBusy) return;
	if (!s_hz50 && ++s_skip >= 6) { s_skip = 0; return; }		// keep the tempo identical on 60 Hz machines

	// song change request
	u8 req = s_newSong;
	if (req != 0xFF)
	{
		s_newSong = 0xFF;
		if (req == 0xFE) { s_song = 0; s_curSong = 0xFF; }
		else if (req != s_curSong)
		{
			const MusicSong* sg = &g_Songs[req];
			s_song = sg; s_curSong = req;
			for (u8 i = 0; i < 3; i++) { s_voice[i].p = sg->tr[i]; s_voice[i].left = 0; s_voice[i].vol = 0; s_voice[i].age = 0; s_voice[i].on = 0; }
		}
	}

	u16 tone[3];
	u8 vol[3];
	tone[0] = tone[1] = tone[2] = 0;
	vol[0] = vol[1] = vol[2] = 0;
	u8 noise = 0;
	u8 mix = 0x38;												// tones on, noise off (bits 3-5 = 1)

	// music: a voice only does real work when a note starts or its envelope steps
	if (s_song && s_musicOn)
	{
		u8 sp = s_song->speed;
		for (u8 i = 0; i < 3; i++)
		{
			Voice* v = &s_voice[i];
			if (v->left == 0)
			{
				const u8* p = v->p;
				if (p[0] == 0xFF) p = s_song->tr[i];			// loop
				u8 n = p[0];
				v->left = p[1] * sp;
				v->p = p + 2;
				v->age = 0;
				v->on = (n != 0);
				if (n) { v->period = g_NotePeriod[n]; v->vol = env_vol(i, 0, v->left); }
			}
			else
			{
				v->left--; v->age++;
				if (v->on && (((v->age & 7) == 0) || v->left == 1)) v->vol = env_vol(i, v->age, v->left);
			}
			if (v->on) { tone[i] = v->period; vol[i] = v->vol; }
		}
	}

	// sound effect on channel A (replaces the melody)
	const SfxStep* sf = s_sfx;
	if (sf)
	{
		if (s_sfxLeft == 0)
		{
			sf++;
			if (sf->len == 0) { s_sfx = 0; sf = 0; }
			else { s_sfx = sf; s_sfxLeft = sf->len; }
		}
		else s_sfxLeft--;
		if (sf)
		{
			tone[0] = sf->tone; vol[0] = sf->vol; noise = sf->noise;
			if (sf->noise) mix &= ~0x08;						// noise on channel A
		}
	}

	if (!tone[0]) mix |= 0x01;
	if (!tone[1]) mix |= 0x02;
	if (!tone[2]) mix |= 0x04;
	for (u8 i = 0; i < 3; i++)
	{
		psg_set(i * 2, tone[i] & 0xFF);
		psg_set(i * 2 + 1, tone[i] >> 8);
		psg_set(8 + i, vol[i]);
	}
	psg_set(6, noise);
	psg_set(7, 0x80 | mix);										// bit 7 = I/O port B as output (MSX convention)
}

// ---- interrupt hook installation (H.TIMI = 0FD9Fh, 5 bytes) ----
#define H_TIMI			0xFD9F
static u8 s_oldHook[5];
static bool s_hooked = FALSE;

static void isr_entry(void) __naked
{
	__asm
		push	af
		push	bc
		push	de
		push	hl
		push	ix
		push	iy
		exx
		.db		0x08				; ex af,af,alt
		push	af
		push	bc
		push	de
		push	hl
		call	_snd_tick
		pop		hl
		pop		de
		pop		bc
		pop		af
		.db		0x08				; ex af,af,alt
		exx
		pop		iy
		pop		ix
		pop		hl
		pop		de
		pop		bc
		pop		af
		jp		_s_oldHook
	__endasm;
}

void snd_init(void)
{
	s_hz50 = VDP_GetFrequency() != 0;
	for (u8 r = 0; r < 14; r++) s_shadow[r] = 0xA5;		// impossible value: forces the first write
	if (s_hooked) return;
	u8* h = (u8*)H_TIMI;
	__asm__("di");
	for (u8 i = 0; i < 5; i++) s_oldHook[i] = h[i];
	h[0] = 0xC3; h[1] = (u8)((u16)isr_entry & 0xFF); h[2] = (u8)((u16)isr_entry >> 8);
	s_hooked = TRUE;
	__asm__("ei");
}

void snd_shutdown(void)
{
	if (!s_hooked) return;
	u8* h = (u8*)H_TIMI;
	__asm__("di");
	for (u8 i = 0; i < 5; i++) h[i] = s_oldHook[i];
	s_hooked = FALSE;
	for (u8 r = 8; r <= 10; r++) psg_w(r, 0);
	psg_w(7, 0xB8);
	__asm__("ei");
}

void ui_beep(u8 kind)
{
	__asm__("di");
	s_sfx = sfx_table[kind];
	s_sfxLeft = sfx_table[kind]->len;
	__asm__("ei");
}

void ui_sfx_update(void) { }			// effects are now played by the interrupt routine

void music_play(u8 id)  { s_newSong = id; }
void music_stop(void)   { s_newSong = 0xFE; }
bool music_toggle(void) { s_musicOn = !s_musicOn; return s_musicOn; }

volatile u8 g_keywait;			// incremented each time the game waits for a key (used by automated tests)

//-----------------------------------------------------------------------------
// Input: keyboard (BIOS buffer) + joystick ports 1 & 2 mapped to the same key codes
static u8 s_joyPrev[2] = { 0, 0 };
static u8 s_joyHold[2] = { 0, 0 };
static bool s_trgPrev[4] = { FALSE, FALSE, FALSE, FALSE };

void ui_init(void)
{
	g_keywait = 0;
	snd_init();
	for (u8 i = 0; i < 12; i++) rnd16();
}

static u8 dir_key(u8 d)
{
	switch (d)
	{
	case 1: return KEY_UP_ARROW;
	case 3: return KEY_RIGHT_ARROW;
	case 5: return KEY_DOWN_ARROW;
	case 7: return KEY_LEFT_ARROW;
	}
	return 0;
}

// Main-ROM call through CALSLT (MSX-DOS has RAM in page 0, so direct BIOS calls are not possible)
// A = input/output register
static u8 bios_call(u8 a, u16 addr) __naked
{
	a; addr;
	__asm
		push	ix
		push	iy
		push	de
		pop		ix				; IX = BIOS routine
		ld		iy, (#0xFCC0)	; IYh = main ROM slot (EXPTBL)
		call	#0x001C			; CALSLT
		ei
		pop		iy
		pop		ix
		ret
	__endasm;
}

#define R_CHGMOD_ADDR	0x005F
#define R_CHGET_ADDR	0x009F
#define R_GTSTCK_ADDR	0x00D5
#define R_GTTRIG_ADDR	0x00D8

void ui_text_mode(void)
{
	bios_call(0, R_CHGMOD_ADDR);
}

// Non blocking: only call CHGET when the keyboard buffer is not empty
static u8 kbd_poll(void)
{
	if (g_GETPNT == g_PUTPNT) return 0;
	return bios_call(0, R_CHGET_ADDR);
}

u8 ui_pollkey(void)
{
	u8 k = kbd_poll();
	if (k) return k;
	for (u8 p = 0; p < 2; p++)
	{
		u8 key = 0;
		g_psgBusy = TRUE;
		u8 d = bios_call(p + 1, R_GTSTCK_ADDR);
		if (d != s_joyPrev[p]) { s_joyPrev[p] = d; s_joyHold[p] = 0; key = dir_key(d); }
		else if (d && ++s_joyHold[p] >= 14) { s_joyHold[p] = 10; key = dir_key(d); }
		bool a = bios_call(1 + p, R_GTTRIG_ADDR) != 0, b = bios_call(3 + p, R_GTTRIG_ADDR) != 0;
		g_psgBusy = FALSE;
		bool ea = a && !s_trgPrev[p], eb = b && !s_trgPrev[2 + p];
		s_trgPrev[p] = a; s_trgPrev[2 + p] = b;
		if (key) return key;
		if (ea) return 13;
		if (eb) return 'A';
	}
	return 0;
}


u8 ui_getkey(void)
{
	u8 ph = 0, t = 0;
	g_keywait++;
	for (;;)
	{
		u8 k = ui_pollkey();
		if (k) { rnd16(); return k; }
		Halt();
		rnd16();
		ui_sfx_update();
		if (g_in && ((++t & 7) == 0)) gfx_flicker(ph++);
	}
}

u8 ui_getkey_upper(void)
{
	u8 k = ui_getkey();
	if (k >= 'a' && k <= 'z') k -= 32;
	return k;
}

void ui_wait(u8 frames)
{
	while (frames--)
	{
		Halt();
		ui_sfx_update();
		if (g_GETPNT != g_PUTPNT) return;
	}
}

void ui_pause_key(const c8* msg)
{
	con_putsn(msg, C_CREAM);
	ui_getkey();
}

// Line editor (name entry)
void ui_input_line(u16 x, u16 y, c8* buf, u8 max, u8 color)
{
	u8 n = 0;
	buf[0] = 0;
	for (;;)
	{
		gfx_text(x, y, buf, color, C_BLACK);
		gfx_text(x + n * 6, y, "_ ", C_GOLD, C_BLACK);
		u8 k = ui_getkey();
		if (k == 13) { gfx_text(x + n * 6, y, "  ", color, C_BLACK); return; }
		if ((k == 8 || k == 127) && n > 0) { n--; buf[n] = 0; gfx_text(x + n * 6, y, "   ", color, C_BLACK); continue; }
		if (k >= 32 && k < 127 && n < max)
		{
			if (k >= 'a' && k <= 'z') k -= 32;
			buf[n++] = k; buf[n] = 0;
		}
	}
}
