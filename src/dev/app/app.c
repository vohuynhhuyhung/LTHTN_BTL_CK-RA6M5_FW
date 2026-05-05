/*
 * app.c
 *
 * Application layer – tái cấu trúc zmod4410_read_iaq() thành các task
 * độc lập để gọi trong vòng lặp while(1) của main().
 *
 *  Luồng:
 *      my_prj_init()          ← gọi 1 lần
 *      while (1) {
 *          read_iaq_task()    ← đo sensor, cập nhật current
 *          ML_handle_task()   ← inference, cập nhật predict
 *          Uart_pkt_update()  ← đóng gói & gửi UART
 *      }
 */

#include "app.h"

/* ── Driver & middleware ────────────────────────────────── */
#include "dev/zmod4410/zmod4410_adapter.h"
#include "../i2c/i2c.h"
#include "../uart/uart.h"
#include "../iaq_packet/iaq_packet.h"
#include "zmod4xxx_types.h"
#include "iaq_2nd_gen/iaq_2nd_gen.h"
#include "inference.h"
#include "hal_data.h"
#include "bsp_pin_cfg.h"


#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════
 * Hằng số nội bộ
 * ═══════════════════════════════════════════════════════════ */
#define ZMOD4410_I2C_ADDR           0x32U
#define ZMOD4410_PID                0x2310U
#define ZMOD4410_ADC_DATA_LEN       32U
#define ZMOD4410_PROD_DATA_LEN      7U
#define ZMOD4410_IAQ2_SAMPLE_TIME   3000U   /* ms */

#define REG_PID                     0x00U
#define REG_CONF                    0x20U
#define REG_PROD_DATA               0x26U
#define REG_CMD                     0x93U
#define REG_STATUS                  0x94U
#define STATUS_SEQUENCER_RUNNING    0x80U

#define INIT                        0
#define MEASUREMENT                 1

/* ═══════════════════════════════════════════════════════════
 * Biến extern
 * ═══════════════════════════════════════════════════════════ */
extern zmod4xxx_conf    g_zmod4410_iaq_2nd_gen_sensor_type[];
extern sensor_manager_t g_sensors[SENSOR_MAX];

/* ═══════════════════════════════════════════════════════════
 * Trạng thái nội bộ module
 * ═══════════════════════════════════════════════════════════ */
static AppState_t           s_state      = APP_STATE_UNINITIALIZED;

/* Con trỏ tới các cấu trúc do FSP generate */
static zmod4xxx_dev_t        *s_p_dev     = NULL;
static iaq_2nd_gen_handle_t  *s_p_handle  = NULL;
static iaq_2nd_gen_results_t *s_p_results = NULL;

/* Buffer ADC dùng chung giữa các task */
static uint8_t               s_adc_buf[ZMOD4410_ADC_DATA_LEN];

/* FIFO window cho TinyML */
static SensorWindow_t        s_win;

/* Giá trị đo hiện tại – chia sẻ giữa read_iaq_task → ML_handle_task */
static IaqReading_t          s_current_reading;

/* ═══════════════════════════════════════════════════════════
 * Hàm nội bộ (static) – mirror của các helper trong adapter cũ
 * ═══════════════════════════════════════════════════════════ */

///* ── Ghi burst I2C ──────────────────────────────────────── */
//static void prv_write_burst(uint8_t reg, uint8_t *buf, uint8_t len)
//{
//    i2c0_write_mult_reg(ZMOD4410_I2C_ADDR, reg, buf, len);
//}
//
///* ── Tính heater set-point ──────────────────────────────── */
//static void prv_calc_hsp(zmod4xxx_conf *conf, uint8_t *config, uint8_t *hsp_out)
//{
//    uint8_t i = 0;
//    while (i < conf->h.len)
//    {
//        int16_t hsp_temp = (int16_t)(((uint16_t)conf->h.data_buf[i] << 8)
//                                     + conf->h.data_buf[i + 1]);
//        float hspf = (-((float)config[2] * 256.0f + config[3]) *
//                      ((config[4] + 640.0f) * (float)(config[5] + hsp_temp)
//                       - 512000.0f)) / 12288000.0f;
//        hsp_out[i]     = (uint8_t)((uint16_t)hspf >> 8);
//        hsp_out[i + 1] = (uint8_t)((uint16_t)hspf & 0x00FFU);
//        i = (uint8_t)(i + 2U);
//    }
//}
//
///* ── Đọc PID + config + prod_data từ sensor ────────────── */
//static bool prv_read_sensor_info(void)
//{
//    uint8_t buf[2];
//
//    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, REG_PID, buf, 2);
//    s_p_dev->pid = (uint16_t)((buf[0] << 8) | buf[1]);
//
//    if (s_p_dev->pid != ZMOD4410_PID)
//        return false;
//
//    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, REG_CONF,      s_p_dev->config,    6);
//    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR, REG_PROD_DATA,  s_p_dev->prod_data, ZMOD4410_PROD_DATA_LEN);
//
//    return true;
//}
//
///* ── Nạp measurement sequence lên sensor ───────────────── */
//static bool prv_prepare_measurement(void)
//{
//    zmod4xxx_conf *conf = s_p_dev->meas_conf;
//    uint8_t hsp[16]     = {0};
//
//    prv_calc_hsp(conf, s_p_dev->config, hsp);
//
//    prv_write_burst(conf->h.addr, hsp,                conf->h.len);
//    prv_write_burst(conf->d.addr, conf->d.data_buf,   conf->d.len);
//    prv_write_burst(conf->m.addr, conf->m.data_buf,   conf->m.len);
//    prv_write_burst(conf->s.addr, conf->s.data_buf,   conf->s.len);
//
//    return true;
//}
//
///* ── Poll REG_STATUS cho đến khi sequencer dừng ────────── */
static bool prv_wait_sequencer(uint32_t timeout_loops, uint32_t poll_ms)
{
    uint8_t status;
    do {
        R_BSP_SoftwareDelay(poll_ms, BSP_DELAY_UNITS_MILLISECONDS);
        status = i2c0_read_reg(ZMOD4410_I2C_ADDR, REG_STATUS);
        timeout_loops--;
    } while ((status & STATUS_SEQUENCER_RUNNING) && timeout_loops);

    return (timeout_loops > 0U);
}

/* ════════════════════════════════════════════════════════════
 * my_prj_init()
 * ════════════════════════════════════════════════════════════ */
bool my_prj_init(void)
{
    s_state = APP_STATE_ERROR; /* giả sử lỗi, sẽ cập nhật nếu OK */

    /* ── 1. Lấy con trỏ từ FSP extended config ──────────── */
    rm_zmod4xxx_lib_extended_cfg_t *p_ext =
        (rm_zmod4xxx_lib_extended_cfg_t *)g_zmod4xxx_sensor0_cfg.p_extend;

    s_p_dev     = (zmod4xxx_dev_t *)p_ext->p_device;
    s_p_handle  = (iaq_2nd_gen_handle_t *)p_ext->p_handle;
    s_p_results = (iaq_2nd_gen_results_t *)p_ext->p_results;

    s_p_dev->i2c_addr  = ZMOD4410_I2C_ADDR;
    s_p_dev->init_conf = &g_zmod4410_iaq_2nd_gen_sensor_type[INIT];
    s_p_dev->meas_conf = &g_zmod4410_iaq_2nd_gen_sensor_type[MEASUREMENT];
    s_p_dev->prod_data = (uint8_t *)p_ext->p_product_data;

    /* ── 2. Reset phần cứng ──────────────────────────────── */
    R_BSP_PinAccessEnable();
    R_BSP_PinWrite(ZMOD4410_RESET, BSP_IO_LEVEL_HIGH);
    R_BSP_PinAccessDisable();
    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);

    /* ── 3. Đọc sensor info (PID, config, prod_data) ─────── */
    if (zmod_read_sensor_info(s_p_dev))
        return false;

    /* ── 4. Clear POR event ──────────────────────────────── */
    i2c0_read_reg(ZMOD4410_I2C_ADDR, 0xB7U);

    /* ── 5. Init sequence ────────────────────────────────── */
    uint8_t hsp_init[4] = {0};
    zmod_calc_hsp(s_p_dev->init_conf, s_p_dev->config, hsp_init);

    zmod_write_burst(s_p_dev->init_conf->h.addr, hsp_init,                           s_p_dev->init_conf->h.len);
    zmod_write_burst(s_p_dev->init_conf->d.addr, s_p_dev->init_conf->d.data_buf,     s_p_dev->init_conf->d.len);
    zmod_write_burst(s_p_dev->init_conf->m.addr, s_p_dev->init_conf->m.data_buf,     s_p_dev->init_conf->m.len);
    zmod_write_burst(s_p_dev->init_conf->s.addr, s_p_dev->init_conf->s.data_buf,     s_p_dev->init_conf->s.len);
    i2c0_write_reg(ZMOD4410_I2C_ADDR, REG_CMD, s_p_dev->init_conf->start);

    if (!prv_wait_sequencer(200U, 50U))
        return false;

    /* ── 6. Đọc mox_lr / mox_er ─────────────────────────── */
    uint8_t r_buf[4];
    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR,
                       s_p_dev->init_conf->r.addr,
                       r_buf,
                       s_p_dev->init_conf->r.len);
    s_p_dev->mox_lr = (uint16_t)((r_buf[0] << 8) | r_buf[1]);
    s_p_dev->mox_er = (uint16_t)((r_buf[2] << 8) | r_buf[3]);

    /* ── 7. Chuẩn bị measurement sequence ───────────────── */
    if (zmod_prepare_sensor(s_p_dev))
        return false;

    /* ── 8. Khởi tạo thuật toán IAQ 2nd Gen ─────────────── */
    if (init_iaq_2nd_gen(s_p_handle) != IAQ_2ND_GEN_OK)
        return false;

    /* ── 9. Khởi tạo TinyML ──────────────────────────────── */
    if (!inference_init())
        return false;

    inference_window_init(&s_win);

    /* ── 10. Reset reading cache ─────────────────────────── */
    memset(&s_current_reading, 0, sizeof(s_current_reading));
    memset(s_adc_buf,          0, sizeof(s_adc_buf));

    s_state = APP_STATE_READY;
    return true;
}

/* ════════════════════════════════════════════════════════════
 * read_iaq_task()
 * ════════════════════════════════════════════════════════════ */
int cnt = 0;
bool read_iaq_task(IaqReading_t *p_out)
{
    if (s_state == APP_STATE_UNINITIALIZED || s_state == APP_STATE_ERROR)
        return false;

    /* ── 1. Cấu hình input thuật toán ───────────────────── */
    iaq_2nd_gen_inputs_t algo_input;
    algo_input.adc_result       = s_adc_buf;
    algo_input.adc_rmox3_4510   = NULL;
    algo_input.humidity_pct     = 50.0f;    /* TODO: đọc từ sensor độ ẩm thực */
    algo_input.temperature_degc = 20.0f;    /* TODO: đọc từ sensor nhiệt độ thực */

    /* ── 2. Bắt đầu chu kỳ đo ───────────────────────────── */
    i2c0_write_reg(ZMOD4410_I2C_ADDR, REG_CMD, s_p_dev->meas_conf->start);

    /* ── 3. Chờ sensor hoàn thành measurement ───────────── */
    R_BSP_SoftwareDelay(ZMOD4410_IAQ2_SAMPLE_TIME, BSP_DELAY_UNITS_MILLISECONDS);

    if (!prv_wait_sequencer(50U, 100U))
        return false;                       /* timeout */

    /* ── 4. Đọc ADC data ────────────────────────────────── */
    i2c0_read_mult_reg(ZMOD4410_I2C_ADDR,
                       s_p_dev->meas_conf->r.addr,
                       s_adc_buf,
                       s_p_dev->meas_conf->r.len);

    /* ── 5. Tính IAQ bằng FSP algorithm ─────────────────── */
    int8_t ret = calc_iaq_2nd_gen(s_p_handle,
                                   s_p_dev,
                                   NULL,
                                   &algo_input,
                                   s_p_results);

    bool stabilized = (ret == IAQ_2ND_GEN_OK);
    bool warm_up    = (ret == IAQ_2ND_GEN_STABILIZATION);

    if (!stabilized && !warm_up)
        return false;                       /* lỗi algorithm */

    /* ── 6. Cập nhật cache nội bộ ───────────────────────── */
    s_current_reading.iaq        = s_p_results->iaq;
    s_current_reading.tvoc       = s_p_results->tvoc;
    s_current_reading.eco2       = s_p_results->eco2;
    s_current_reading.etoh       = s_p_results->etoh;
    s_current_reading.valid      = true;
    s_current_reading.stabilized = stabilized;

    /* ── 7. Cập nhật g_sensors current ──────────────────── */
    g_sensors[SENSOR_ID_IAQ].data_current  = s_current_reading.iaq;
    g_sensors[SENSOR_ID_TVOC].data_current = s_current_reading.tvoc;
    g_sensors[SENSOR_ID_ECO2].data_current = s_current_reading.eco2;
    g_sensors[SENSOR_ID_ETOH].data_current = s_current_reading.etoh;

    s_state = stabilized ? APP_STATE_RUNNING : APP_STATE_WARMING_UP;

    /* ── 8. Trả kết quả ra ngoài nếu caller muốn ────────── */
    if (p_out != NULL)
        *p_out = s_current_reading;

    return true;
    cnt++;
}

/* ════════════════════════════════════════════════════════════
 * ML_handle_task()
 * ════════════════════════════════════════════════════════════ */
void ML_handle_task(void)
{
    if (!s_current_reading.valid)
        return;                             /* chưa có dữ liệu hợp lệ */

    /* ── 1. Clamp + push vào FIFO window ────────────────── */
    float raw[N_FEATURES] = {
        clamp(s_current_reading.iaq,  FEAT_MIN_0, FEAT_MAX_0),
        clamp(s_current_reading.tvoc, FEAT_MIN_1, FEAT_MAX_1),
        clamp(s_current_reading.eco2, FEAT_MIN_2, FEAT_MAX_2),
        clamp(s_current_reading.etoh, FEAT_MIN_3, FEAT_MAX_3)
    };
    inference_push(&s_win, raw);

    /* ── 2. Chạy inference (chỉ khi window đã đầy) ──────── */
    InferenceResult_t ml_result = inference_run(&s_win);

    /* ── 3. Cập nhật g_sensors predict ──────────────────── */
    if (ml_result.valid)
    {
        g_sensors[SENSOR_ID_IAQ].data_predict  = ml_result.value[0];
        g_sensors[SENSOR_ID_TVOC].data_predict = ml_result.value[1];
        g_sensors[SENSOR_ID_ECO2].data_predict = ml_result.value[2];
        g_sensors[SENSOR_ID_ETOH].data_predict = ml_result.value[3];
    }
    else
    {
        /* Window chưa đầy → fallback: predict = current */
        g_sensors[SENSOR_ID_IAQ].data_predict  = s_current_reading.iaq;
        g_sensors[SENSOR_ID_TVOC].data_predict = s_current_reading.tvoc;
        g_sensors[SENSOR_ID_ECO2].data_predict = s_current_reading.eco2;
        g_sensors[SENSOR_ID_ETOH].data_predict = s_current_reading.etoh;
    }
}

/* ════════════════════════════════════════════════════════════
 * Uart_pkt_update()
 * ════════════════════════════════════════════════════════════ */
void Uart_pkt_update(void)
{
    for (uint8_t i = 0U; i < (uint8_t)SENSOR_MAX; i++)
    {
        iaq_packet_send(&g_sensors[i]);
    }
}

/* ════════════════════════════════════════════════════════════
 * app_get_state()
 * ════════════════════════════════════════════════════════════ */
AppState_t app_get_state(void)
{
    return s_state;
}
