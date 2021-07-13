mcc68k ship.c/ipath=..\segahdrs\,ship.src/strintext/mix=comments/opt=time,ship.lst
asm68k -o shipc.obj ship.src
mcc68k play.c/ipath=..\segahdrs\,play.src/strintext/mix=comments/opt=time,play.lst
asm68k -o play.obj play.src
asm68k ship.a
asm68k ship2.a
asm68k -Imusic\ -o music\gems.obj music\gems.a
lnk68k -c ship.cmd -o ship ..\segahdrs\intshell
:xray68k ship debug.bat/i
