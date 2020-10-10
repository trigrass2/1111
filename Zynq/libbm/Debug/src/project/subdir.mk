################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/project/bm_goose_rcv.c \
../src/project/bm_io.c \
../src/project/bm_protect.c \
../src/project/bm_sv_rcv.c \
../src/project/plt_Main.c \
../src/project/plt_bm_dbgmsg.c \
../src/project/plt_bm_inner_data.c \
../src/project/plt_bm_time.c \
../src/project/plt_component.c \
../src/project/plt_component_register.c \
../src/project/plt_diagram.c \
../src/project/plt_exception.c \
../src/project/plt_init.c \
../src/project/plt_interface.c \
../src/project/plt_load_config.c \
../src/project/plt_ram.c \
../src/project/plt_register.c \
../src/project/plt_signal.c \
../src/project/plt_soe_handle.c \
../src/project/plt_soe_send.c \
../src/project/plt_sysinfo.c \
../src/project/plt_task.c \
../src/project/plt_test.c 

S_UPPER_SRCS += \
../src/project/plt_exception_handler.S 

OBJS += \
./src/project/bm_goose_rcv.o \
./src/project/bm_io.o \
./src/project/bm_protect.o \
./src/project/bm_sv_rcv.o \
./src/project/plt_Main.o \
./src/project/plt_bm_dbgmsg.o \
./src/project/plt_bm_inner_data.o \
./src/project/plt_bm_time.o \
./src/project/plt_component.o \
./src/project/plt_component_register.o \
./src/project/plt_diagram.o \
./src/project/plt_exception.o \
./src/project/plt_exception_handler.o \
./src/project/plt_init.o \
./src/project/plt_interface.o \
./src/project/plt_load_config.o \
./src/project/plt_ram.o \
./src/project/plt_register.o \
./src/project/plt_signal.o \
./src/project/plt_soe_handle.o \
./src/project/plt_soe_send.o \
./src/project/plt_sysinfo.o \
./src/project/plt_task.o \
./src/project/plt_test.o 

S_UPPER_DEPS += \
./src/project/plt_exception_handler.d 

C_DEPS += \
./src/project/bm_goose_rcv.d \
./src/project/bm_io.d \
./src/project/bm_protect.d \
./src/project/bm_sv_rcv.d \
./src/project/plt_Main.d \
./src/project/plt_bm_dbgmsg.d \
./src/project/plt_bm_inner_data.d \
./src/project/plt_bm_time.d \
./src/project/plt_component.d \
./src/project/plt_component_register.d \
./src/project/plt_diagram.d \
./src/project/plt_exception.d \
./src/project/plt_init.d \
./src/project/plt_interface.d \
./src/project/plt_load_config.d \
./src/project/plt_ram.d \
./src/project/plt_register.d \
./src/project/plt_signal.d \
./src/project/plt_soe_handle.d \
./src/project/plt_soe_send.d \
./src/project/plt_sysinfo.d \
./src/project/plt_task.d \
./src/project/plt_test.d 


# Each subdirectory must supply rules for building sources it contributes
src/project/%.o: ../src/project/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_config" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_Export" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_public" -I"D:\SVN\Source\Berametal\zynq7015\libbm\src\h_baremetal" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"D:\SVN\Source\Berametal\zynq7015\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/project/%.o: ../src/project/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_config" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_Export" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_public" -I"D:\SVN\Source\Berametal\zynq7015\libbm\src\h_baremetal" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"D:\SVN\Source\Berametal\zynq7015\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


