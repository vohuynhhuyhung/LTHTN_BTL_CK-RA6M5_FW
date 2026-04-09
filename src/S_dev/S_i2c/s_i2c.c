/*
 * s_i2c.c
 *
 *  Created on: Apr 9, 2026
 *      Author: HTSANG
 */


#include "s_i2c.h"
#include "r_iic_master.h"

volatile bool i2c_done = false;
volatile bool i2c_error = false;

void i2c_init(void)
{
    fsp_err_t err;

    err = R_IIC_MASTER_Open(g_i2c_master0.p_ctrl, g_i2c_master0.p_cfg);
    if (FSP_SUCCESS != err)
    {
        while(1);
    }

    R_IIC_MASTER_SlaveAddressSet(g_i2c_master0.p_ctrl, 0x32, I2C_MASTER_ADDR_MODE_7BIT);

    R_IIC_MASTER_CallbackSet(g_i2c_master0.p_ctrl, i2c_callback, NULL, NULL);
}

fsp_err_t i2c_write_reg(uint8_t reg, uint8_t data)
{
    uint8_t buf[2];

    buf[0] = reg;
    buf[1] = data;

    i2c_done = false;
    i2c_error = false;

    fsp_err_t err = R_IIC_MASTER_Write(g_i2c_master0.p_ctrl, buf, 2, false);
    if (err != FSP_SUCCESS)
        return err;

    while (!i2c_done && !i2c_error);

    if (i2c_error)
        return FSP_ERR_ABORTED;

    return FSP_SUCCESS;
}

fsp_err_t i2c_read_reg(uint8_t reg, uint8_t *data, uint32_t len)
{
    fsp_err_t err;

    i2c_done = false;
    i2c_error = false;

    err = R_IIC_MASTER_Write(g_i2c_master0.p_ctrl, &reg, 1, true);
    if (err != FSP_SUCCESS)
        return err;

    while (!i2c_done && !i2c_error);

    if (i2c_error)
        return FSP_ERR_ABORTED;

    i2c_done = false;

    err = R_IIC_MASTER_Read(g_i2c_master0.p_ctrl, data, len, false);
    if (err != FSP_SUCCESS)
        return err;

    while (!i2c_done && !i2c_error);

    if (i2c_error)
        return FSP_ERR_ABORTED;

    return FSP_SUCCESS;
}

void i2c_callback(i2c_master_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case I2C_MASTER_EVENT_TX_COMPLETE:
        case I2C_MASTER_EVENT_RX_COMPLETE:
            i2c_done = true;
            break;

        case I2C_MASTER_EVENT_ABORTED:
            i2c_error = true;
            break;

        default:
            break;
    }
}


