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
all: adc-burst-no-interrupt-multichannel.axf

# Tool invocations
adc-burst-no-interrupt-multichannel.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\apes\docs\lcpxpresso-projects\ARM\LPC1114\tests\CMSISv2p00_LPC11xx\Debug" -Xlinker -Map="adc-burst-no-interrupt-multichannel.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "adc-burst-no-interrupt-multichannel_Debug.ld" -o "adc-burst-no-interrupt-multichannel.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) adc-burst-no-interrupt-multichannel.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "adc-burst-no-interrupt-multichannel.axf" ; arm-none-eabi-objcopy -v -O ihex "adc-burst-no-interrupt-multichannel.axf" "adc-burst-no-interrupt-multichannel.hex" ; # arm-none-eabi-objcopy -v -O binary "adc-burst-no-interrupt-multichannel.axf" "adc-burst-no-interrupt-multichannel.bin" ; # checksum -p LPC1114_302 -d "adc-burst-no-interrupt-multichannel.bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
