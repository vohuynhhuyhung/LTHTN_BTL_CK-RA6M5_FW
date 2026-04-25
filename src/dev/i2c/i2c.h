/*
 * s_i2c.h
 *
 *  Created on: Apr 9, 2026
 *      Author: HTSANG
 */

#ifndef S_DEV_S_I2C_S_I2C_H_
#define S_DEV_S_I2C_S_I2C_H_

#include "hal_data.h"

void i2c0_register_init(void);

void i2c0_write_reg(uint8_t slave_addr, uint8_t reg, uint8_t data);

void i2c0_write_mult_reg(uint8_t slave_addr, uint8_t start_reg, uint8_t *p_data, uint8_t len);

uint8_t i2c0_read_reg(uint8_t slave_addr, uint8_t reg);

void i2c0_read_mult_reg(uint8_t slave_addr, uint8_t start_reg, uint8_t *p_data, uint8_t len);



void i2c1_register_init(void);

void i2c1_write_reg(uint8_t slave_addr, uint8_t reg, uint8_t data);

uint8_t i2c1_read_reg(uint8_t slave_addr, uint8_t reg);

void i2c1_read_mult_reg(uint8_t slave_addr, uint8_t start_reg, uint8_t *p_data, uint8_t len);

#endif
