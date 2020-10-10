################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/icomm/icomm_base_class.c \
../src/icomm/icomm_config.c \
../src/icomm/icomm_port.c 

OBJS += \
./src/icomm/icomm_base_class.o \
./src/icomm/icomm_config.o \
./src/icomm/icomm_port.o 

C_DEPS += \
./src/icomm/icomm_base_class.d \
./src/icomm/icomm_config.d \
./src/icomm/icomm_port.d 


# Each subdirectory must supply rules for building sources it contributes
src/icomm/%.o: ../src/icomm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_config" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_Export" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_public" -I"D:\SVN\Source\Berametal\zynq7015\libbm\src\h_baremetal" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"D:\SVN\Source\Berametal\zynq7015\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


