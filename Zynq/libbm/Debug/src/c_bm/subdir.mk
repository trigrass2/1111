################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/c_bm/bm_goose_rcv.c \
../src/c_bm/bm_io.c \
../src/c_bm/bm_sv_rcv.c \
../src/c_bm/bm_xuartns550.c \
../src/c_bm/icomm_base_class.c \
../src/c_bm/icomm_config.c \
../src/c_bm/icomm_port.c \
../src/c_bm/plt_Main.c \
../src/c_bm/plt_bm_dbgmsg.c \
../src/c_bm/plt_bm_time.c \
../src/c_bm/plt_exception.c \
../src/c_bm/plt_init.c \
../src/c_bm/plt_mc_inner_comm.c \
../src/c_bm/plt_mc_print.c \
../src/c_bm/plt_ram.c \
../src/c_bm/plt_soe_handle.c \
../src/c_bm/plt_soe_send.c \
../src/c_bm/plt_task.c \
../src/c_bm/plt_test.c 

S_UPPER_SRCS += \
../src/c_bm/plt_exception_handler.S 

OBJS += \
./src/c_bm/bm_goose_rcv.o \
./src/c_bm/bm_io.o \
./src/c_bm/bm_sv_rcv.o \
./src/c_bm/bm_xuartns550.o \
./src/c_bm/icomm_base_class.o \
./src/c_bm/icomm_config.o \
./src/c_bm/icomm_port.o \
./src/c_bm/plt_Main.o \
./src/c_bm/plt_bm_dbgmsg.o \
./src/c_bm/plt_bm_time.o \
./src/c_bm/plt_exception.o \
./src/c_bm/plt_exception_handler.o \
./src/c_bm/plt_init.o \
./src/c_bm/plt_mc_inner_comm.o \
./src/c_bm/plt_mc_print.o \
./src/c_bm/plt_ram.o \
./src/c_bm/plt_soe_handle.o \
./src/c_bm/plt_soe_send.o \
./src/c_bm/plt_task.o \
./src/c_bm/plt_test.o 

S_UPPER_DEPS += \
./src/c_bm/plt_exception_handler.d 

C_DEPS += \
./src/c_bm/bm_goose_rcv.d \
./src/c_bm/bm_io.d \
./src/c_bm/bm_sv_rcv.d \
./src/c_bm/bm_xuartns550.d \
./src/c_bm/icomm_base_class.d \
./src/c_bm/icomm_config.d \
./src/c_bm/icomm_port.d \
./src/c_bm/plt_Main.d \
./src/c_bm/plt_bm_dbgmsg.d \
./src/c_bm/plt_bm_time.d \
./src/c_bm/plt_exception.d \
./src/c_bm/plt_init.d \
./src/c_bm/plt_mc_inner_comm.d \
./src/c_bm/plt_mc_print.d \
./src/c_bm/plt_ram.d \
./src/c_bm/plt_soe_handle.d \
./src/c_bm/plt_soe_send.d \
./src/c_bm/plt_task.d \
./src/c_bm/plt_test.d 


# Each subdirectory must supply rules for building sources it contributes
src/c_bm/%.o: ../src/c_bm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -I"E:\xilinx\Zynq\libbm\src\h_bm" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/c_bm/%.o: ../src/c_bm/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -I"E:\xilinx\Zynq\libdrv\src\h_drv" -I"E:\xilinx\Zynq\libbm\src\h_bm" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I"E:\xilinx\Zynq\zynqBM_bsp\ps7_cortexa9_1\include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


