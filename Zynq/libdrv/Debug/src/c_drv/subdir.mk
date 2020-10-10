################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/c_drv/hd_driver.c \
../src/c_drv/hd_fpga.c \
../src/c_drv/hd_fpga_deal.c \
../src/c_drv/hd_fpga_msg.c \
../src/c_drv/hd_fpga_test.c \
../src/c_drv/hd_isr.c \
../src/c_drv/os_memory.c \
../src/c_drv/os_none.c 

S_UPPER_SRCS += \
../src/c_drv/os_cpu_a.S 

OBJS += \
./src/c_drv/hd_driver.o \
./src/c_drv/hd_fpga.o \
./src/c_drv/hd_fpga_deal.o \
./src/c_drv/hd_fpga_msg.o \
./src/c_drv/hd_fpga_test.o \
./src/c_drv/hd_isr.o \
./src/c_drv/os_cpu_a.o \
./src/c_drv/os_memory.o \
./src/c_drv/os_none.o 

S_UPPER_DEPS += \
./src/c_drv/os_cpu_a.d 

C_DEPS += \
./src/c_drv/hd_driver.d \
./src/c_drv/hd_fpga.d \
./src/c_drv/hd_fpga_deal.d \
./src/c_drv/hd_fpga_msg.d \
./src/c_drv/hd_fpga_test.d \
./src/c_drv/hd_isr.d \
./src/c_drv/os_memory.d \
./src/c_drv/os_none.d 


# Each subdirectory must supply rules for building sources it contributes
src/c_drv/%.o: ../src/c_drv/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/c_drv/%.o: ../src/c_drv/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


