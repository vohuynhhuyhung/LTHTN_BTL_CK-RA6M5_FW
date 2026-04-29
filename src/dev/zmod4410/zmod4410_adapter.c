/*
 * zmod4410_adapter.c
 *
 * Tự implement init/measurement sequence qua I2C driver tự viết (s_i2c.c).
 * Dùng FSP algorithm library (calc_iaq_2nd_gen) để tính kết quả IAQ.
 * Không dùng rm_comms_i2c / r_iic_master FSP stack.
 */

#include "zmod4410_adapter.h"
#include "../i2c/i2c.h"
#include "../uart/uart.h"
#include "../iaq_packet/iaq_packet.h"
#include "zmod4xxx_types.h"
#include "iaq_2nd_gen/iaq_2nd_gen.h"
#include "hal_data.h"
#include "bsp_pin_cfg.h"
#include <stdio.h>
#include <string.h>

extern zmod4xxx_conf g_zmod4410_iaq_2nd_gen_sensor_type[];

#define ZMOD4410_I2C_ADDR         0x32
#define INIT                      0
#define MEASUREMENT               1
#define ZMOD4410_PID              0x2310
#define ZMOD4410_ADC_DATA_LEN     32
#define ZMOD4410_PROD_DATA_LEN    7
#define ZMOD4410_IAQ2_SAMPLE_TIME 3000U

#define REG_PID         (0x00U)
#define REG_CONF        (0x20U)
#define REG_PROD_DATA   (0x26U)
#define REG_CMD         (0x93U)
#define REG_STATUS      (0x94U)

#define STATUS_SEQUENCER_RUNNING    (0x80U)

void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t *p_args)
{
    (void)p_args;
}

static void zmod_write_burst(uint8_t reg, uint8_t *buf, uint8_t len)
{
    i2c0_write_mult_reg(ZMOD4410_I2C_ADDR, reg, buf, len);
}

static int8_t zmod_read_sensor_info(zmod4xxx_dev_t *dev)
{
    uint8_t buf[2];

    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, REG_PID, buf, 2);
    dev->pid = (uint16_t)((buf[0] << 8) | buf[1]);

    if (dev->pid != ZMOD4410_PID)
        return ERROR_SENSOR_UNSUPPORTED;

    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, REG_CONF, dev->config, 6);
    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, REG_PROD_DATA, dev->prod_data, ZMOD4410_PROD_DATA_LEN);

    return ZMOD4XXX_OK;
}

static void zmod_calc_hsp(zmod4xxx_conf *conf, uint8_t *config, uint8_t *hsp_out)
{
    uint8_t i = 0;
    while (i < conf->h.len)
    {
        int16_t hsp_temp = (int16_t)(((uint16_t)conf->h.data_buf[i] << 8) + conf->h.data_buf[i + 1]);
        float hspf = (-((float)config[2] * 256.0f + config[3]) *
                      ((config[4] + 640.0f) * (float)(config[5] + hsp_temp) - 512000.0f)) /
                     12288000.0f;
        hsp_out[i]     = (uint8_t)((uint16_t)hspf >> 8);
        hsp_out[i + 1] = (uint8_t)((uint16_t)hspf & 0x00FF);
        i = (uint8_t)(i + 2);
    }
}

static int8_t zmod_prepare_sensor(zmod4xxx_dev_t *dev)
{
    zmod4xxx_conf *conf = dev->meas_conf;
    uint8_t hsp[16] = {0};

    zmod_calc_hsp(conf, dev->config, hsp);

    zmod_write_burst(conf->h.addr, hsp, conf->h.len);
    zmod_write_burst(conf->d.addr, conf->d.data_buf, conf->d.len);
    zmod_write_burst(conf->m.addr, conf->m.data_buf, conf->m.len);
    zmod_write_burst(conf->s.addr, conf->s.data_buf, conf->s.len);

    return ZMOD4XXX_OK;
}

static int8_t zmod_start_measurement(zmod4xxx_dev_t *dev)
{
    i2c0_write_reg(ZMOD4410_I2C_ADDR, REG_CMD, dev->meas_conf->start);
    return ZMOD4XXX_OK;
}

static int8_t zmod_read_adc(zmod4xxx_dev_t *dev, uint8_t *adc_buf)
{
    zmod4xxx_conf *conf = dev->meas_conf;
    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, conf->r.addr, adc_buf, conf->r.len);
    return ZMOD4XXX_OK;
}

void zmod4410_read_iaq(void)
{
    char    uart_str[80];
    int     slen;
    int8_t  ret;

    rm_zmod4xxx_lib_extended_cfg_t *p_ext =
        (rm_zmod4xxx_lib_extended_cfg_t *)g_zmod4xxx_sensor0_cfg.p_extend;

    zmod4xxx_dev_t        *p_dev     = (zmod4xxx_dev_t *)p_ext->p_device;
    iaq_2nd_gen_handle_t  *p_handle  = (iaq_2nd_gen_handle_t *)p_ext->p_handle;
    iaq_2nd_gen_results_t *p_results = (iaq_2nd_gen_results_t *)p_ext->p_results;

    p_dev->i2c_addr  = ZMOD4410_I2C_ADDR;
    p_dev->init_conf = &g_zmod4410_iaq_2nd_gen_sensor_type[INIT];
    p_dev->meas_conf = &g_zmod4410_iaq_2nd_gen_sensor_type[MEASUREMENT];
    p_dev->prod_data = (uint8_t *)p_ext->p_product_data;

    R_BSP_PinAccessEnable();
    R_BSP_PinWrite(ZMOD4410_RESET, BSP_IO_LEVEL_HIGH);
    R_BSP_PinAccessDisable();
    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);

    ret = zmod_read_sensor_info(p_dev);
    if (ret != ZMOD4XXX_OK)
    {
        slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Sensor init failed: %d\r\n", (int)ret);
        uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
        return;
    }

    /* Clear POR event flag */
    i2c0_read_reg(ZMOD4410_I2C_ADDR, 0xB7);

    /* Init sequence */
    uint8_t hsp_init[4] = {0};
    zmod_calc_hsp(p_dev->init_conf, p_dev->config, hsp_init);
    zmod_write_burst(p_dev->init_conf->h.addr, hsp_init,                     p_dev->init_conf->h.len);
    zmod_write_burst(p_dev->init_conf->d.addr, p_dev->init_conf->d.data_buf, p_dev->init_conf->d.len);
    zmod_write_burst(p_dev->init_conf->m.addr, p_dev->init_conf->m.data_buf, p_dev->init_conf->m.len);
    zmod_write_burst(p_dev->init_conf->s.addr, p_dev->init_conf->s.data_buf, p_dev->init_conf->s.len);
    i2c0_write_reg(ZMOD4410_I2C_ADDR, REG_CMD, p_dev->init_conf->start);

    uint8_t  status;
    uint32_t timeout = 200;
    do {
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
        status = i2c0_read_reg(ZMOD4410_I2C_ADDR, REG_STATUS);
        timeout--;
    } while ((status & STATUS_SEQUENCER_RUNNING) && timeout);

    if (timeout == 0)
    {
        slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Init timeout!\r\n");
        uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
        return;
    }

    uint8_t r_buf[4];
    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, p_dev->init_conf->r.addr, r_buf, p_dev->init_conf->r.len);
    p_dev->mox_lr = (uint16_t)((r_buf[0] << 8) | r_buf[1]);
    p_dev->mox_er = (uint16_t)((r_buf[2] << 8) | r_buf[3]);

    ret = zmod_prepare_sensor(p_dev);
    if (ret != ZMOD4XXX_OK)
    {
        slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Prepare failed: %d\r\n", (int)ret);
        uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
        return;
    }

    ret = init_iaq_2nd_gen(p_handle);
    if (ret != IAQ_2ND_GEN_OK)
    {
        slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Algo init failed: %d\r\n", (int)ret);
        uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
        return;
    }

    slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Sensor ready.\r\n");
    uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);

    uint8_t adc_buf[ZMOD4410_ADC_DATA_LEN];

    iaq_2nd_gen_inputs_t algo_input;
    algo_input.adc_result        = adc_buf;
    algo_input.adc_rmox3_4510    = NULL;
    algo_input.humidity_pct      = 50.0f;
    algo_input.temperature_degc  = 20.0f;

    while (1)
    {
        ret = zmod_start_measurement(p_dev);
        if (ret != ZMOD4XXX_OK)
        {
            slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Start failed: %d\r\n", (int)ret);
            uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
            break;
        }

        R_BSP_SoftwareDelay(ZMOD4410_IAQ2_SAMPLE_TIME, BSP_DELAY_UNITS_MILLISECONDS);

        timeout = 50;
        do {
            R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
            status = i2c0_read_reg(ZMOD4410_I2C_ADDR, REG_STATUS);
            timeout--;
        } while ((status & STATUS_SEQUENCER_RUNNING) && timeout);

        if (timeout == 0)
        {
            slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Meas timeout!\r\n");
            uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
            continue;
        }

        ret = zmod_read_adc(p_dev, adc_buf);
        if (ret != ZMOD4XXX_OK)
        {
            slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Read ADC failed: %d\r\n", (int)ret);
            uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
            continue;
        }

        ret = calc_iaq_2nd_gen(p_handle, p_dev, NULL, &algo_input, p_results);
        if (ret == IAQ_2ND_GEN_STABILIZATION)
        {
            slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Warming up...\r\n");
            uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
            continue;
        }
        else if (ret != IAQ_2ND_GEN_OK)
        {
            slen = snprintf(uart_str, sizeof(uart_str), "[ZMOD] Algo error: %d\r\n", (int)ret);
            uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);
            continue;
        }

        int iaq_i  = (int)(p_results->iaq  * 10.0f);
        int tvoc_i = (int)(p_results->tvoc * 1000.0f);
        int eco2_i = (int)(p_results->eco2 * 10.0f);
        int etoh_i = (int)(p_results->etoh * 1000.0f);
        slen = snprintf(uart_str, sizeof(uart_str),
                        "[ZMOD] IAQ=%d.%d TVOC=%d.%03d eCO2=%d.%d EtOH=%d.%03d\r\n",
                        iaq_i / 10,  iaq_i % 10,
                        tvoc_i / 1000, tvoc_i % 1000,
                        eco2_i / 10,  eco2_i % 10,
                        etoh_i / 1000, etoh_i % 1000);
        uart_send_buf((uint8_t *)uart_str, (uint32_t)slen);

        iaq_data_t pkt = {
            .iaq  = p_results->iaq,
            .tvoc = p_results->tvoc,
            .eco2 = p_results->eco2,
            .etoh = p_results->etoh,
        };
        iaq_packet_send(&pkt);
    }
}
