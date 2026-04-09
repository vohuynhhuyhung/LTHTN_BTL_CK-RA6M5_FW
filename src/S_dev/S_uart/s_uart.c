/*
 * s_uart.c
 *
 *  Created on: Apr 10, 2026
 *      Author: HTSANG
 */


#include "s_uart.h"
#include "r_sci_uart.h"
#include <stdbool.h>

/* ================= GLOBAL FLAG ================= */

volatile bool uart_tx_done = false;
volatile bool uart_rx_done = false;
volatile bool uart_error   = false;

/* ================= INIT ================= */

void uart_init(void)
{
    fsp_err_t err;

    err = R_SCI_UART_Open(&g_uart3_ctrl, &g_uart3_cfg);
    if (FSP_SUCCESS != err)
    {
        while (1);
    }

    err = R_SCI_UART_CallbackSet(&g_uart3_ctrl,
                                uart_callback,
                                NULL,
                                NULL);
    if (FSP_SUCCESS != err)
    {
        while (1);
    }
}

/* ================= SEND ================= */

fsp_err_t uart_send(uint8_t data)
{
    fsp_err_t err;

    uart_tx_done = false;
    uart_error   = false;

    err = R_SCI_UART_Write(&g_uart3_ctrl, &data, 1);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Wait TX done */
    uint32_t timeout = 1000000;

    while (!uart_tx_done && !uart_error && timeout--)
        ;

    if (timeout == 0)
    {
        return FSP_ERR_TIMEOUT;
    }

    if (uart_error)
    {
        return FSP_ERR_ABORTED;
    }

    return FSP_SUCCESS;
}

/* ================= RECEIVE ================= */

fsp_err_t uart_receive(uint8_t *data)
{
    fsp_err_t err;

    uart_rx_done = false;
    uart_error   = false;

    err = R_SCI_UART_Read(&g_uart3_ctrl, data, 1);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Wait RX done */
    uint32_t timeout = 1000000;

    while (!uart_rx_done && !uart_error && timeout--)
        ;

    if (timeout == 0)
    {
        return FSP_ERR_TIMEOUT;
    }

    if (uart_error)
    {
        return FSP_ERR_ABORTED;
    }

    return FSP_SUCCESS;
}

fsp_err_t uart_send_buf(uint8_t *buf, uint32_t len)
{
    fsp_err_t err;

    if ((buf == NULL) || (len == 0))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    uart_tx_done = false;
    uart_error   = false;

    err = R_SCI_UART_Write(&g_uart3_ctrl, buf, len);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Wait TX done */
    uint32_t timeout = 1000000;

    while (!uart_tx_done && !uart_error && timeout--)
        ;

    if (timeout == 0)
    {
        return FSP_ERR_TIMEOUT;
    }

    if (uart_error)
    {
        return FSP_ERR_ABORTED;
    }

    return FSP_SUCCESS;
}

/* ================= CALLBACK ================= */

void uart_callback(uart_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
            uart_tx_done = true;
            break;

        case UART_EVENT_RX_COMPLETE:
            uart_rx_done = true;
            break;

        case UART_EVENT_ERR_OVERFLOW:
        case UART_EVENT_ERR_FRAMING:
        case UART_EVENT_ERR_PARITY:
            uart_error = true;
            break;

        default:
            break;
    }
}
