################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: delay-systick.axf

# Tool invocations
delay-systick.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\apes\docs\lcpxpresso-projects\ARM\LPC1114\tests\CMSISv2p00_LPC11xx\Debug" -Xlinker -Map="delay-systick.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "delay-systick_Debug.ld" -o "delay-systick.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) delay-systick.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "delay-systick.axf" ; arm-none-eabi-objcopy -O ihex "delay-systick.axf" "delay-systick.hex" ; # arm-none-eabi-objcopy -v -O binary "delay-systick.axf" "delay-systick.bin" ; # checksum -p LPC1114_302 -d "delay-systick.bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
