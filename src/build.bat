pdp11-none-aout-gcc.exe -std=gnu2x -fomit-frame-pointer -msoft-float -fcprop-registers -fPIC -nostartfiles -nodefaultlibs -nostdlib -m10 -O2 -c -o bkgccgame.o bkgccgame.c
pause
pdp11-none-aout-as -mno-fpu -mlimited-eis -pic crt0.s -o crt0.o
pdp11-none-aout-as -mno-fpu -mlimited-eis -pic memory.s -o memory.o
pause
pdp11-none-aout-ld -T a.out.ld -Map bkgccgame.map -o bkgccgame.out crt0.o bkgccgame.o memory.o
pause
mkdir ..\bin
aout2bin bkgccgame.out ..\bin\bkgccgame.bin
del *.o
del *.map
del *.out
