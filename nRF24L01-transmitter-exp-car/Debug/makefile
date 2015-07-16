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
all: nRF24L01-transmitter-exp-car.axf

# Tool invocations
nRF24L01-transmitter-exp-car.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\apes\docs\lcpxpresso-projects\ARM\LPC1114\tests\CMSISv2p00_LPC11xx\Debug" -Xlinker -Map="nRF24L01-transmitter-exp-car.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "nRF24L01-transmitter-exp-car_Debug.ld" -o "nRF24L01-transmitter-exp-car.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) nRF24L01-transmitter-exp-car.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "nRF24L01-transmitter-exp-car.axf" ; arm-none-eabi-objcopy -v -O ihex "nRF24L01-transmitter-exp-car.axf" "nRF24L01-transmitter-exp-car.hex" ; # arm-none-eabi-objcopy -v -O binary "nRF24L01-transmitter-exp-car.axf" "nRF24L01-transmitter-exp-car.bin" ; # checksum -p LPC1114_302 -d "nRF24L01-transmitter-exp-car.bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
