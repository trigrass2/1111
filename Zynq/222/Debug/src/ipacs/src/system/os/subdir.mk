################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ipacs/src/system/os/os.c 

OBJS += \
./src/ipacs/src/system/os/os.o 

C_DEPS += \
./src/ipacs/src/system/os/os.d 


# Each subdirectory must supply rules for building sources it contributes
src/ipacs/src/system/os/%.o: ../src/ipacs/src/system/os/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -I"E:\xilinx\Zynq\libbm\src\h_bm" -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -I"E:\xilinx\Zynq\222\src\ipacs\h" -I"E:\xilinx\Zynq\222\src\ipacs\h\system" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio\yc" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\common" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio\yk" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\myio\yx" -I"E:\xilinx\Zynq\222\src\ipacs\h\product\protect" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\os" -I"E:\xilinx\Zynq\222\src\ipacs\h\system\bsp" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


