################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/HDC1080.c \
../Core/Src/MQTT.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/mh19z.c \
../Core/Src/stm32f103xx_CMSIS.c \
../Core/Src/stm32f103xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/tasks_list.c 

OBJS += \
./Core/Src/HDC1080.o \
./Core/Src/MQTT.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/mh19z.o \
./Core/Src/stm32f103xx_CMSIS.o \
./Core/Src/stm32f103xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/tasks_list.o 

C_DEPS += \
./Core/Src/HDC1080.d \
./Core/Src/MQTT.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/mh19z.d \
./Core/Src/stm32f103xx_CMSIS.d \
./Core/Src/stm32f103xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/tasks_list.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32F103xB -c -I../Core/Inc -I../Drivers/CMSIS -I../FreeRTOS/include -I../FreeRTOS -I../FreeRTOS/portable -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

