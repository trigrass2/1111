################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/c_public/base64.c \
../src/c_public/plt_Func.c \
../src/c_public/plt_Func_Crc32.c \
../src/c_public/plt_Time.c \
../src/c_public/plt_list.c \
../src/c_public/plt_shared_memory.c \
../src/c_public/plt_string_res.c \
../src/c_public/plt_trace.c \
../src/c_public/plt_viewstring.c 

OBJS += \
./src/c_public/base64.o \
./src/c_public/plt_Func.o \
./src/c_public/plt_Func_Crc32.o \
./src/c_public/plt_Time.o \
./src/c_public/plt_list.o \
./src/c_public/plt_shared_memory.o \
./src/c_public/plt_string_res.o \
./src/c_public/plt_trace.o \
./src/c_public/plt_viewstring.o 

C_DEPS += \
./src/c_public/base64.d \
./src/c_public/plt_Func.d \
./src/c_public/plt_Func_Crc32.d \
./src/c_public/plt_Time.d \
./src/c_public/plt_list.d \
./src/c_public/plt_shared_memory.d \
./src/c_public/plt_string_res.d \
./src/c_public/plt_trace.d \
./src/c_public/plt_viewstring.d 


# Each subdirectory must supply rules for building sources it contributes
src/c_public/%.o: ../src/c_public/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -D_DEBUG -D_PROT_UNIT_ -Wall -O0 -g3 -I"E:\xilinx\Zynq\libpub\src\h_public" -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


