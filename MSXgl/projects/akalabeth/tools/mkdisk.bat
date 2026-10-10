@echo off
:: Builds ..\..\..\dist\akalabeth.dsk (720 KB, MSX-DOS 2) with msxdisk.exe from out\akalabeth.com
setlocal
set PROJ=%~dp0..
set ROOT=%PROJ%\..\..\..
set DOS=%PROJ%\..\..\tools\build\DOS
set DIST=%ROOT%\dist
if not exist "%DIST%\stage" mkdir "%DIST%\stage"
pushd "%DIST%\stage"
del /q * 2>nul
if exist ..\akalabeth.dsk del ..\akalabeth.dsk
copy /y "%PROJ%\out\akalabeth.com" AKALABET.COM >nul
<nul set /p =AKALABET> AUTOEXEC.BAT
echo.>> AUTOEXEC.BAT
copy /y "%DOS%\COMMAND2.COM" . >nul
copy /y "%DOS%\MSXDOS2.SYS" . >nul
"%ROOT%\msxdisk.exe" create ..\akalabeth.dsk
"%ROOT%\msxdisk.exe" add ..\akalabeth.dsk AUTOEXEC.BAT AKALABET.COM COMMAND2.COM MSXDOS2.SYS
copy /y AKALABET.COM .. >nul
popd
rmdir /s /q "%DIST%\stage"
"%ROOT%\msxdisk.exe" list "%DIST%\akalabeth.dsk" -l
