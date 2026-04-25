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
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra_gen" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/src/rm_zmod4xxx/iaq_2nd_gen" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/src/rm_zmod4xxx" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/src/S_dev/S_zmod4410" -I"." -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra_cfg/fsp_cfg/bsp" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra_cfg/fsp_cfg" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/src" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/inc" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/inc/api" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/inc/instances" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

