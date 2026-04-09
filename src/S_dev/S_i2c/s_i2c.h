/*
 * s_i2c.h
 *
 *  Created on: Apr 9, 2026
 *      Author: HTSANG
 */

#ifndef S_DEV_S_I2C_S_I2C_H_
#define S_DEV_S_I2C_S_I2C_H_

#include "hal_data.h"

void i2c_init(void);

fsp_err_t i2c_write_reg(uint8_t reg, uint8_t data);

fsp_err_t i2c_read_reg(uint8_t reg, uint8_t *data, uint32_t len);

void i2c_callback(i2c_master_callback_args_t *p_args);

#endif /* S_DEV_S_I2C_S_I2C_H_ */
