################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/public/base64.c \
../src/public/plt_Func.c \
../src/public/plt_Func_Crc32.c \
../src/public/plt_Time.c \
../src/public/plt_calc.c \
../src/public/plt_command.c \
../src/public/plt_command_exec.c \
../src/public/plt_func_register.c \
../src/public/plt_inner_data.c \
../src/public/plt_list.c \
../src/public/plt_point_rw.c \
../src/public/plt_pointtype.c \
../src/public/plt_pub_register.c \
../src/public/plt_setting.c \
../src/public/plt_shared_memory.c \
../src/public/plt_shell.c \
../src/public/plt_string_res.c \
../src/public/plt_trace.c \
../src/public/plt_var_register.c \
../src/public/plt_viewstring.c 

OBJS += \
./src/public/base64.o \
./src/public/plt_Func.o \
./src/public/plt_Func_Crc32.o \
./src/public/plt_Time.o \
./src/public/plt_calc.o \
./src/public/plt_command.o \
./src/public/plt_command_exec.o \
./src/public/plt_func_register.o \
./src/public/plt_inner_data.o \
./src/public/plt_list.o \
./src/public/plt_point_rw.o \
./src/public/plt_pointtype.o \
./src/public/plt_pub_register.o \
./src/public/plt_setting.o \
./src/public/plt_shared_memory.o \
./src/public/plt_shell.o \
./src/public/plt_string_res.o \
./src/public/plt_trace.o \
./src/public/plt_var_register.o \
./src/public/plt_viewstring.o 

C_DEPS += \
./src/public/base64.d \
./src/public/plt_Func.d \
./src/public/plt_Func_Crc32.d \
./src/public/plt_Time.d \
./src/public/plt_calc.d \
./src/public/plt_command.d \
./src/public/plt_command_exec.d \
./src/public/plt_func_register.d \
./src/public/plt_inner_data.d \
./src/public/plt_list.d \
./src/public/plt_point_rw.d \
./src/public/plt_pointtype.d \
./src/public/plt_pub_register.d \
./src/public/plt_setting.d \
./src/public/plt_shared_memory.d \
./src/public/plt_shell.d \
./src/public/plt_string_res.d \
./src/public/plt_trace.d \
./src/public/plt_var_register.d \
./src/public/plt_viewstring.d 


# Each subdirectory must supply rules for building sources it contributes
src/public/%.o: ../src/public/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_config" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_Export" -I"D:\SVN\Source\Berametal\zynq7015\libpub\src\h_public" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


