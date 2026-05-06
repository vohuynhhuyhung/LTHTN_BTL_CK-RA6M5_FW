/*
 * tflm_wrapper.h
 *
 *  Created on: May 1, 2026
 *      Author: PV
 */


#ifdef __cplusplus
extern "C" {
#endif

void ai_model_init(void);
void ai_model_predict(float* input_48, float* output_4);

#ifdef __cplusplus
}
#endif
