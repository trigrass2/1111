################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Export_if/plt_if_build.c 

OBJS += \
./src/Export_if/plt_if_build.o 

C_DEPS += \
./src/Export_if/plt_if_build.d 


# Each subdirectory must supply rules for building sources it contributes
src/Export_if/%.o: ../src/Export_if/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -I"E:\xilinx\Zynq\libbm\src\h_bm" -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -I"${workspace_loc:/$(ProjName)/src}" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


