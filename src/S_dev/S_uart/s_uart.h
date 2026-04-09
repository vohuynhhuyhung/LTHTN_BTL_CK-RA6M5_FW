/*
 * s_uart.h
 *
 *  Created on: Apr 10, 2026
 *      Author: HTSANG
 */

#ifndef S_DEV_S_UART_S_UART_H_
#define S_DEV_S_UART_S_UART_H_

#include "hal_data.h"

void uart_init(void);

fsp_err_t uart_send(uint8_t data);

fsp_err_t uart_receive(uint8_t *data);

fsp_err_t uart_send_buf(uint8_t *buf, uint32_t len);

void uart_callback(uart_callback_args_t * p_args);

#endif /* S_DEV_S_UART_S_UART_H_ */
