/**
 * inference.cpp
 * TFLite Micro inference engine cho Renesas CK-RA6M5
 * Model: LSTM/GRU 12×4 → 4 (time-series prediction)
 */

#include "inference.h"




#include "modelhihi.h"   /* C array generate từ Colab Cell 9 */

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <cstring>
#include <cstdio>

/* ── Bộ nhớ tĩnh cho TFLite Micro ──────────────────────── */
static uint8_t tensor_arena[TENSOR_ARENA_SIZE] __attribute__((aligned(16)));

/* ── Đối tượng TFLite Micro (tĩnh, không dùng heap) ────── */
static const tflite::Model              *s_model      = nullptr;
static tflite::MicroInterpreter         *s_interpreter = nullptr;
static TfLiteTensor                     *s_input       = nullptr;
static TfLiteTensor                     *s_output      = nullptr;

/* Op resolver — thêm đúng các op mà model dùng            */
static tflite::MicroMutableOpResolver<12> s_resolver;

/* ── Bảng scaler (axis=1, mỗi feature 1 cặp min/max) ───── */
static const float kFeatMin[N_FEATURES] = {
    FEAT_MIN_0, FEAT_MIN_1, FEAT_MIN_2, FEAT_MIN_3
};
static const float kFeatMax[N_FEATURES] = {
    FEAT_MAX_0, FEAT_MAX_1, FEAT_MAX_2, FEAT_MAX_3
};
static const float kTgtMin[N_TARGETS] = {
    TGT_MIN_0, TGT_MIN_1, TGT_MIN_2, TGT_MIN_3
};
static const float kTgtMax[N_TARGETS] = {
    TGT_MAX_0, TGT_MAX_1, TGT_MAX_2, TGT_MAX_3
};

/* ═══════════════════════════════════════════════════════════
 * Helper — normalize / denormalize (MinMaxScaler axis=1)
 * ═══════════════════════════════════════════════════════════ */
static inline float normalize(float v, float mn, float mx)
{
    float range = mx - mn;
    if (range < 1e-9f) return 0.0f;
    return (v - mn) / range;
}

static inline float denormalize(float v, float mn, float mx)
{
    return v * (mx - mn) + mn;
}

/* ═══════════════════════════════════════════════════════════
 * inference_init
 * ═══════════════════════════════════════════════════════════ */
namespace tflite {
    namespace ops {
        namespace micro {
            TfLiteRegistration* Register_UNIDIRECTIONAL_SEQUENCE_LSTM();
        }
    }
}
extern "C" bool inference_init(void)
{
    tflite::InitializeTarget();

    s_resolver.AddShape();
    s_resolver.AddStridedSlice();
    s_resolver.AddTranspose();
    s_resolver.AddUnpack();
    s_resolver.AddPack();
    s_resolver.AddFill();
    s_resolver.AddFullyConnected();
    s_resolver.AddAdd();
    s_resolver.AddSplit();
    s_resolver.AddLogistic();
    s_resolver.AddMul();
    s_resolver.AddTanh();

    s_resolver.AddSub();
    s_resolver.AddMinimum();
    s_resolver.AddMaximum();
    s_resolver.AddQuantize();
    s_resolver.AddDequantize();

    s_model = tflite::GetModel(sensor_model);
    if (s_model->version() != TFLITE_SCHEMA_VERSION) return false;

    static tflite::MicroInterpreter static_interpreter(
        s_model, s_resolver, tensor_arena, TENSOR_ARENA_SIZE);
    s_interpreter = &static_interpreter;

    if (s_interpreter->AllocateTensors() != kTfLiteOk) return false;

    s_input  = s_interpreter->input(0);
    s_output = s_interpreter->output(0);

    if (s_input->dims->data[1] != WINDOW_SIZE ||
        s_input->dims->data[2] != N_FEATURES) return false;

    return true;
}

extern "C" void inference_window_init(SensorWindow_t *win)
{
    memset(win, 0, sizeof(SensorWindow_t));
    win->head  = 0;
    win->count = 0;
}

extern "C" void inference_push(SensorWindow_t *win, const float raw[N_FEATURES])
{
    for (int f = 0; f < N_FEATURES; f++)
        win->data[win->head][f] = raw[f];

    win->head = (win->head + 1) % WINDOW_SIZE;
    if (win->count < WINDOW_SIZE) win->count++;
}

extern "C" InferenceResult_t inference_run(const SensorWindow_t *win)
{
    InferenceResult_t result;
    result.valid = false;

    if (win->count < WINDOW_SIZE) return result;

    int8_t *inp_ptr = s_input->data.int8;

    for (int t = 0; t < WINDOW_SIZE; t++) {
        int src = (win->head + t) % WINDOW_SIZE;
        for (int f = 0; f < N_FEATURES; f++) {
            float norm = normalize(win->data[src][f],
                                   kFeatMin[f], kFeatMax[f]);
            int q = (int)(norm / INPUT_SCALE) + INPUT_ZP;
            if (q >  127) q =  127;
            if (q < -128) q = -128;
            inp_ptr[t * N_FEATURES + f] = (int8_t)q;
        }
    }

    if (s_interpreter->Invoke() != kTfLiteOk) return result;

    const int8_t *out_ptr = s_output->data.int8;
    for (int i = 0; i < N_TARGETS; i++) {
        float norm = ((float)out_ptr[i] - OUTPUT_ZP) * OUTPUT_SCALE;
        result.value[i] = denormalize(norm, kTgtMin[i], kTgtMax[i]);
    }

    result.valid = true;
    return result;
}

extern "C" int inference_format_result(const InferenceResult_t *res,
                                        uint32_t                 step,
                                        char                    *buf,
                                        int                      buf_len)
{
    if (!res->valid) {
        return snprintf(buf, (size_t)buf_len,
                        "[Inference] Step %lu: filling window...\r\n",
                        (unsigned long)step);
    }
    return snprintf(buf, (size_t)buf_len,
                    "[Inference] Step %lu | Next 10s:\r\n"
                    "  Sensor0 = %.4f\r\n"
                    "  Sensor1 = %.4f\r\n"
                    "  Sensor2 = %.4f\r\n"
                    "  Sensor3 = %.4f\r\n",
                    (unsigned long)step,
                    res->value[0], res->value[1],
                    res->value[2], res->value[3]);
}
