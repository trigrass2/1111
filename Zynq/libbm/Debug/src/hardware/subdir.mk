################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hardware/plt_hd_driver.c \
../src/hardware/plt_hd_fpga.c \
../src/hardware/plt_hd_fpga_deal.c \
../src/hardware/plt_hd_fpga_msg.c \
../src/hardware/plt_hd_fpga_test.c \
../src/hardware/plt_hd_isr.c 

OBJS += \
./src/hardware/plt_hd_driver.o \
./src/hardware/plt_hd_fpga.o \
./src/hardware/plt_hd_fpga_deal.o \
./src/hardware/plt_hd_fpga_msg.o \
./src/hardware/plt_hd_fpga_test.o \
./src/hardware/plt_hd_isr.o 

C_DEPS += \
./src/hardware/plt_hd_driver.d \
./src/hardware/plt_hd_fpga.d \
./src/hardware/plt_hd_fpga_deal.d \
./src/hardware/plt_hd_fpga_msg.d \
./src/hardware/plt_hd_fpga_test.d \
./src/hardware/plt_hd_isr.d 


# Each subdirectory must supply rules for building sources it contributes
src/hardware/%.o: ../src/hardware/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_config" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_Export" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_public" -I"D:\SVN\Source\Berametal\zynq7015\libbm\src\h_baremetal" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"D:\SVN\Source\Berametal\zynq7015\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


