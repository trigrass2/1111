################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ipacs/src/system/myio/yc/ad.c \
../src/ipacs/src/system/myio/yc/ai.c \
../src/ipacs/src/system/myio/yc/ft.c \
../src/ipacs/src/system/myio/yc/yc.c 

OBJS += \
./src/ipacs/src/system/myio/yc/ad.o \
./src/ipacs/src/system/myio/yc/ai.o \
./src/ipacs/src/system/myio/yc/ft.o \
./src/ipacs/src/system/myio/yc/yc.o 

C_DEPS += \
./src/ipacs/src/system/myio/yc/ad.d \
./src/ipacs/src/system/myio/yc/ai.d \
./src/ipacs/src/system/myio/yc/ft.d \
./src/ipacs/src/system/myio/yc/yc.d 


# Each subdirectory must supply rules for building sources it contributes
src/ipacs/src/system/myio/yc/%.o: ../src/ipacs/src/system/myio/yc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -I"E:\xilinx\Zynq\libbm\src\h_bm" -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -I"E:\xilinx\Zynq\222\src\ipacs\h" -I"E:\xilinx\Zynq\222\src\ipacs\h\system" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio\yc" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\common" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio\yk" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio\yx" -I"E:\xilinx\Zynq\222\src\ipacs\h\product\protect" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\os" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\bsp" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


