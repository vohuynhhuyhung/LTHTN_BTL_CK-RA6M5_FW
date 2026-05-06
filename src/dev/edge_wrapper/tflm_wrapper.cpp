// File: src/tflm_wrapper.cpp
#include "tflm_wrapper.h"
#include <stdint.h>
#include "model.h"
#include "dev/uart/uart.h"
#include "stdio.h"
#include <string.h>


#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_op_resolver.h"

float    debug_input_scale = 0.0f;
int      debug_input_zp    = 0;
int8_t   debug_raw_int8_input[4] = {0};
int8_t   debug_raw_int8_output[4] = {0};
uint32_t debug_invoke_count = 0;  
TfLiteStatus debug_last_status = kTfLiteOk;

constexpr int kTensorArenaSize = 80 * 1024;
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((section(".bss"), aligned(16)));
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

void ai_model_init(void) {
	model = tflite::GetModel(g_model);

	    static tflite::MicroMutableOpResolver<8> resolver;

	    resolver.AddConv2D();
	    resolver.AddMaxPool2D();
	    resolver.AddReshape();
	    resolver.AddFullyConnected();

	    resolver.AddQuantize();
	    resolver.AddDequantize();
	    static tflite::MicroInterpreter static_interpreter(
	        model, resolver, tensor_arena, kTensorArenaSize);
	    interpreter = &static_interpreter;

	    interpreter->AllocateTensors();
	    input = interpreter->input(0);
	    output = interpreter->output(0);
}

void ai_model_predict(float* input_48, float* output_4) {
    for (int i = 0; i < 48; i++) {
        int32_t val = (int32_t)(input_48[i] / debug_input_scale + debug_input_zp);

        if (val > 127)  val = 127;
        if (val < -128) val = -128;

        input->data.int8[i] = (int8_t)val;

        if (i < 4) debug_raw_int8_input[i] = input->data.int8[i];
    }

    debug_last_status = interpreter->Invoke();

    if (debug_last_status == kTfLiteOk) {
        debug_invoke_count++;

        float output_scale = output->params.scale;
        int output_zero_point = output->params.zero_point;

        for (int i = 0; i < 4; i++) {
            debug_raw_int8_output[i] = output->data.int8[i];

            output_4[i] = (float)(output->data.int8[i] - output_zero_point) * output_scale;
        }
    }
}
