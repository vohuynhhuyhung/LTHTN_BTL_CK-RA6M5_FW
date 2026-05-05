// File: src/tflm_wrapper.cpp
#include "tflm_wrapper.h"
#include <stdint.h>
#include "model.h" // Nhúng file model 224KB của bạn vào ĐÂY
#include "dev/uart/uart.h"
#include "stdio.h"
#include <string.h>


// Các thư viện C++ của TensorFlow Lite

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_op_resolver.h"
// --- CÁC BIẾN DEBUG ĐỂ THEO DÕI TRÊN LIVE EXPRESSIONS ---
float    debug_input_scale = 0.0f;
int      debug_input_zp    = 0;
int8_t   debug_raw_int8_input[4] = {0};  // Theo dõi 4 giá trị int8 đầu tiên sau khi Quantize
int8_t   debug_raw_int8_output[4] = {0}; // Theo dõi 4 giá trị int8 đầu ra trước khi Dequantize
uint32_t debug_invoke_count = 0;         // Đếm số lần hàm Invoke chạy thành công
TfLiteStatus debug_last_status = kTfLiteOk; // Kiểm tra lỗi thực thi gần nhất



// Biến toàn cục (chỉ hiển thị trong file .cpp này)
constexpr int kTensorArenaSize = 80 * 1024;
//uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((section(".bss"), aligned(16)));
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Triển khai hàm init đã khai báo ở file .h
void ai_model_init(void) {
	model = tflite::GetModel(g_model);

	    // Tăng số lượng lên 6 để dự phòng cho Reshape hoặc các Op ẩn khác
	    static tflite::MicroMutableOpResolver<8> resolver;

	    // Sử dụng tên gọi đầy đủ để Linker dễ nhận diện
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



//void ai_model_predict(float* input_48, float* output_4) {
//    float input_scale = input->params.scale;
//    int input_zero_point = input->params.zero_point;
//
//    // Log kiểm tra thông số Quantization một lần
//     printf("Scale: %f, ZP: %d\n", input_scale, input_zero_point);
//
//    for (int i = 0; i < 48; i++) {
//        input->data.int8[i] = (int8_t)(input_48[i] / input_scale + input_zero_point);
//    }
//
//    TfLiteStatus invoke_status = interpreter->Invoke();
//
//    if (invoke_status != kTfLiteOk) {
//        // Nếu in ra dòng này, nghĩa là mô hình bị lỗi thực thi
//         printf("Invoke failed!\n");
//        return;
//    }
//
//    float output_scale = output->params.scale;
//    int output_zero_point = output->params.zero_point;
//
//    for (int i = 0; i < 4; i++) {
//        // In giá trị int8 thô của output trước khi Dequantize
//         printf("Raw Out[%d]: %d\n", i, output->data.int8[i]);
//        output_4[i] = (output->data.int8[i] - output_zero_point) * output_scale;
//    }
//}
void ai_model_predict(float* input_48, float* output_4) {
    // 1. Quantize: Chuyển đổi float sang int8
    for (int i = 0; i < 48; i++) {
        int32_t val = (int32_t)(input_48[i] / debug_input_scale + debug_input_zp);

        // Clipping để đảm bảo không vượt quá giới hạn int8 (-128 đến 127)
        if (val > 127)  val = 127;
        if (val < -128) val = -128;

        input->data.int8[i] = (int8_t)val;

        // Lưu 4 mẫu đầu tiên để xem trên Expressions
        if (i < 4) debug_raw_int8_input[i] = input->data.int8[i];
    }

    // 2. Thực thi mô hình
    debug_last_status = interpreter->Invoke();

    if (debug_last_status == kTfLiteOk) {
        debug_invoke_count++;

        // 3. Dequantize: Lấy kết quả ra
        float output_scale = output->params.scale;
        int output_zero_point = output->params.zero_point;

        for (int i = 0; i < 4; i++) {
            // Lưu giá trị int8 thô của output để debug
            debug_raw_int8_output[i] = output->data.int8[i];

            // Tính toán giá trị float cuối cùng
            output_4[i] = (float)(output->data.int8[i] - output_zero_point) * output_scale;
        }
    }
}
