@echo off
setlocal enabledelayedexpansion

set CC=gcc
set CFLAGS=-std=c11 -Wall -Wextra -O2
if "%1"=="--debug" (
  set CFLAGS=-std=c11 -Wall -Wextra -O0 -g -fsanitize=address,undefined
  shift
)

set SRC=interrupts.c
set HDR=interrupts.h
set OUT=interrupts.exe

if not exist "%SRC%" ( echo Error: %SRC% not found.& exit /b 1 )
if not exist "%HDR%" ( echo Error: %HDR% not found.& exit /b 1 )

echo Compiling...
%CC% %CFLAGS% "%SRC%" -o "%OUT%"
if errorlevel 1 ( echo Compilation failed.& exit /b 1 )

echo Running .\%OUT%
.\%OUT% %*
endlocal