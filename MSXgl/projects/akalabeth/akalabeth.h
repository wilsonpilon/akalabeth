//=============================================================================
// AKALABETH - World of Doom  (MSX2 / MSX-DOS 2 port)
//
// Original game: Richard Garriott ("Lord British"), Apple II, 1979
// GW-BASIC port used as reference: Oscar Toledo G. (Nanochess), Jan-Feb 2019
// MSX2 port: MSXgl + SDCC
//=============================================================================
#pragma once
#include "msxgl.h"
#include "ramstore.h"

#define AK_VERSION		"0.4.1"

// Debug start: begin directly in dungeon level 1 with a rat in front (test builds only)
#define AK_DEBUG		0

//-----------------------------------------------------------------------------
// Palette (SCREEN 5, 16 entries)
enum
{
	C_BLACK = 0,
	C_NAVY,			// 1  panel background / dark floor
	C_ST1,			// 2  stone, far
	C_ST2,			// 3  stone
	C_ST3,			// 4  stone
	C_ST4,			// 5  stone, near
	C_BLUE,			// 6  magic / thief
	C_FLOOR,		// 7  dungeon floor
	C_CEIL,			// 8  dungeon ceiling
	C_CREAM,		// 9  outline / highlight text
	C_WOOD,			// 10 wood / doors
	C_RED,			// 11 monsters / danger
	C_GRASS,		// 12 grass / good
	C_FOREST,		// 13 dark green
	C_GOLD,			// 14 gold
	C_WHITE			// 15
};

//-----------------------------------------------------------------------------
// RAM store: AKALABET.DAT = 16 KB segments; each picture takes two (rows 0..127, rows 128..211)
enum { ART_INTRO = 0, ART_HELP = 2, ART_OUTRO = 4, MUS_SEG = 6, ART_SEGS = 7 };	// MUS_SEG: all songs (see tools/gen_music.js)

//-----------------------------------------------------------------------------
// Geometry
#define VIEW_W			256
#define VIEW_H			160			// 3D / overworld viewport (y = 0..159)
#define CON_X			6			// message console
#define CON_Y			165
#define CON_LINES		5
#define CON_COLS		27
#define STAT_X			178			// stats panel

#define OW_X0			52			// overworld 3x3 tile area
#define OW_Y0			5
#define OW_TS			50

#define MAPW			21			// overworld size
#define DUNW			11			// dungeon size

//-----------------------------------------------------------------------------
// Game data
enum { ST_HP = 0, ST_STR, ST_DEX, ST_STA, ST_WIS, ST_GOLD };

enum
{
	T_GRASS = 0, T_MOUNTAIN, T_TREES, T_TOWN, T_DUNGEON, T_CASTLE
};

enum
{
	M_NONE = 0, M_SKELETON, M_THIEF, M_RAT, M_ORC, M_VIPER, M_CRAWLER, M_GREMLIN, M_MIMIC, M_DAEMON, M_BALROG
};

extern const c8* const g_MonsterName[11];
extern const c8* const g_WeaponName[6];
extern const c8* const g_StatName[6];

// World state
extern u8  g_te[MAPW][MAPW];			// overworld
extern u8  g_seen[MAPW][MAPW];			// explored flag (minimap)
extern u8  g_dn[DUNW][DUNW];			// dungeon: type + 10 * monster
extern u8  g_tx, g_ty;					// overworld position
extern i8  g_px, g_py, g_dx, g_dy;		// dungeon position and heading
extern u8  g_in;						// dungeon level (0 = overworld)
extern u8  g_lp;						// level of play

// Player
extern i16 g_stat[6];
extern i16 g_inv[6];					// [0] food in tenths, [1..5] weapons
extern c8  g_name[24];
extern u8  g_class;						// 'F' or 'M'

// Monsters currently visible (filled by the 3D renderer, used to label them)
extern u8  g_vis[10];
extern u8  g_visn;
extern bool g_visChest;

//-----------------------------------------------------------------------------
// gfx.c
void gfx_init(void);
void gfx_rect(u16 x, u16 y, u16 w, u16 h, u8 c);
void gfx_hline(i16 x0, i16 x1, i16 y, u8 c);
void gfx_vline(i16 x, i16 y0, i16 y1, u8 c);
void gfx_line(i16 x0, i16 y0, i16 x1, i16 y1, u8 c);
void gfx_box(i16 x, i16 y, i16 w, i16 h, u8 c);			// outline only
void gfx_clear(u8 c);
void gfx_text(u16 x, u16 y, const c8* s, u8 fg, u8 bg);
void gfx_text_c(u16 y, const c8* s, u8 fg, u8 bg);		// centred horizontally
void gfx_panel(u16 x, u16 y, u16 w, u16 h);				// framed panel
void gfx_page(const c8* title);							// full-screen decorated page
void gfx_draw_dungeon(void);
void gfx_draw_overworld(void);
void gfx_target_front(bool on);
bool gfx_art(u8 idx);							// full-screen picture from the RAM store (FALSE: not available)
void gfx_text_big(u16 x, u16 y, const c8* s, u8 scale, u8 color);
void gfx_restore_palette(void);
void gfx_flicker(u8 phase);
void gfx_dmap_reset(void);
void gfx_dmap_rebuild(void);
void gfx_cheat_map(u8 k);
extern u8 g_cheatMap;
extern bool g_cheat;						// test cheats enabled (TAB on the title screen)
extern u8 g_dseen[DUNW][DUNW];
extern bool g_dmapon;
void gfx_set_palette_fade(u8 level);					// 0..7, 7 = normal

// ui.c
#define KEY_UP_ARROW	30
#define KEY_DOWN_ARROW	31
#define KEY_LEFT_ARROW	29
#define KEY_RIGHT_ARROW	28

void ui_init(void);
void ui_text_mode(void);
void con_clear(void);
void con_puts(const c8* s, u8 color);					// print a line (advances)
void con_putsn(const c8* s, u8 color);					// print w/o newline
void con_putint(i16 v, u8 color);
void con_nl(void);
void hud_draw_static(void);
void hud_update(void);
void hud_invalidate(void);
u16  con_nlcount(void);
void gfx_ow_invalidate(void);
u8   ui_getkey(void);									// blocking, with idle animation
u8   ui_getkey_upper(void);
u8   ui_pollkey(void);									// non blocking, 0 = none
void ui_wait(u8 frames);								// skippable delay
void ui_pause_key(const c8* msg);						// "-CR- TO CONT."
void ui_input_line(u16 x, u16 y, c8* buf, u8 max, u8 color);
u16  ui_frames(void);
void ui_beep(u8 kind);
void snd_init(void);
void snd_shutdown(void);
void music_play(u8 id);
void music_pump(void);
void music_stop(void);
bool music_toggle(void);
extern volatile u16 g_sndTicks;
void ui_sfx_update(void);

enum { SFX_STEP = 0, SFX_BUMP, SFX_HIT, SFX_MISS, SFX_HURT, SFX_COIN, SFX_TRAP, SFX_DOWN, SFX_UP, SFX_DEATH, SFX_MAGIC, SFX_SELECT, SFX_BUY, SFX_FANFARE };

#define MUS_TITLE 0
#define MUS_WORLD 1
#define MUS_DUNGEON 2
#define MUS_COURT 3
#define MUS_VICTORY 4

// rng
u16  rnd16(void);
u16  rnd_n(u16 n);										// 0..n-1
void rnd_seed(u16 s);
extern u16 g_rngState;
