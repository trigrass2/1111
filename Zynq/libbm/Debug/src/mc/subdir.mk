################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mc/plt_mc_inner_comm.c \
../src/mc/plt_mc_print.c 

OBJS += \
./src/mc/plt_mc_inner_comm.o \
./src/mc/plt_mc_print.o 

C_DEPS += \
./src/mc/plt_mc_inner_comm.d \
./src/mc/plt_mc_print.d 


# Each subdirectory must supply rules for building sources it contributes
src/mc/%.o: ../src/mc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_config" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_Export" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_public" -I"D:\SVN\Source\Berametal\zynq7015\libbm\src\h_baremetal" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"D:\SVN\Source\Berametal\zynq7015\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


