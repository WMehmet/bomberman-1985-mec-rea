@echo off
setlocal

echo ========================================================
echo   Bomberman 1985 MEC REA - Windows Quick Build Script
echo ========================================================
echo.

where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [HATA] GCC/MinGW bulunamadi!
    echo Lutfen w64devkit veya MSYS2 MinGW-w64 ortamini PATH'e ekleyin.
    pause
    exit /b 1
)

echo Kaynaklar derleniyor...
windres bomberman.rc -O coff -o bomberman_res.o

echo Program derleniyor...
gcc -O2 -Wall -Wextra -std=c99 src/main.c src/game.c src/graphics.c src/audio.c src/settings.c src/font.c src/scores.c bomberman_res.o -static -s -Wl,--insert-timestamp -Wl,--subsystem,windows:6.0 -Wl,-e,mainCRTStartup -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lmingw32 -lSDL2main -lSDL2 -lm -mwindows -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luxtheme -o bomberman.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [BASARILI] bomberman.exe basariyla derlendi!
    echo Calistirmak icin: bomberman.exe
) else (
    echo.
    echo [HATA] Derleme basarisiz oldu. Lutfen SDL2 kutuphanesinin kurulu oldugundan emin olun.
)

pause
