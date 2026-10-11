//*****************************************************************************
// AKALABETH - MSX2 / MSX-DOS 2 port
//*****************************************************************************
ProjName   = "akalabeth";
ProjModules = [ "ramstore", ProjName, "gfx", "ui" ];
LibModules = [ "system", "bios", "vdp", "dos", "dos_mapper" ];
Machine    = "2";
Target     = "DOS2";
DOSParseArg = false;
Optim      = "Default";
CompileComplexity = "Default";
AppID      = "AK";
Verbose    = true;
