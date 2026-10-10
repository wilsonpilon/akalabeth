param([string]$Scn = "scenA", [string]$Machine = "Philips_NMS_8250", [string]$Disk = "dos1", [string]$Ext = "")
$sp = "D:\temp\user\claude\E--akalabeth\4b906f04-2e13-4ac8-afe1-1fc63e17fd4d\scratchpad"
Set-Location $sp
$prefix = $Scn.Substring(4)
Get-ChildItem "$sp\shots" -Filter "$($prefix)*.png" -ErrorAction SilentlyContinue | ForEach-Object { $_.Delete() }
$a = @("-machine", $Machine, "-diska", "$sp\$Disk", "-script", "$Scn.tcl")
if ($Ext -ne "") { $a = @("-machine", $Machine, "-ext", $Ext, "-diska", "$sp\$Disk", "-script", "$Scn.tcl") }
$p = Start-Process -FilePath "D:\msx\openmsx\openmsx.exe" -ArgumentList $a -PassThru
$null = $p.WaitForExit(500000)
if (-not $p.HasExited) { $p.Kill(); "TIMEOUT" }
Get-ChildItem "$sp\shots" -Filter "$($prefix)*.png" | Select-Object Name, Length | Format-Table -AutoSize | Out-String
