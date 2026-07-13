@echo off
setlocal
cd /d "%~dp0"
where gcc >nul 2>&1
if errorlevel 1 (
  echo gcc introuvable. Installez MinGW-w64.
  exit /b 1
)
gcc -std=c11 -Wall -Wextra -O2 -Iinclude src/main.c src/sensors.c src/access.c src/alert.c -o rfid_access.exe
if errorlevel 1 exit /b 1
echo Build OK: rfid_access.exe
rfid_access.exe %*
