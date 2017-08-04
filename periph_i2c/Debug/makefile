################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include example/src/subdir.mk
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
all: periph_i2c.axf

# Tool invocations
periph_i2c.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\Joey\Documents\LPCXpresso_8.2.2_650\MATE_Workspace\wstest\lpc_chip_175x_6x\Debug" -L"C:\Users\Joey\Documents\LPCXpresso_8.2.2_650\MATE_Workspace\wstest\lpc_board_nxp_lpcxpresso_1769\Debug" -Xlinker -Map="periph_i2c.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "periph_i2c_Debug.ld" -o "periph_i2c.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) periph_i2c.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "periph_i2c.axf"; # arm-none-eabi-objcopy -v -O binary "periph_i2c.axf" "periph_i2c.bin" ; # checksum -p LPC1769 -d "periph_i2c.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
