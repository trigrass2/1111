::´òÓ¡zynqBMÖ´ÐÐ³ÌÐò
setlocal ENABLEDELAYEDEXPANSION
echo off

SET BM_NAME=%1
SET Z_PATH=%~dp07z.exe
SET BMZ_FILE=%BM_NAME%.bmz
del /q /f %BMZ_FILE%
del /q /f %BM_NAME%*.txt
del /q /f %BM_NAME%.bm

arm-none-eabi-objcopy -O binary -S %BM_NAME%.elf %BM_NAME%.bm 
arm-none-eabi-objdump -t %BM_NAME%.elf > %BM_NAME%_objdump_t.txt 
arm-none-eabi-objdump -d %BM_NAME%.elf > %BM_NAME%_objdump_d.txt
arm-none-eabi-nm -a -n %BM_NAME%.elf > %BM_NAME%_nm.txt
arm-none-eabi-readelf -a %BM_NAME%.elf > %BM_NAME%_readelf.txt

%Z_PATH% a -tzip %BMZ_FILE% %BM_NAME%.bm
%Z_PATH% a -tzip %BMZ_FILE% %BM_NAME%_nm.txt
%Z_PATH% a -tzip %BMZ_FILE% %BM_NAME%_readelf.txt
%Z_PATH% a -tzip %BMZ_FILE% %BM_NAME%_objdump_t.txt

copy /y ..\src\Export\iPACS-5711.cfpx .\%BM_NAME%.cfpx
%Z_PATH% a -tzip %BMZ_FILE% %BM_NAME%.cfpx
del /q /f .\%BM_NAME%.cfpx

@echo output to "%BMZ_FILE%"!