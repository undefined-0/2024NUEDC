################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
bsp/%.o: ../bsp/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ccstheia141/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"D:/A_Scholastic/33/workspace1/2024NUEDC" -I"D:/A_Scholastic/33/workspace1/2024NUEDC/Debug" -I"D:/A_Scholastic/33/ti/mspm0_sdk_2_04_00_06/source/third_party/CMSIS/Core/Include" -I"D:/A_Scholastic/33/ti/mspm0_sdk_2_04_00_06/source" -gdwarf-3 -MMD -MP -MF"bsp/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


