pdp11-none-aout-gcc.exe -DGAMELANG_EN -std=gnu2x -fomit-frame-pointer -msoft-float -fcprop-registers -fPIC -nostartfiles -nodefaultlibs -nostdlib -m10 -O2 -c -o bkgccgame_en.o bkgccgame.c
pdp11-none-aout-gcc.exe -DGAMELANG_RU -std=gnu2x -fomit-frame-pointer -msoft-float -fcprop-registers -fPIC -nostartfiles -nodefaultlibs -nostdlib -m10 -O2 -c -o bkgccgame_ru.o bkgccgame.c
pause
pdp11-none-aout-as -mno-fpu -mlimited-eis -pic crt0.s -o crt0.o
pdp11-none-aout-as -mno-fpu -mlimited-eis -pic memory.s -o memory.o
pause
pdp11-none-aout-ld -T a.out.ld -Map bkgccgame.map -o bkgccgame_en.out crt0.o bkgccgame_en.o memory.o
pdp11-none-aout-ld -T a.out.ld -Map bkgccgame.map -o bkgccgame_ru.out crt0.o bkgccgame_ru.o memory.o
pause
mkdir ..\bin
aout2bin bkgccgame_en.out ..\bin\bkgccgame_en.bin
aout2bin bkgccgame_ru.out ..\bin\bkgccgame_ru.bin
del *.o
del *.map
del *.out
