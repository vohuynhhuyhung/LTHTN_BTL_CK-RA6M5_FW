/*
 * tflm_wrapper.h
 *
 *  Created on: May 1, 2026
 *      Author: PV
 */


#ifdef __cplusplus
extern "C" {
#endif

// Khai báo các hàm giao tiếp giữa C và C++
// Sử dụng các kiểu dữ liệu cơ bản của C (như mảng 1 chiều, con trỏ)

void ai_model_init(void);
void ai_model_predict(float* input_48, float* output_4);

#ifdef __cplusplus
}
#endif
