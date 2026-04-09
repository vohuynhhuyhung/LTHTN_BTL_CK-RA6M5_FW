################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/S_dev/S_i2c/s_i2c.c 

C_DEPS += \
./src/S_dev/S_i2c/s_i2c.d 

OBJS += \
./src/S_dev/S_i2c/s_i2c.o 

SREC += \
i2c_prj.srec 

MAP += \
i2c_prj.map 


# Each subdirectory must supply rules for building sources it contributes
src/S_dev/S_i2c/%.o: ../src/S_dev/S_i2c/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"D:/Renesas_Workspace/i2c_prj/ra_gen" -I"." -I"D:/Renesas_Workspace/i2c_prj/ra_cfg/fsp_cfg/bsp" -I"D:/Renesas_Workspace/i2c_prj/ra_cfg/fsp_cfg" -I"D:/Renesas_Workspace/i2c_prj/src" -I"D:/Renesas_Workspace/i2c_prj/ra/fsp/inc" -I"D:/Renesas_Workspace/i2c_prj/ra/fsp/inc/api" -I"D:/Renesas_Workspace/i2c_prj/ra/fsp/inc/instances" -I"D:/Renesas_Workspace/i2c_prj/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

