$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$out  = Join-Path (Split-Path -Parent $root) "FiveMExternal.zip"
if (Test-Path $out) { Remove-Item $out }
Compress-Archive -Path (Join-Path $root "*") -DestinationPath $out -CompressionLevel Optimal
Write-Host "[+] $out"