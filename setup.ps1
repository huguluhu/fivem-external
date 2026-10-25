$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$vendor = Join-Path $root "vendor"
New-Item -ItemType Directory -Force -Path $vendor | Out-Null

if (-not (Test-Path "$vendor\imgui\imgui.cpp")) {
    Write-Host "[*] Downloading ImGui v1.90.9 ..."
    Invoke-WebRequest "https://github.com/ocornut/imgui/archive/refs/tags/v1.90.9.zip" -OutFile "$vendor\imgui.zip"
    Expand-Archive "$vendor\imgui.zip" -DestinationPath "$vendor" -Force
    Rename-Item "$vendor\imgui-1.90.9" "$vendor\imgui"
    Remove-Item "$vendor\imgui.zip"
}

if (-not (Test-Path "$vendor\lua\src\lua.h")) {
    Write-Host "[*] Downloading Lua 5.4.6 ..."
    Invoke-WebRequest "https://www.lua.org/ftp/lua-5.4.6.tar.gz" -OutFile "$vendor\lua.tar.gz"
    tar -xzf "$vendor\lua.tar.gz" -C "$vendor"
    Rename-Item "$vendor\lua-5.4.6" "$vendor\lua"
    Remove-Item "$vendor\lua.tar.gz"
}

Write-Host "[+] Dependencies ready."