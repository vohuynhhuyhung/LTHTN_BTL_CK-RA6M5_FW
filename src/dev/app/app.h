/*
 * app.h
 *
 * Application layer – tách riêng các task từ zmod4410_read_iaq().
 *
 * Luồng sử dụng trong main():
 *   my_prj_init();
 *   while (1) {
 *       read_iaq_task();
 *       ML_handle_task();
 *       Uart_pkt_update();
 *   }
 */

#ifndef APP_H_
#define APP_H_

#include <stdint.h>
#include <stdbool.h>

/* ═══════════════════════════════════════════════════════════
 * Trạng thái nội bộ của ứng dụng
 * ═══════════════════════════════════════════════════════════ */

/**
 * @brief Trạng thái khởi động / đo lường của sensor.
 */
typedef enum {
    APP_STATE_UNINITIALIZED = 0,  /**< Chưa gọi my_prj_init()          */
    APP_STATE_READY,              /**< Init thành công, sẵn sàng đo    */
    APP_STATE_WARMING_UP,         /**< Sensor đang warm-up (stabilize) */
    APP_STATE_RUNNING,            /**< Đo lường bình thường            */
    APP_STATE_ERROR               /**< Lỗi khởi tạo hoặc đo lường      */
} AppState_t;

/* ═══════════════════════════════════════════════════════════
 * Dữ liệu IAQ hiện tại (output của read_iaq_task)
 * ═══════════════════════════════════════════════════════════ */

/**
 * @brief Giá trị IAQ đo được trong một chu kỳ.
 *        Được cập nhật sau mỗi lần gọi read_iaq_task().
 */
typedef struct {
    float   iaq;        /**< IAQ index                    */
    float   tvoc;       /**< TVOC  [mg/m³]                */
    float   eco2;       /**< eCO2  [ppm]                  */
    float   etoh;       /**< EtOH  [ppm]                  */
    bool    valid;      /**< true nếu dữ liệu hợp lệ      */
    bool    stabilized; /**< true nếu sensor đã ổn định   */
} IaqReading_t;

/* ═══════════════════════════════════════════════════════════
 * API công khai
 * ═══════════════════════════════════════════════════════════ */

/**
 * @brief  Khởi tạo toàn bộ hệ thống: reset phần cứng, đọc sensor info,
 *         chạy init-sequence, đọc mox_lr/mox_er, chuẩn bị measurement
 *         sequence, khởi tạo thuật toán IAQ 2nd Gen và TinyML model.
 *
 * @return true  – khởi tạo thành công.
 *         false – thất bại (sensor không phản hồi, PID sai, …).
 *
 * @note   Gọi đúng một lần trước vòng lặp while(1).
 */
bool my_prj_init(void);

/**
 * @brief  Thực hiện một chu kỳ đo ZMOD4410:
 *           1. Bắt đầu measurement sequence.
 *           2. Chờ sensor hoàn thành (poll REG_STATUS).
 *           3. Đọc ADC data.
 *           4. Gọi calc_iaq_2nd_gen() để tính IAQ/TVOC/eCO2/EtOH.
 *           5. Cập nhật g_sensors[*].data_current.
 *
 * @param[out] p_out  Con trỏ tới IaqReading_t nhận kết quả đo.
 *                    Nếu NULL, hàm vẫn cập nhật g_sensors nhưng
 *                    không ghi ra ngoài.
 *
 * @return true  – đo thành công (kể cả khi đang warm-up).
 *         false – lỗi I2C hoặc timeout.
 *
 * @note   Hàm có blocking delay ~3 s (ZMOD4410_IAQ2_SAMPLE_TIME).
 *         Gọi định kỳ trong vòng lặp chính.
 */
bool read_iaq_task(IaqReading_t *p_out);

/**
 * @brief  Thực hiện TinyML inference:
 *           1. Clamp + push giá trị IAQ hiện tại vào FIFO window.
 *           2. Gọi inference_run() nếu window đã đầy.
 *           3. Cập nhật g_sensors[*].data_predict.
 *              Nếu window chưa đầy → fallback: predict = current.
 *
 * @note   Phải gọi SAU read_iaq_task() để đảm bảo dữ liệu mới nhất
 *         đã có trong g_sensors.
 *         Không có blocking delay; thường hoàn thành trong vài ms.
 */
void ML_handle_task(void);

/**
 * @brief  Đóng gói và gửi dữ liệu qua UART tới ESP32:
 *           – Duyệt toàn bộ g_sensors[SENSOR_MAX].
 *           – Gọi iaq_packet_send() cho từng sensor.
 *
 * @note   Phải gọi SAU ML_handle_task() để gói tin chứa cả
 *         data_current lẫn data_predict đã cập nhật.
 */
void Uart_pkt_update(void);

/**
 * @brief  Trả về trạng thái hiện tại của ứng dụng.
 *         Hữu ích để kiểm tra từ main() hoặc task giám sát.
 */
AppState_t app_get_state(void);

#endif /* APP_H_ */
