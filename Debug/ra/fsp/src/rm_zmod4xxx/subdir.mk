################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/rm_zmod4xxx/rm_zmod4xxx.c \
../ra/fsp/src/rm_zmod4xxx/rm_zmod4xxx_ra_driver.c 

C_DEPS += \
./ra/fsp/src/rm_zmod4xxx/rm_zmod4xxx.d \
./ra/fsp/src/rm_zmod4xxx/rm_zmod4xxx_ra_driver.d 

OBJS += \
./ra/fsp/src/rm_zmod4xxx/rm_zmod4xxx.o \
./ra/fsp/src/rm_zmod4xxx/rm_zmod4xxx_ra_driver.o 

SREC += \
i2c_prj.srec 

MAP += \
i2c_prj.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/rm_zmod4xxx/%.o: ../ra/fsp/src/rm_zmod4xxx/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra_gen" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/src/dev/iaq_packet" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/src/rm_zmod4xxx/iaq_2nd_gen" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/src/rm_zmod4xxx" -I"." -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra_cfg/fsp_cfg/bsp" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra_cfg/fsp_cfg" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/src" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/inc" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/inc/api" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/fsp/inc/instances" -I"D:/1_WorkSpace/FW/Renesas/CK_RA6M5---I2C/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

