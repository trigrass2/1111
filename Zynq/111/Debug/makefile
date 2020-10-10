################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/ipacs/src/system/os/subdir.mk
-include src/ipacs/src/system/myio/yx/subdir.mk
-include src/ipacs/src/system/myio/yk/subdir.mk
-include src/ipacs/src/system/myio/yc/subdir.mk
-include src/ipacs/src/system/myio/subdir.mk
-include src/ipacs/src/system/common/subdir.mk
-include src/ipacs/src/system/bsp/subdir.mk
-include src/ipacs/src/protocol/subdir.mk
-include src/ipacs/src/product/protect/subdir.mk
-include src/ipacs/src/subdir.mk
-include src/Export_if/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
111.elf.size \
111.elf.size \


# All Target
all: pre-build main-build

# Main-build Target
main-build: 111.elf secondary-outputs

# Tool invocations
111.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM v7 g++ linker'
	arm-none-eabi-g++ -L"E:\xilinx\Zynq\libbm\lib" -L"E:\xilinx\Zynq\libdrv\lib" -L"E:\xilinx\Zynq\libpub\lib" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wl,-build-id=none -specs=Xilinx.spec -Wl,-T -Wl,../src/lscript.ld -L../../zynqBM_bsp/ps7_cortexa9_1/lib -o "111.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

111.elf.size: 111.elf
	@echo 'Invoking: ARM v7 Print Size'
	arm-none-eabi-size 111.elf  |tee "111.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(S_UPPER_DEPS)$(C_DEPS)$(CPP_DEPS)$(ELFSIZE) 111.elf
	-@echo ' '

pre-build:
	-a9-linaro-pre-build-step
	-@echo ' '

post-build:
	-arm-none-eabi-objdump.exe -t 111.elf > 111_1.txt && arm-none-eabi-objdump.exe -d 111.elf > 111_2.txt && arm-none-eabi-nm -a -n 111.elf > 111_3.txt && arm-none-eabi-readelf -a 111.elf > 111_4.txt&&arm-none-eabi-objcopy -O binary -S zynqBM.elf zynqBM.bin
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents
.SECONDARY: main-build pre-build post-build

-include ../makefile.targets
