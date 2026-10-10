//*****************************************************************************
// AKALABETH - MSX2 / MSX-DOS 2 port
//*****************************************************************************
ProjName   = "akalabeth";
ProjModules = [ ProjName, "gfx", "ui" ];
LibModules = [ "system", "bios", "vdp", "print", "input", "memory", "draw", "math", "dos", "dos_mapper", "string", "psg" ];
Machine    = "2";
Target     = "DOS2";
DOSParseArg = false;
Optim      = "Default";
CompileComplexity = "Default";
AppID      = "AK";
Verbose    = true;
