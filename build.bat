@echo off

echo --- COMPILING %1.c ---
gcc %1.c -o %1.exe -e mainCRTStartup -nostdlib -std=c89 -lwinmm -lgdi32 -lkernel32 -luser32 -ldwmapi -Os
echo --- STARTING %1.exe ---
%1.exe
echo --- DONE ---
