@echo off
echo ---------------------------------
echo    HMZ-RM1 Heceta Relay Module 
echo           Bacharach Inc
echo         Post Build Steps
echo                CPF
echo ---------------------------------
echo.
set project_name=%~1
echo.
echo Generating the CRC checksum...
C:\ST\STM32CubeIDE_1.4.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.win32_1.4.0.202007081208\tools\arm-none-eabi\bin\objcopy.exe -O ihex "%project_name%.elf"  "%project_name%.hex"
..\srecord-1.64-win32\srec_cat.exe "%project_name%.hex" -Intel -fill 0xFF 0x08000000 0x0801FFFC -STM32 0x0801FFFC -o "%project_name%_CRC.hex" -Intel
:: pause
exit


@echo off
:: Run post-build actions for this project.
:: %1 is the path to the project output dir (e.g. ${build_project}\${config_name:<projectname>})
:: %2 is the name of the bin file to append the CRC, without extension (e.g. <projectname>)

echo Generating binary with CRC from %1\%2.bin

:: Convert .elf to .bin
echo Converting %2.elf to %2.bin
call arm-none-eabi-objcopy -O binary "%1\%2.elf" "%1\%2.bin"

:: Append CRC to the .bin
echo Appending CRC to %2.bin
"%1\..\..\tools\CRCTool\Release\CRCTool.exe" "%1\%2.bin"

:: Convert the .bin file to an .srec file
echo Converting %2.bin to %2.srec
:: Address for --change-addresses must match FLASH in mem.ld
call arm-none-eabi-objcopy -I binary -O srec %1\%2.bin %1\%2.srec --srec-forceS3 --srec-len=32 --change-addresses 0x08010000