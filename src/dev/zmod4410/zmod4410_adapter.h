/*
 * zmod4410_adapter.h
 * Adapter layer: nối ZMOD4410 FSP SDK với I2C driver tự viết (s_i2c.c)
 */

#ifndef S_DEV_S_ZMOD4410_ZMOD4410_ADAPTER_H_
#define S_DEV_S_ZMOD4410_ZMOD4410_ADAPTER_H_

#include "stdint.h"
#include "rm_zmod4xxx.h"

/* ZMOD4410_I2C_ADDR = 0x32 — defined in zmod4410_config_iaq2.h */

/* Khai báo instance được generate bởi FSP (dùng để gọi SDK) */
extern rm_zmod4xxx_instance_t const g_zmod4xxx_sensor0;

/* Callback được gọi khi SDK hoàn thành một thao tác I2C */
void zmod4410_comms_callback(rm_zmod4xxx_callback_args_t *p_args);

/* Hàm đọc dữ liệu IAQ 2nd Gen và in ra UART */
void zmod4410_read_iaq(void);

#endif /* S_DEV_S_ZMOD4410_ZMOD4410_ADAPTER_H_ */
