################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/ADC.c \
../Src/ByteFIFO.c \
../Src/CRC.c \
../Src/Command.c \
../Src/Configuration.c \
../Src/DRV8860.c \
../Src/Debug.c \
../Src/EEPROM.c \
../Src/Fault.c \
../Src/LED.c \
../Src/ModbusDataModel.c \
../Src/ModbusSlave.c \
../Src/OptionByte.c \
../Src/RAMIntegrity.c \
../Src/Relay.c \
../Src/SPIFlash.c \
../Src/Switches.c \
../Src/UART.c \
../Src/UUID.c \
../Src/delay.c \
../Src/main.c \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32l4xx.c 

OBJS += \
./Src/ADC.o \
./Src/ByteFIFO.o \
./Src/CRC.o \
./Src/Command.o \
./Src/Configuration.o \
./Src/DRV8860.o \
./Src/Debug.o \
./Src/EEPROM.o \
./Src/Fault.o \
./Src/LED.o \
./Src/ModbusDataModel.o \
./Src/ModbusSlave.o \
./Src/OptionByte.o \
./Src/RAMIntegrity.o \
./Src/Relay.o \
./Src/SPIFlash.o \
./Src/Switches.o \
./Src/UART.o \
./Src/UUID.o \
./Src/delay.o \
./Src/main.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32l4xx.o 

C_DEPS += \
./Src/ADC.d \
./Src/ByteFIFO.d \
./Src/CRC.d \
./Src/Command.d \
./Src/Configuration.d \
./Src/DRV8860.d \
./Src/Debug.d \
./Src/EEPROM.d \
./Src/Fault.d \
./Src/LED.d \
./Src/ModbusDataModel.d \
./Src/ModbusSlave.d \
./Src/OptionByte.d \
./Src/RAMIntegrity.d \
./Src/Relay.d \
./Src/SPIFlash.d \
./Src/Switches.d \
./Src/UART.d \
./Src/UUID.d \
./Src/delay.d \
./Src/main.d \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32L431xx -c -I../Inc -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

