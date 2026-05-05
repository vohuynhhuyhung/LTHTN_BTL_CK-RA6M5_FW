/*
 * zmod4410_adapter.h
 * Adapter layer: nối ZMOD4410 FSP SDK với I2C driver tự viết (s_i2c.c)
 */

#ifndef S_DEV_S_ZMOD4410_ZMOD4410_ADAPTER_H_
#define S_DEV_S_ZMOD4410_ZMOD4410_ADAPTER_H_

#include "stdint.h"
#include "rm_zmod4xxx.h"
#include "zmod4xxx_types.h"

/* ZMOD4410_I2C_ADDR = 0x32 — defined in zmod4410_config_iaq2.h */

/* Khai báo instance được generate bởi FSP (dùng để gọi SDK) */
extern rm_zmod4xxx_instance_t const g_zmod4xxx_sensor0;

/* Callback được gọi khi SDK hoàn thành một thao tác I2C */
void zmod4410_comms_callback(rm_zmod4xxx_callback_args_t *p_args);
void zmod_write_burst(uint8_t reg, uint8_t *buf, uint8_t len);
void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t *p_args);
int8_t zmod_read_sensor_info(zmod4xxx_dev_t *dev);
void zmod_calc_hsp(zmod4xxx_conf *conf, uint8_t *config, uint8_t *hsp_out);
int8_t zmod_prepare_sensor(zmod4xxx_dev_t *dev);
int8_t zmod_start_measurement(zmod4xxx_dev_t *dev);
int8_t zmod_read_adc(zmod4xxx_dev_t *dev, uint8_t *adc_buf);

/* Hàm đọc dữ liệu IAQ 2nd Gen và in ra UART */
void zmod4410_read_iaq(void);
float clamp(float x, float min, float max);

#endif /* S_DEV_S_ZMOD4410_ZMOD4410_ADAPTER_H_ */
