################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include system/subdir.mk
-include loramac/soft-se/subdir.mk
-include loramac/radio/sx1276/subdir.mk
-include loramac/mac/region/subdir.mk
-include loramac/mac/subdir.mk
-include example/stm32/STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/subdir.mk
-include example/stm32/STM32Cube_FW_L0_V1.11.2/Drivers/BSP/STM32L0xx_Nucleo/subdir.mk
-include example/stm32/NucleoL073/board/cmsis/arm-gcc/subdir.mk
-include example/stm32/NucleoL073/board/cmsis/subdir.mk
-include example/stm32/NucleoL073/board/subdir.mk
-include classA/subdir.mk
-include apps/common/LmHandler/packages/subdir.mk
-include apps/common/LmHandler/subdir.mk
-include apps/common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
classA.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
classA.list \


# All Target
all: classA.elf secondary-outputs

# Tool invocations
classA.elf: $(OBJS) $(USER_OBJS)
	arm-none-eabi-gcc -o "classA.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0plus --specs=nosys.specs -Wl,-Map="classA.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

classA.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "classA.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
