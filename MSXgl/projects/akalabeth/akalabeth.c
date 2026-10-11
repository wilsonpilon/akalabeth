//=============================================================================
// AKALABETH - World of Doom  (MSX2 / MSX-DOS 2 port)
//
// Original game: Richard Garriott ("Lord British"), Apple II, 1979
// GW-BASIC port used as reference: Oscar Toledo G. (Nanochess), Jan-Feb 2019
// MSX2 port: MSXgl + SDCC
//=============================================================================
#include "akalabeth.h"
#include "dos.h"

//=============================================================================
// DATA
//=============================================================================
const c8* const g_MonsterName[11] =
{
	"", "SKELETON", "THIEF", "GIANT RAT", "ORC", "VIPER", "CARRION CRAWLER", "GREMLIN", "MIMIC", "DAEMON", "BALROG"
};
const c8* const g_WeaponName[6] = { "FOOD", "RAPIER", "AXE", "SHIELD", "BOW AND ARROWS", "MAGIC AMULET" };
const c8* const g_StatName[6] =
{
	"HIT POINTS.....", "STRENGTH.......", "DEXTERITY......", "STAMINA........", "WISDOM.........", "GOLD..........."
};

u8  g_te[MAPW][MAPW];
u8  g_seen[MAPW][MAPW];
u8  g_dn[DUNW][DUNW];
u8  g_tx, g_ty;
i8  g_px, g_py, g_dx, g_dy;
u8  g_in;
u8  g_lp;
i16 g_stat[6];
i16 g_inv[6];
c8  g_name[24];
u8  g_class;
u8  g_vis[10];
u8  g_visn;
bool g_visChest;

static u16 s_ln;				// lucky number
static i8  s_ta;				// quest target (negative = accomplished)
static i16 s_luck;				// "LK": hit points gained when leaving the dungeon
static bool s_extra;			// the last command printed more than its own echo line (worth a pause)
static bool s_noWait;			// the command already paused by itself
static bool s_pause;			// PA
static bool s_fade;				// do a fade transition on next scene draw

// Monsters of the current dungeon level
static u8  mz_alive[11];
static i16 mz_hp[11];
static i8  ml_x[11], ml_y[11];

#define TURN_DELAY		22
#define FOOD_MAX		30000

// Chance helpers: RND > p/1000
#define RNDGT(p)		(rnd16() > (u16)(((p) * 65535UL) / 1000UL))

//=============================================================================
// SMALL HELPERS
//=============================================================================
static u8 isqrt16(u16 n)
{
	u16 res = 0, bit = 0x4000;
	while (bit > n) bit >>= 2;
	while (bit)
	{
		if (n >= res + bit) { n -= res + bit; res = (res >> 1) + bit; }
		else res >>= 1;
		bit >>= 2;
	}
	return (u8)res;
}

static i8 sgn8(i8 v) { return v > 0 ? 1 : (v < 0 ? -1 : 0); }

static u8 num2s(i16 v, c8* o)
{
	c8 t[7]; u8 n = 0, m = 0;
	if (v < 0) { o[m++] = '-'; v = -v; }
	do { t[n++] = '0' + (u16)v % 10; v = (u16)v / 10; } while (v);
	while (n) o[m++] = t[--n];
	o[m] = 0;
	return m;
}

static u16 s2num(const c8* s)
{
	u32 v = 0;
	for (; *s >= '0' && *s <= '9'; s++) { v = v * 10 + (*s - '0'); if (v > 65535UL) v = 65535UL; }
	return (u16)v;
}

static bool s_dark = FALSE;

static void fade_out(void)
{
	if (s_dark) return;
	for (i8 l = 6; l >= 0; l--) { gfx_set_palette_fade(l); Halt(); Halt(); }
	s_dark = TRUE;
}

static void fade_in(void)
{
	for (u8 l = 1; l <= 7; l++) { gfx_set_palette_fade(l); Halt(); Halt(); }
	s_dark = FALSE;
}

// Text page helpers (x = 14, 10 px per line)
static void pline(u8 line, const c8* s, u8 color)
{
	gfx_text(14, 28 + line * 10, s, color, C_BLACK);
}

static void pcenter(u8 line, const c8* s, u8 color)
{
	gfx_text_c(28 + line * 10, s, color, C_BLACK);
}

static u8 wait_key_msg(u16 y, const c8* msg)
{
	gfx_text_c(y, msg, C_CREAM, C_BLACK);
	return ui_getkey_upper();
}

static void pause_or_wait(void)
{
	if (s_pause)
	{
		con_putsn("-CR- TO CONT. ", C_CREAM);
		ui_getkey();
	}
	else ui_wait(TURN_DELAY);
}

//=============================================================================
// WORLD GENERATION
//=============================================================================
static void gen_world(void)
{
	// borders = mountains
	for (u8 i = 0; i < MAPW; i++)
	{
		g_te[i][0] = T_MOUNTAIN; g_te[0][i] = T_MOUNTAIN; g_te[i][MAPW - 1] = T_MOUNTAIN; g_te[MAPW - 1][i] = T_MOUNTAIN;
	}
	// INT(RND^5 * 4.5): thresholds on RND for 1,2,3,4 = .740 .852 .922 .977
	for (u8 x = 1; x < MAPW - 1; x++)
		for (u8 y = 1; y < MAPW - 1; y++)
		{
			u16 r = rnd16();
			u8 t;
			if (r < 48489) t = 0;
			else if (r < 55803) t = T_MOUNTAIN;
			else if (r < 60430) t = T_TREES;
			else if (r < 64008) t = T_TOWN;
			else t = T_DUNGEON;
			if (t == T_TOWN && RNDGT(500)) t = 0;
			g_te[x][y] = t;
		}
	g_te[rnd_n(19) + 1][rnd_n(19) + 1] = T_CASTLE;
	g_tx = rnd_n(19) + 1;
	g_ty = rnd_n(19) + 1;
	g_te[g_tx][g_ty] = T_TOWN;			// you start in a town
	// make sure there is at least one dungeon entrance
	bool any = FALSE;
	for (u8 x = 1; x < MAPW - 1 && !any; x++)
		for (u8 y = 1; y < MAPW - 1; y++)
			if (g_te[x][y] == T_DUNGEON) { any = TRUE; break; }
	if (!any)
	{
		u8 x, y;
		do { x = rnd_n(19) + 1; y = rnd_n(19) + 1; } while (g_te[x][y] != 0);
		g_te[x][y] = T_DUNGEON;
	}
	for (u8 x = 0; x < MAPW; x++)
		for (u8 y = 0; y < MAPW; y++)
			g_seen[x][y] = 0;
}

static void spawn_monsters(void)
{
	for (u8 x = 1; x <= 10; x++)
	{
		mz_alive[x] = 0;
		mz_hp[x] = x + 3 + g_in;
		if ((i8)(x - 2) > (i8)g_in || RNDGT(400)) continue;
		u8 mx, my, tries = 0;
		do
		{
			mx = rnd_n(9) + 1; my = rnd_n(9) + 1;
			if (++tries > 250) break;
		} while (g_dn[mx][my] != 0 || (mx == g_px && my == g_py));
		if (tries > 250) continue;
		ml_x[x] = mx; ml_y[x] = my;
		g_dn[mx][my] = x * 10;
		mz_alive[x] = 1;
		mz_hp[x] = x * 2 + g_in * 2 * g_lp;
	}
}

// Line 500: dungeon level generation, seeded by (lucky number, entrance, level)
static void gen_level(void)
{
	u16 play = g_rngState;
	rnd_seed(s_ln * 31 + g_tx * 1009 + g_ty * 2003 + g_in * 4001 + 0x1357);
	for (u8 x = 0; x < DUNW; x++)
		for (u8 y = 0; y < DUNW; y++)
			g_dn[x][y] = 0;
	for (u8 x = 0; x <= 10; x++) { g_dn[x][0] = 1; g_dn[x][10] = 1; g_dn[0][x] = 1; g_dn[10][x] = 1; }
	for (u8 x = 2; x <= 8; x += 2)
		for (u8 y = 1; y <= 9; y++) { g_dn[x][y] = 1; g_dn[y][x] = 1; }
	for (u8 x = 2; x <= 8; x += 2)
		for (u8 y = 1; y <= 9; y += 2)
		{
			if (RNDGT(950)) g_dn[x][y] = 2;
			if (RNDGT(950)) g_dn[y][x] = 2;
			if (RNDGT(600)) g_dn[y][x] = 3;
			if (RNDGT(600)) g_dn[x][y] = 3;
			if (RNDGT(600)) g_dn[x][y] = 4;
			if (RNDGT(600)) g_dn[y][x] = 4;
			if (RNDGT(970)) g_dn[y][x] = 9;
			if (RNDGT(970)) g_dn[x][y] = 9;
			if (RNDGT(940)) g_dn[x][y] = 5;
			if (RNDGT(940)) g_dn[y][x] = 5;
		}
	g_dn[2][1] = 0;
	gfx_dmap_reset();
	if ((g_in & 1) == 0) { g_dn[7][3] = 7; g_dn[3][7] = 8; }
	else                 { g_dn[7][3] = 8; g_dn[3][7] = 7; }
	if (g_in == 1) { g_dn[1][1] = 8; g_dn[7][3] = 0; }
	spawn_monsters();
	g_rngState = play ^ (u16)g_JIFFY;		// back to the free-running play RNG
	if (!g_rngState) g_rngState = 1;
}

//=============================================================================
// SAVE / LOAD  (MSX-DOS 2 file handles; file AKALABET.SAV in the current directory)
//=============================================================================
#define SAVE_FILE		"AKALABET.SAV"

typedef struct
{
	c8  magic[4];
	u16 ln, rng, luck;
	i8  ta;
	u8  pause, lp, cls, in, tx, ty;
	i8  px, py, dx, dy;
	i16 stat[6], inv[6];
	c8  name[24];
	u8  te[MAPW][MAPW], seen[MAPW][MAPW], dn[DUNW][DUNW], dseen[DUNW][DUNW];
	u8  alive[11];
	i16 hp[11];
	i8  mx[11], my[11];
} SaveData;

static SaveData s_sv;

static bool dos_is_2(void)
{
	DOS_Version v;
	v.Kernel = 0; v.System = 0;
	DOS_GetVersion(&v);
	return (v.Kernel >> 8) >= 2;
}

static u8 mem_copy(void* dst, const void* src, u16 n)
{
	u8* d = (u8*)dst; const u8* s = (const u8*)src;
	while (n--) *d++ = *s++;
	return 0;
}

// returns 0 = ok, 1 = needs MSX-DOS 2, 2 = disk error
static u8 save_game(void)
{
	if (!dos_is_2()) return 1;
	SaveData* p = &s_sv;
	p->magic[0] = 'A'; p->magic[1] = 'K'; p->magic[2] = 'S'; p->magic[3] = '1';
	p->ln = s_ln; p->rng = g_rngState; p->luck = (u16)s_luck; p->ta = s_ta;
	p->pause = s_pause; p->lp = g_lp; p->cls = g_class; p->in = g_in; p->tx = g_tx; p->ty = g_ty;
	p->px = g_px; p->py = g_py; p->dx = g_dx; p->dy = g_dy;
	mem_copy(p->stat, g_stat, sizeof(g_stat)); mem_copy(p->inv, g_inv, sizeof(g_inv));
	mem_copy(p->name, g_name, sizeof(g_name));
	mem_copy(p->te, g_te, sizeof(g_te)); mem_copy(p->seen, g_seen, sizeof(g_seen));
	mem_copy(p->dn, g_dn, sizeof(g_dn)); mem_copy(p->dseen, g_dseen, sizeof(g_dseen));
	mem_copy(p->alive, mz_alive, sizeof(mz_alive)); mem_copy(p->hp, mz_hp, sizeof(mz_hp));
	mem_copy(p->mx, ml_x, sizeof(ml_x)); mem_copy(p->my, ml_y, sizeof(ml_y));
	u8 h = DOS_CreateHandle(SAVE_FILE, O_WRONLY, 0);
	if (h == HANDLE_INVALID) return 2;
	u16 w = DOS_WriteHandle(h, p, sizeof(SaveData));
	DOS_CloseHandle(h);
	return (w == sizeof(SaveData)) ? 0 : 2;
}

// returns 0 = ok, 1 = needs MSX-DOS 2, 2 = no (valid) save file
static u8 load_game(void)
{
	if (!dos_is_2()) return 1;
	SaveData* p = &s_sv;
	u8 h = DOS_OpenHandle(SAVE_FILE, O_RDONLY);
	if (h == HANDLE_INVALID) return 2;
	u16 r = DOS_ReadHandle(h, p, sizeof(SaveData));
	DOS_CloseHandle(h);
	if (r != sizeof(SaveData) || p->magic[0] != 'A' || p->magic[1] != 'K' || p->magic[2] != 'S' || p->magic[3] != '1') return 2;
	s_ln = p->ln; g_rngState = p->rng ? p->rng : 1; s_luck = (i16)p->luck; s_ta = p->ta;
	s_pause = p->pause; g_lp = p->lp; g_class = p->cls; g_in = p->in; g_tx = p->tx; g_ty = p->ty;
	g_px = p->px; g_py = p->py; g_dx = p->dx; g_dy = p->dy;
	mem_copy(g_stat, p->stat, sizeof(g_stat)); mem_copy(g_inv, p->inv, sizeof(g_inv));
	mem_copy(g_name, p->name, sizeof(g_name));
	mem_copy(g_te, p->te, sizeof(g_te)); mem_copy(g_seen, p->seen, sizeof(g_seen));
	mem_copy(g_dn, p->dn, sizeof(g_dn)); mem_copy(g_dseen, p->dseen, sizeof(g_dseen));
	mem_copy(mz_alive, p->alive, sizeof(mz_alive)); mem_copy(mz_hp, p->hp, sizeof(mz_hp));
	mem_copy(ml_x, p->mx, sizeof(ml_x)); mem_copy(ml_y, p->my, sizeof(ml_y));
	gfx_dmap_rebuild();
	return 0;
}

//=============================================================================
// SCENE
//=============================================================================
static void prompt(void)
{
	con_clear();
	for (i8 i = g_visn - 1; i >= 0; i--)
		con_puts(g_vis[i] == M_MIMIC ? "CHEST!" : g_MonsterName[g_vis[i]], C_RED);
	if (g_visChest) con_puts("CHEST!", C_GOLD);
	con_putsn("COMMAND? ", C_CREAM);
}

volatile u16 g_drawtime;		// frames spent drawing the last scene (profiling)
volatile u16 g_hudtime;			// frames spent updating the HUD
volatile u16 g_tparts[4];		// profiling: frames spent before/after each phase of a turn
volatile u16 g_turntime;		// frames from key press to next prompt

static void draw_scene(void)
{
	if (s_fade) { fade_out(); hud_draw_static(); }
	music_play(g_in ? MUS_DUNGEON : MUS_WORLD);
	u16 t0 = g_JIFFY;
	if (g_in == 0) gfx_draw_overworld(); else gfx_draw_dungeon();
	g_drawtime = g_JIFFY - t0;
	t0 = g_JIFFY;
	hud_update();
	g_hudtime = g_JIFFY - t0;
	if (s_fade) { s_fade = FALSE; fade_in(); }
}

//=============================================================================
// SHOP / STATUS
//=============================================================================
static const c8* const s_StatShort[6] = { "HIT POINTS", "STRENGTH", "DEXTERITY", "STAMINA", "WISDOM", "GOLD" };

// right aligned 3-character number field
static void num3(u16 x, u16 y, i16 v, u8 fg, u8 bg)
{
	c8 b[10], o[5];
	u8 n = num2s(v, b);
	u8 i = 0;
	for (; i + n < 3; i++) o[i] = ' ';
	for (u8 j = 0; j < n && i < 4; j++) o[i++] = b[j];
	o[i] = 0;
	gfx_text(x, y, o, fg, bg);
}

static void st_row_stat(u8 i)
{
	u16 y = 42 + i * 8;
	num3(78, y, g_stat[i], (i == ST_GOLD) ? C_GOLD : C_WHITE, C_NAVY);
	if (i != ST_GOLD)
	{
		u8 w = g_stat[i] > 40 ? 20 : g_stat[i] / 2;
		gfx_rect(100, y + 1, 21, 4, C_NAVY);
		gfx_rect(100, y + 1, w, 4, g_stat[i] >= 18 ? C_GRASS : (g_stat[i] <= 8 ? C_RED : C_GOLD));
	}
}

static void st_row_pack(u8 i)
{
	u16 y = 42 + i * 8;
	i16 q = g_inv[i];
	if (i == 0) q = (q + 5) / 10;
	num3(137, y, q, q ? C_WHITE : C_ST4, C_NAVY);
	gfx_text(159, y, g_WeaponName[i], q ? C_CREAM : C_ST4, C_NAVY);
}

static void draw_status(bool shop)
{
	gfx_page(shop ? "ADVENTURE SHOP" : "THY STATUS");
	gfx_panel(8, 26, 118, 70);
	gfx_panel(131, 26, 117, 70);
	gfx_text(14, 30, "STATS", C_GOLD, C_NAVY);
	gfx_text(137, 30, "PACK", C_GOLD, C_NAVY);
	for (u8 i = 0; i < 6; i++)
	{
		gfx_text(14, 42 + i * 8, s_StatShort[i], C_CREAM, C_NAVY);
		st_row_stat(i);
		st_row_pack(i);
	}
	if (shop)
	{
		static const c8* const price[6] = { "1 FOR 10", "8", "5", "6", "3", "15" };
		static const c8* const dmg[6]   = { "N/A", "1-10", "1-5", "1", "1-4", "?????" };
		gfx_panel(8, 100, 240, 64);
		gfx_text(14, 104, "PRICE", C_GOLD, C_NAVY);
		gfx_text(74, 104, "DAMAGE", C_GOLD, C_NAVY);
		gfx_text(134, 104, "ITEM", C_GOLD, C_NAVY);
		for (u8 i = 0; i < 6; i++)
		{
			u16 y = 113 + i * 8;
			gfx_text(14, y, price[i], C_GOLD, C_NAVY);
			gfx_text(74, y, dmg[i], C_CREAM, C_NAVY);
			c8 k[2] = { g_WeaponName[i][0], 0 };
			gfx_text(134, y, k, C_GRASS, C_NAVY);
			gfx_text(146, y, g_WeaponName[i], C_CREAM, C_NAVY);
		}
	}
}

static void shop(void)
{
	static const u8 cost[6] = { 1, 8, 5, 6, 3, 15 };
	music_play(MUS_COURT);
	fade_out();
	draw_status(TRUE);
	gfx_text_c(168, "WELCOME TO THE ADVENTURE SHOP", C_GOLD, C_BLACK);
	fade_in();
	for (;;)
	{
		gfx_rect(8, 176, 240, 29, C_BLACK);
		gfx_text_c(178, "WHICH ITEM SHALT THOU BUY?  (Q-QUIT)", C_CREAM, C_BLACK);
		u8 k = ui_getkey_upper();
		if (k == 'Q') { gfx_text_c(190, "BYE", C_GOLD, C_BLACK); ui_wait(20); return; }
		i8 z = -1;
		switch (k)
		{
		case 'F': z = 0; break;
		case 'R': z = 1; break;
		case 'A': z = 2; break;
		case 'S': z = 3; break;
		case 'B': z = 4; break;
		case 'M': z = 5; break;
		}
		gfx_rect(8, 176, 240, 29, C_BLACK);
		if (z < 0) { gfx_text_c(190, "I'M SORRY WE DON'T HAVE THAT.", C_RED, C_BLACK); ui_beep(SFX_BUMP); ui_wait(40); continue; }
		if (g_class == 'M' && (z == 1 || z == 4))
		{
			gfx_text_c(188, "I'M SORRY MAGES", C_RED, C_BLACK);
			gfx_text_c(197, "CAN'T USE THAT!", C_RED, C_BLACK);
			ui_beep(SFX_BUMP); ui_wait(50); continue;
		}
		if (g_stat[ST_GOLD] < cost[z])
		{
			gfx_text_c(188, "M'LORD THOU CAN NOT AFFORD", C_RED, C_BLACK);
			gfx_text_c(197, "THAT ITEM.", C_RED, C_BLACK);
			ui_beep(SFX_BUMP); ui_wait(50); continue;
		}
		if (z == 0) { if (g_inv[0] + 100 <= FOOD_MAX) g_inv[0] += 100; }
		else g_inv[z]++;
		g_stat[ST_GOLD] -= cost[z];
		ui_beep(SFX_BUY);
		st_row_stat(ST_GOLD);
		st_row_pack(z);
	}
}

static void status_page(void)
{
	fade_out();
	draw_status(FALSE);
	fade_in();
	wait_key_msg(190, "PRESS ENTER TO CONTINUE");
}

//=============================================================================
// COMBAT
//=============================================================================
// RND*20 - K < 0   <=>  rnd*20 < K
static bool rnd20_lt(i16 k)
{
	if (k <= 0) return FALSE;
	if (k >= 20) return TRUE;
	return (u32)rnd16() * 20UL < ((u32)k << 16);
}

static void mon_attack(u8 mm)
{
	bool normal = TRUE;
	if (mm == M_THIEF || mm == M_GREMLIN)
	{
		if (rnd16() >= 0x8000)
		{
			con_clear();
			if (mm == M_GREMLIN)
			{
				g_inv[0] = ((g_inv[0] / 20) * 10);			// INT(food / 2) in whole units
				con_puts("A GREMLIN STOLE SOME FOOD", C_RED);
				ui_beep(SFX_HURT);
				normal = FALSE;
			}
			else
			{
				bool any = FALSE;
				for (u8 z = 0; z < 6; z++) if (g_inv[z] >= (z ? 1 : 10)) any = TRUE;
				if (any)
				{
					u8 z;
					do z = rnd_n(6); while (g_inv[z] < (z ? 1 : 10));
					g_inv[z] -= z ? 1 : 10;
					con_putsn("A THIEF STOLE A ", C_RED);
					con_puts(g_WeaponName[z], C_RED);
					ui_beep(SFX_HURT);
					normal = FALSE;
				}
			}
		}
	}
	if (normal)
	{
		con_clear();
		con_puts("YOU ARE BEING ATTACKED", C_RED);
		con_putsn("BY A ", C_RED);
		con_puts(g_MonsterName[mm], C_RED);
		if (rnd20_lt((g_inv[3] > 0 ? 1 : 0) + g_stat[ST_STA] - mm - g_in))
		{
			con_puts("MISSED", C_GRASS);
			ui_beep(SFX_MISS);
		}
		else
		{
			con_puts("HIT", C_RED);
			g_stat[ST_HP] -= rnd_n(mm) + g_in;
			ui_beep(SFX_HURT);
		}
	}
	hud_update();
	pause_or_wait();
}

static void monsters_act(void)
{
	for (u8 mm = 1; mm <= 10; mm++)
	{
		if (!mz_alive[mm]) continue;
		i8 mx = ml_x[mm], my = ml_y[mm];
		i16 ddx = g_px - mx, ddy = g_py - my;
		i16 d2 = ddx * ddx + ddy * ddy;
		bool adj = d2 <= 1;						// RA < 1.3
		bool low = mz_hp[mm] < (i16)g_in * g_lp;
		if (!low)
		{
			if (adj) { mon_attack(mm); if (g_stat[ST_HP] <= 0) return; continue; }
			if (mm == M_MIMIC && d2 < 9) continue;
		}
		i8 x1 = sgn8(ddx), y1 = sgn8(ddy);
		if (low) { x1 = -x1; y1 = -y1; }
		bool tryx = TRUE;
		if (y1 != 0)
		{
			u8 d = g_dn[mx][my + y1];
			if (!(d == 1 || d > 9 || d == 2)) { x1 = 0; tryx = FALSE; }
		}
		if (tryx)
		{
			y1 = 0;
			if (x1 != 0)
			{
				u8 d = g_dn[mx + x1][my];
				if (d == 1 || d > 9 || d == 2) x1 = 0;
			}
		}
		if ((x1 || y1) && !(mx + x1 == g_px && my + y1 == g_py))
		{
			g_dn[mx][my] -= 10 * mm;
			mx += x1; my += y1;
			ml_x[mm] = mx; ml_y[mm] = my;
			g_dn[mx][my] += 10 * mm;
			continue;
		}
		if (low && adj) { mon_attack(mm); if (g_stat[ST_HP] <= 0) return; continue; }
		if (low) mz_hp[mm] += mm + g_in;
	}
}

// Resolve a player strike on monster 'mn' (0 = nothing in reach) with weapon damage 'da'
static void player_strike(u8 mn, u8 da)
{
	// RND*25 > C(2) - MN - IN  => miss
	i16 lim = g_stat[ST_DEX] - mn - g_in;
	bool miss = (mn < 1);
	if (!miss)
	{
		if (lim < 0) miss = TRUE;
		else if (lim < 25) miss = ((u32)rnd16() * 25UL > ((u32)lim << 16));
	}
	if (miss)
	{
		con_puts("YOU MISSED", C_RED);
		ui_beep(SFX_MISS);
		return;
	}
	ui_beep(SFX_HIT);
	con_puts("HIT!!!", C_GRASS);
	i32 dmg = (i32)(((u32)rnd16() * da) >> 8) + ((i32)g_stat[ST_STR] * 256) / 5;
	i32 hp = (i32)mz_hp[mn] * 256 - dmg + 128;
	mz_hp[mn] = (i16)(hp >> 8);
	con_putsn(g_MonsterName[mn], C_CREAM);
	con_putsn("'S HIT POINTS=", C_CREAM);
	con_putint(mz_hp[mn], C_WHITE);
	con_nl();
	if (mz_hp[mn] < 1)
	{
		i16 reward = mn + g_in;
		con_putsn("THOU HAST KILLED A ", C_GOLD);
		con_puts(g_MonsterName[mn], C_GOLD);
		con_puts("THOU SHALT RECEIVE", C_GOLD);
		con_putint(reward, C_WHITE);
		con_puts(" PIECES OF EIGHT", C_GOLD);
		g_stat[ST_GOLD] += reward;
		g_dn[ml_x[mn]][ml_y[mn]] -= 10 * mn;
		mz_alive[mn] = 0;
		s_luck += (mn * g_in) / 2;
		if (mn == s_ta) s_ta = -s_ta;
		ui_beep(SFX_COIN);
	}
}

// First monster in the line of fire (up to 5 cells)
static u8 line_of_fire(void)
{
	for (u8 y = 1; y <= 5; y++)
	{
		i8 x = g_px + g_dx * y, yy = g_py + g_dy * y;
		if (x < 1 || x > 9 || yy > 9 || yy < 0) return 0;
		u8 mn = g_dn[x][yy] / 10;
		if (mn > 0) return mn;
	}
	return 0;
}

static bool cmd_attack(void)
{
	u8 da;
	for (;;)
	{
		da = 0;
		con_puts("ATTACK", C_WHITE);
		con_putsn("WHICH WEAPON ", C_CREAM);
		u8 k = ui_getkey_upper();
		i8 z = -1;
		switch (k)
		{
		case 'R': da = 10; z = 1; con_puts("RAPIER", C_WHITE); break;
		case 'A': da = 5;  z = 2; con_puts("AXE", C_WHITE); break;
		case 'S': da = 1;  z = 3; con_puts("SHIELD", C_WHITE); break;
		case 'B': da = 4;  z = 4; con_puts("BOW", C_WHITE); break;
		case 'M': con_puts("MAGIC AMULET", C_BLUE); z = 5; break;
		default: break;
		}
		if (z > 0 && g_inv[z] < 1) { con_puts("NOT OWNED", C_RED); ui_beep(SFX_BUMP); ui_wait(30); con_clear(); continue; }
		if (k == 'M')
		{
			u8 q;
			if (g_class == 'F') q = rnd_n(4) + 1;
			else
			{
				con_puts("1-LADDER-UP 2-LADDER-DN", C_CREAM);
				con_puts("3-KILL      4-BAD??", C_CREAM);
				for (;;)
				{
					con_putsn("CHOICE ", C_CREAM);
					q = ui_getkey() - '0';
					if (q >= 1 && q <= 4) break;
					con_nl();
				}
				con_putint(q, C_WHITE); con_nl();
				if (RNDGT(750)) { con_puts("LAST CHARGE ON THIS AMULET!", C_RED); g_inv[5]--; }
			}
			ui_beep(SFX_MAGIC);
			switch (q)
			{
			case 1: con_puts("LADDER UP", C_BLUE);   g_dn[g_px][g_py] = 8; return TRUE;
			case 2: con_puts("LADDER DOWN", C_BLUE); g_dn[g_px][g_py] = 7; return TRUE;
			case 3:
				con_puts("MAGIC ATTACK", C_BLUE);
				player_strike(line_of_fire(), 10 + g_in);
				pause_or_wait();
				s_noWait = TRUE;
				return TRUE;
			default:
				switch (rnd_n(3))
				{
				case 0:
					con_puts("YOU HAVE BEEN TURNED", C_RED); con_puts("INTO A TOAD!", C_RED);
					for (u8 i = 1; i <= 4; i++) g_stat[i] = 3;
					break;
				case 1:
					con_puts("YOU HAVE BEEN TURNED", C_RED); con_puts("INTO A LIZARD MAN", C_RED);
					for (u8 i = 0; i <= 4; i++) g_stat[i] = (g_stat[i] * 5) / 2;
					break;
				default:
					con_puts("BACKFIRE", C_RED); g_stat[ST_HP] /= 2;
					break;
				}
				return TRUE;
			}
		}
		break;
	}
	if (da == 0) con_puts("HANDS", C_WHITE);
	u8 mn;
	if (da == 5 || da == 4)
	{
		if (da == 5)
		{
			con_putsn("TO THROW OR SWING:", C_CREAM);
			u8 k = ui_getkey_upper();
			if (k == 'T') { con_puts("THROW", C_WHITE); g_inv[2]--; mn = line_of_fire(); }
			else { con_puts("SWING", C_WHITE); mn = g_dn[g_px + g_dx][g_py + g_dy] / 10; }
		}
		else mn = line_of_fire();
	}
	else mn = g_dn[g_px + g_dx][g_py + g_dy] / 10;
	player_strike(mn, da);
	hud_update();
	pause_or_wait();
	s_noWait = TRUE;
	return TRUE;
}

//=============================================================================
// CASTLE / QUEST
//=============================================================================
static void quest_given(bool first)
{
	if (first)
	{
		pline(0, "GOOD! THOU SHALT TRY TO BECOME A", C_CREAM);
		pline(1, "KNIGHT!!!", C_GOLD);
		pline(3, "THY FIRST TASK IS TO GO INTO THE", C_CREAM);
		pline(4, "DUNGEONS AND TO RETURN ONLY AFTER", C_CREAM);
		pline(5, "KILLING A(N)", C_CREAM);
	}
	else
	{
		pline(0, "NOW THOU MUST KILL A(N)", C_CREAM);
	}
}

static void quest_tail(u8 line)
{
	pline(line,     "GO NOW UPON THIS QUEST, AND MAY", C_CREAM);
	pline(line + 1, "LADY LUCK BE FAIR UNTO YOU.....", C_CREAM);
	pline(line + 3, ".....ALSO I, BRITISH, HAVE INCREASED", C_GOLD);
	pline(line + 4, "EACH OF THY ATTRIBUTES BY ONE!", C_GOLD);
	wait_key_msg(190, "PRESS -SPACE- TO CONT.");
	for (u8 i = 0; i < 6; i++) g_stat[i]++;
}

static void castle(void)
{
	music_play(MUS_COURT);
	fade_out();
	gfx_page("LORD BRITISH'S CASTLE");
	fade_in();
	if (g_name[0] == 0)
	{
		pline(2, "WELCOME PEASANT INTO THE HALLS OF", C_CREAM);
		pline(3, "THE MIGHTY LORD BRITISH. HEREIN", C_CREAM);
		pline(4, "THOU MAYST CHOOSE TO DARE BATTLE", C_CREAM);
		pline(5, "WITH THE EVIL CREATURES OF THE", C_CREAM);
		pline(6, "DEPTHS, FOR GREAT REWARD!", C_CREAM);
		pline(8, "WHAT IS THY NAME PEASANT?", C_GOLD);
		ui_input_line(14, 118, g_name, 20, C_WHITE);
		pline(12, "DOEST THOU WISH FOR GRAND ADVENTURE?", C_GOLD);
		if (ui_getkey_upper() != 'Y')
		{
			pline(14, "THEN LEAVE AND BEGONE!", C_RED);
			g_name[0] = 0;
			wait_key_msg(190, "PRESS -SPACE- TO CONT.");
			return;
		}
		gfx_page("LORD BRITISH'S CASTLE");
		s_ta = g_stat[ST_WIS] / 3;
		if (s_ta < 1) s_ta = 1;
		if (s_ta > 10) s_ta = 10;
		quest_given(TRUE);
		gfx_text(14 + 13 * 6, 28 + 5 * 10, g_MonsterName[(u8)s_ta], C_RED, C_BLACK);
		quest_tail(8);
		return;
	}
	if (s_ta > 0)
	{
		pline(2, g_name, C_WHITE);
		pline(3, "WHY HAST THOU RETURNED?", C_CREAM);
		pline(5, "THOU MUST KILL A(N)", C_CREAM);
		pline(6, g_MonsterName[(u8)s_ta], C_RED);
		pline(8, "GO NOW AND COMPLETE THY QUEST!", C_GOLD);
		wait_key_msg(190, "PRESS -SPACE- TO CONT.");
		return;
	}
	pline(0, "AAHH!!......", C_GOLD);
	pline(1, g_name, C_WHITE);
	pline(3, "THOU HAST ACCOMPLISHED THY QUEST!", C_GRASS);
	ui_beep(SFX_FANFARE);
	if (-s_ta == 10)
	{
		gfx_page("KNIGHTHOOD");
		c8 nm[30] = "LORD ";
		u8 n = 5; for (u8 i = 0; g_name[i] && n < 28; i++) nm[n++] = g_name[i];
		nm[n] = 0;
		for (u8 i = 0; i < 24; i++) g_name[i] = nm[i];
		g_name[23] = 0;
		pcenter(3, g_name, C_GOLD);
		pline(5, "THOU HAST PROVED THYSELF WORTHY", C_CREAM);
		pline(6, "OF KNIGHTHOOD, CONTINUE PLAY IF THOU", C_CREAM);
		pline(7, "DOTH WISH, BUT THOU HAST ACCOMPLISHED", C_CREAM);
		pline(8, "THE MAIN OBJECTIVE OF THIS GAME...", C_CREAM);
		if (g_lp == 10)
		{
			pline(10, "...CALL CALIFORNIA PACIFIC COMPUTER", C_GOLD);
			pline(11, "AT (415)-569-9126 TO REPORT THIS", C_GOLD);
			pline(12, "AMAZING FEAT!", C_GOLD);
		}
		else
		{
			pline(10, "NOW MAYBE THOU ART FOOLHEARTY", C_GOLD);
			pline(11, "ENOUGH TO TRY DIFFICULTY LEVEL", C_GOLD);
			c8 b[4]; num2s(g_lp + 1, b);
			pline(12, b, C_WHITE);
		}
		wait_key_msg(190, "PRESS -SPACE- TO CONT.");
		for (u8 i = 0; i < 6; i++) g_stat[i]++;
		return;
	}
	pline(5, "UNFORTUNATELY, THIS IS NOT ENOUGH TO", C_CREAM);
	pline(6, "BECOME A KNIGHT.", C_CREAM);
	s_ta = -s_ta + 1;
	pline(8, "NOW THOU MUST KILL A(N)", C_CREAM);
	pline(9, g_MonsterName[(u8)s_ta], C_RED);
	pline(11, "GO NOW UPON THIS QUEST, AND MAY", C_CREAM);
	pline(12, "LADY LUCK BE FAIR UNTO YOU.....", C_CREAM);
	pline(14, ".....ALSO I, BRITISH, HAVE INCREASED", C_GOLD);
	pline(15, "EACH OF THY ATTRIBUTES BY ONE!", C_GOLD);
	wait_key_msg(190, "PRESS -SPACE- TO CONT.");
	for (u8 i = 0; i < 6; i++) g_stat[i]++;
}

//=============================================================================
// COMMANDS
//=============================================================================
static bool ow_move(i8 mx, i8 my, const c8* name)
{
	con_puts(name, C_WHITE);
	if (g_te[g_tx + mx][g_ty + my] == T_MOUNTAIN)
	{
		con_puts("YOU CAN'T PASS THE MOUNTAINS", C_RED);
		ui_beep(SFX_BUMP);
	}
	else
	{
		g_tx += mx; g_ty += my;
		ui_beep(SFX_STEP);
	}
	return TRUE;
}

static void change_level(i8 delta)
{
	g_in += delta;
	gen_level();
	s_fade = TRUE;
}

static bool cmd_forward(void)
{
	con_puts("FORWARD", C_WHITE);
	u8 d = g_dn[g_px + g_dx][g_py + g_dy];
	if (d != 1 && d < 10) { g_px += g_dx; g_py += g_dy; ui_beep(SFX_STEP); }
	else ui_beep(SFX_BUMP);
	u8 here = g_dn[g_px][g_py];
	if (here == 2)
	{
		con_puts("AAARRRGGGHHH!!! A TRAP!", C_RED);
		g_stat[ST_HP] -= rnd_n(g_in) + 3;
		ui_beep(SFX_TRAP);
		hud_update();
		con_putsn("FALLING TO LEVEL ", C_RED);
		con_putint(g_in + 1, C_WHITE);
		con_nl();
		ui_wait(30);
		change_level(1);
		return TRUE;
	}
	if (here == 5)
	{
		g_dn[g_px][g_py] = 0;
		con_puts("GOLD!!!!!", C_GOLD);
		i16 z = rnd_n(5 * g_in) + g_in;
		con_putint(z, C_WHITE);
		con_puts("-PIECES OF EIGHT", C_GOLD);
		g_stat[ST_GOLD] += z;
		u8 w = rnd_n(6);
		con_putsn("AND A ", C_GOLD);
		con_puts(g_WeaponName[w], C_WHITE);
		if (w == 0) { if (g_inv[0] + 10 <= FOOD_MAX) g_inv[0] += 10; } else g_inv[w]++;
		ui_beep(SFX_COIN);
	}
	return TRUE;
}

static bool cmd_enter(void)
{
	if (g_in == 0)
	{
		u8 t = g_te[g_tx][g_ty];
		if (t == T_TOWN)
		{
			fade_out(); shop(); s_fade = TRUE;
			return TRUE;
		}
		if (t == T_DUNGEON)
		{
			con_puts("GO DUNGEON", C_WHITE);
			ui_beep(SFX_DOWN);
			g_in = 1; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
			gen_level();
			s_fade = TRUE;
			return TRUE;
		}
		if (t == T_CASTLE)
		{
			fade_out(); castle(); s_fade = TRUE;
			return TRUE;
		}
		con_puts("HUH?", C_RED);
		return FALSE;
	}
	u8 c = g_dn[g_px][g_py];
	if (c == 7 || c == 9)
	{
		con_putsn("GO DOWN TO LEVEL ", C_WHITE);
		con_putint(g_in + 1, C_WHITE);
		con_nl();
		ui_beep(SFX_DOWN);
		change_level(1);
		return TRUE;
	}
	if (c != 8) { con_puts("HUH?", C_RED); return FALSE; }
	if (g_in == 1)
	{
		con_puts("LEAVE DUNGEON", C_WHITE);
		ui_beep(SFX_UP);
		g_in = 0;
		s_fade = TRUE;
		con_puts("THOU HAST GAINED", C_GOLD);
		con_putint(s_luck, C_WHITE);
		con_puts(" HIT POINTS", C_GOLD);
		g_stat[ST_HP] += s_luck;
		s_luck = 0;
		return TRUE;
	}
	con_putsn("GO UP TO LEVEL ", C_WHITE);
	con_putint(g_in - 1, C_WHITE);
	con_nl();
	ui_beep(SFX_UP);
	change_level(-1);
	return TRUE;
}

static void death(void)
{
	music_stop();
	ui_beep(SFX_DEATH);
	fade_out();
	gfx_page(0);
	// tombstone
	gfx_rect(96, 124, 64, 60, C_ST3);
	gfx_rect(100, 94, 56, 36, C_ST3);
	gfx_rect(106, 84, 44, 12, C_ST3);
	gfx_rect(112, 78, 32, 8, C_ST3);
	gfx_rect(100, 128, 56, 52, C_ST4);
	gfx_rect(106, 86, 44, 42, C_ST4);
	gfx_rect(112, 80, 32, 8, C_ST4);
	gfx_text(116, 100, "R.I.P.", C_BLACK, C_ST4);
	gfx_rect(126, 116, 4, 20, C_BLACK);
	gfx_rect(120, 121, 16, 4, C_BLACK);
	gfx_rect(88, 184, 80, 4, C_FOREST);
	pcenter(0, "WE MOURN THE PASSING OF", C_CREAM);
	c8 nm[40];
	u8 n = 0;
	const c8* who = g_name[0] ? g_name : "THE PEASANT";
	for (u8 i = 0; who[i] && n < 22; i++) nm[n++] = who[i];
	const c8* tail = " AND HIS COMPUTER";
	for (u8 i = 0; tail[i]; i++) nm[n++] = tail[i];
	nm[n] = 0;
	pcenter(1, nm, C_GOLD);
	pcenter(2, "TO INVOKE A MIRACLE OF RESURRECTION", C_CREAM);
	gfx_text_c(194, "<HIT ESC KEY>", C_RED, C_BLACK);
	fade_in();
	for (;;)
	{
		u8 k = ui_getkey();
		if (k == 27 || k == 13) break;
	}
}

// 0 = invalid (wait, re-prompt), 1 = a turn elapsed, 2 = nothing happened (re-prompt at once)
static u8 do_command(u8 k)
{
	bool dungeon = g_in > 0;
	switch (k)
	{
	case KEY_UP_ARROW:
		if (!dungeon) return ow_move(0, -1, "NORTH");
		return cmd_forward();
	case KEY_RIGHT_ARROW:
		if (!dungeon) return ow_move(1, 0, "EAST");
		con_puts("TURN RIGHT", C_WHITE);
		if (g_dx) { g_dy = g_dx; g_dx = 0; } else { g_dx = -g_dy; g_dy = 0; }
		ui_beep(SFX_SELECT);
		return TRUE;
	case KEY_LEFT_ARROW:
		if (!dungeon) return ow_move(-1, 0, "WEST");
		con_puts("TURN LEFT", C_WHITE);
		if (g_dx) { g_dy = -g_dx; g_dx = 0; } else { g_dx = g_dy; g_dy = 0; }
		ui_beep(SFX_SELECT);
		return TRUE;
	case KEY_DOWN_ARROW:
		if (!dungeon) return ow_move(0, 1, "SOUTH");
		con_puts("TURN AROUND", C_WHITE);
		g_dx = -g_dx; g_dy = -g_dy;
		ui_beep(SFX_SELECT);
		return TRUE;
	case 13:
		return cmd_enter();
	case 'A': case 'a':
		if (!dungeon) { con_puts("HUH?", C_RED); return FALSE; }
		return cmd_attack();
	case ' ':
		con_puts("PASS", C_WHITE);
		return TRUE;
	case 'S': case 's':
		status_page();
		s_fade = TRUE;
		draw_scene();
		return 2;
	case 'P': case 'p':
		s_pause = !s_pause;
		con_puts(s_pause ? "PAUSE ON" : "PAUSE OFF", C_WHITE);
		return 0;
	case 'M': case 'm':
		con_puts(music_toggle() ? "MUSIC ON" : "MUSIC OFF", C_WHITE);
		return 0;
	case 'W': case 'w':
	{
		con_putsn("SAVE GAME... ", C_WHITE);
		u8 r = save_game();
		if (r == 0) { con_puts("OK", C_GRASS); ui_beep(SFX_BUY); }
		else { con_puts(r == 1 ? "NEEDS MSX-DOS 2" : "DISK ERROR", C_RED); ui_beep(SFX_BUMP); }
		return 0;
	}
	case 'L': case 'l':
	{
		con_putsn("LOAD GAME... ", C_WHITE);
		u8 r = load_game();
		if (r == 0)
		{
			con_puts("OK", C_GRASS);
			ui_beep(SFX_UP);
			s_fade = TRUE;
			draw_scene();
			return 2;
		}
		con_puts(r == 1 ? "NEEDS MSX-DOS 2" : "NO SAVED GAME", C_RED);
		ui_beep(SFX_BUMP);
		return 0;
	}
	case 27:
		con_putsn("QUIT TO DOS (Y/N)? ", C_RED);
		if (ui_getkey_upper() == 'Y') { g_stat[ST_HP] = -999; return 1; }
		con_puts("NO", C_WHITE);
		return 0;
	}
	con_puts("HUH?", C_RED);
	return 0;
}

// Line 1090..1098
static bool end_turn(void)
{
	if (g_stat[ST_HP] == -999) return FALSE;				// quit
	g_inv[0] -= g_in ? 1 : 10;
	if (g_inv[0] < 0)
	{
		g_inv[0] = 0;
		g_stat[ST_HP] = 0;
		con_nl();
		con_puts("YOU HAVE STARVED!!!!!", C_RED);
		hud_update();
		ui_wait(50);
		return FALSE;
	}
	if (g_stat[ST_HP] <= 0) return FALSE;
	hud_update();
	if (g_in)
	{
		if (s_extra) pause_or_wait();
		con_clear();
		monsters_act();
		if (g_stat[ST_HP] <= 0) return FALSE;
	}
	return TRUE;
}

static void game_loop(void)
{
	draw_scene();
	prompt();
	for (;;)
	{
		u8 k = ui_getkey();
		u16 tt = g_JIFFY;
		u16 nl0 = con_nlcount();
		s_noWait = FALSE;
		u8 r = do_command(k);
		s_extra = (con_nlcount() > nl0 + 1) && !s_noWait;
		if (r != 1)
		{
			if (r == 0) ui_wait(TURN_DELAY);
			prompt();
			continue;
		}
		g_tparts[0] = g_JIFFY - tt;
		if (!end_turn()) return;
		g_tparts[1] = g_JIFFY - tt;
		draw_scene();
		g_tparts[2] = g_JIFFY - tt;
		prompt();
		g_turntime = g_JIFFY - tt;
		g_tparts[3] = g_turntime;
	}
}

//=============================================================================
// TITLE / CHARACTER CREATION
//=============================================================================
static bool title_screen(void)
{
	music_play(MUS_TITLE);
	// demo corridor rendered with the real 3D engine
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) g_dn[x][y] = 1;
	for (u8 x = 1; x <= 9; x++) g_dn[x][5] = 0;
	g_dn[4][4] = 0; g_dn[6][6] = 0; g_dn[4][3] = 0;
	g_dn[2][5] = 5;
	g_dn[7][5] = 7;
	g_dn[5][5] = M_BALROG * 10;
	g_dn[8][5] = 4;
	g_px = 1; g_py = 5; g_dx = 1; g_dy = 0; g_in = 1;
	g_dmapon = false;
	fade_out();
	gfx_clear(C_BLACK);
	gfx_draw_dungeon();
	hud_draw_static();
	// title over the 3D view
	gfx_text_big(35, 11, "AKALABETH", 3, C_BLACK);
	gfx_text_big(33, 9, "AKALABETH", 3, C_GOLD);
	gfx_text_c(38, "WORLD OF DOOM", C_CREAM, C_BLACK);
	// credits in the console area
	gfx_rect(CON_X, CON_Y, CON_COLS * 6, CON_LINES * 8, C_NAVY);
	gfx_text(CON_X, CON_Y, "ORIGINAL GAME", C_GOLD, C_NAVY);
	gfx_text(CON_X, CON_Y + 8, "RICHARD GARRIOTT, 1979", C_WHITE, C_NAVY);
	gfx_text(CON_X, CON_Y + 18, "GW-BASIC PORT", C_GOLD, C_NAVY);
	gfx_text(CON_X, CON_Y + 26, "NANOCHESS, 2019", C_WHITE, C_NAVY);
	gfx_text(CON_X, CON_Y + 36, "MSX2 PORT: MSXGL + SDCC", C_GRASS, C_NAVY);
	gfx_text(181, 169, "PRESS ANY", C_CREAM, C_NAVY);
	gfx_text(181, 178, "KEY, OR L", C_CREAM, C_NAVY);
	gfx_text(181, 187, "TO LOAD", C_GOLD, C_NAVY);
	gfx_text(181, 198, "V" AK_VERSION " DOS2", C_ST4, C_NAVY);
	fade_in();
	g_dmapon = true;
	for (;;)
	{
		u8 k = ui_getkey();
		if (k != 'L' && k != 'l') break;
		u8 r = load_game();
		if (r == 0) { s_fade = TRUE; return TRUE; }
		gfx_text(181, 187, r == 1 ? "NEEDS DOS2" : "NO SAVE   ", C_RED, C_NAVY);
		ui_beep(SFX_BUMP);
	}
	g_in = 0;
	return FALSE;
}

static void create_character(void)
{
	c8 buf[8];
	fade_out();
	gfx_page("THE LEGEND BEGINS");
	fade_in();
	pline(2, "TYPE THY LUCKY NUMBER.....", C_CREAM);
	ui_input_line(14 + 27 * 6, 48, buf, 5, C_WHITE);
	s_ln = s2num(buf);
	for (;;)
	{
		pline(4, "LEVEL OF PLAY(1-10)......", C_CREAM);
		gfx_rect(14 + 26 * 6, 68, 40, 8, C_BLACK);
		ui_input_line(14 + 26 * 6, 68, buf, 2, C_WHITE);
		g_lp = (u8)s2num(buf);
		if (g_lp >= 1 && g_lp <= 10) break;
		ui_beep(SFX_BUMP);
	}
	rnd_seed(s_ln);
	for (;;)
	{
		for (u8 i = 0; i < 6; i++) g_stat[i] = 4 + (isqrt16(rnd16()) * 21) / 256;
		gfx_rect(8, 88, 240, 118, C_BLACK);
		for (u8 i = 0; i < 6; i++)
		{
			gfx_text(34, 92 + i * 10, g_StatName[i], C_CREAM, C_BLACK);
			num2s(g_stat[i], buf);
			gfx_text(34 + 16 * 6, 92 + i * 10, buf, C_WHITE, C_BLACK);
			if (i != ST_GOLD)
				gfx_rect(34 + 16 * 6 + 20, 93 + i * 10, g_stat[i] * 3, 5, g_stat[i] >= 18 ? C_GRASS : (g_stat[i] <= 8 ? C_RED : C_GOLD));
		}
		gfx_text_c(160, "SHALT THOU PLAY WITH THESE QUALITIES?", C_GOLD, C_BLACK);
		ui_beep(SFX_SELECT);
		if (ui_getkey_upper() == 'Y') break;
	}
	for (;;)
	{
		gfx_rect(8, 160, 240, 46, C_BLACK);
		gfx_text_c(164, "AND SHALT THOU BE A FIGHTER OR A MAGE?", C_GOLD, C_BLACK);
		gfx_text_c(180, "(F)IGHTER  (M)AGE", C_CREAM, C_BLACK);
		u8 k = ui_getkey_upper();
		if (k == 'F' || k == 'M') { g_class = k; break; }
		ui_beep(SFX_BUMP);
	}
	for (u8 i = 0; i < 6; i++) g_inv[i] = 0;
	g_name[0] = 0;
	s_ta = 0; s_luck = 0; s_pause = FALSE; g_in = 0;
	g_dx = 1; g_dy = 0;
	gen_world();
	rnd_seed(g_JIFFY ^ 0x2B6D);
#if (AK_DEBUG == 1)
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000; g_inv[1] = 1; g_inv[2] = 2; g_inv[3] = 1; g_inv[4] = 1; g_inv[5] = 3;
	g_name[0] = 'T'; g_name[1] = 0;
	g_in = 1; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	g_dn[3][1] = 0;
	mz_alive[M_RAT] = 1; ml_x[M_RAT] = 3; ml_y[M_RAT] = 1; mz_hp[M_RAT] = 8; g_dn[3][1] = M_RAT * 10;
	s_fade = TRUE;
#elif (AK_DEBUG == 2)		// stand on the castle
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000;
	for (u8 x = 1; x < MAPW - 1; x++) for (u8 y = 1; y < MAPW - 1; y++) if (g_te[x][y] == T_CASTLE) { g_tx = x; g_ty = y; }
#ifdef AK_DBG_TA
	{ const c8* n = "SIR TEST"; u8 i = 0; for (; n[i]; i++) g_name[i] = n[i]; g_name[i] = 0; s_ta = AK_DBG_TA; }
#endif
	s_fade = TRUE;
#elif (AK_DEBUG == 3)		// die at once
	g_stat[ST_HP] = 1; g_inv[0] = 5000; g_name[0] = 'T'; g_name[1] = 0;
	g_in = 1; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	g_dn[2][1] = 0; g_dn[3][1] = 0;
	mz_alive[M_BALROG] = 1; ml_x[M_BALROG] = 2; ml_y[M_BALROG] = 1; mz_hp[M_BALROG] = 200; g_dn[2][1] = M_BALROG * 10;
	s_fade = TRUE;
#elif (AK_DEBUG == 4)		// dungeon with traps / holes / doors for rendering tests
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000; g_name[0] = 'T'; g_name[1] = 0;
	g_in = 2; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	g_dn[2][1] = 0; g_dn[1][1] = 0;
	g_dn[3][2] = 4; g_dn[4][1] = 0; g_dn[5][2] = 0; g_dn[6][1] = 5; g_dn[7][1] = 9;
	for (u8 i = 1; i <= 10; i++) mz_alive[i] = 0;
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) g_dn[x][y] %= 10;
	s_fade = TRUE;
#elif (AK_DEBUG == 5)		// trap right ahead on level 1
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000; g_name[0] = 'T'; g_name[1] = 0;
	g_in = 1; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	for (u8 i = 1; i <= 10; i++) mz_alive[i] = 0;
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) g_dn[x][y] %= 10;
	g_dn[2][1] = 2;
	s_fade = TRUE;
#elif (AK_DEBUG == 6)		// mimic (looks like a chest) two cells ahead, level 2
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000; g_inv[1] = 1; g_name[0] = 'T'; g_name[1] = 0;
	g_in = 2; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	for (u8 i = 1; i <= 10; i++) mz_alive[i] = 0;
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) g_dn[x][y] %= 10;
	g_dn[1][1] = 0; g_dn[2][1] = 0; g_dn[3][1] = 0; g_dn[4][1] = 0;
	mz_alive[M_MIMIC] = 1; ml_x[M_MIMIC] = 3; ml_y[M_MIMIC] = 1; mz_hp[M_MIMIC] = 30; g_dn[3][1] = M_MIMIC * 10;
	s_fade = TRUE;
#elif (AK_DEBUG == 7)		// magic amulet on level 2 (use the 'm' class)
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000; g_inv[5] = 5; g_name[0] = 'T'; g_name[1] = 0;
	g_in = 2; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	for (u8 i = 1; i <= 10; i++) mz_alive[i] = 0;
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) g_dn[x][y] %= 10;
	g_dn[1][1] = 0;
	s_fade = TRUE;
#elif (AK_DEBUG == 8)		// mimic three cells ahead (stays still) to test the amulet's KILL spell
	g_stat[ST_HP] = 60; g_stat[ST_GOLD] = 99; g_inv[0] = 5000; g_inv[5] = 30; g_name[0] = 'T'; g_name[1] = 0;
	g_in = 2; g_px = 1; g_py = 1; g_dx = 1; g_dy = 0;
	gen_level();
	for (u8 i = 1; i <= 10; i++) mz_alive[i] = 0;
	for (u8 x = 0; x < DUNW; x++) for (u8 y = 0; y < DUNW; y++) g_dn[x][y] %= 10;
	g_dn[1][1] = 0; g_dn[2][1] = 0; g_dn[3][1] = 0; g_dn[4][1] = 0;
	mz_alive[M_MIMIC] = 1; ml_x[M_MIMIC] = 4; ml_y[M_MIMIC] = 1; mz_hp[M_MIMIC] = 14; g_dn[4][1] = M_MIMIC * 10;
	s_fade = TRUE;
#else
	// first stop: the adventure shop
	fade_out();
	shop();
	s_fade = TRUE;
#endif
}

//=============================================================================
// MAIN
//=============================================================================
void main()
{
	gfx_init();
	ui_init();
	for (;;)
	{
		if (!title_screen()) create_character();
		game_loop();
		if (g_stat[ST_HP] == -999) break;
		death();
	}
	fade_out();
	gfx_restore_palette();
	ui_text_mode();
	snd_shutdown();
	DOS_Exit0();
}
