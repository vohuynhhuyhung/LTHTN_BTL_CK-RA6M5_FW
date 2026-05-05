#ifndef DEV_EDGE_WRAPPER_INFERENCE_H_
#define DEV_EDGE_WRAPPER_INFERENCE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Cấu hình model ──────────────────────────────────────── */
#define WINDOW_SIZE         12
#define N_FEATURES          4
#define N_TARGETS           4
#define TENSOR_ARENA_SIZE   (100 * 1024)

/* ── Thông số scaler ─────────────────────────────────────── */
#define FEAT_MIN_0      8.5734266e-01f
#define FEAT_MIN_1      1.1412587e-02f
#define FEAT_MIN_2      3.9996448e+02f
#define FEAT_MIN_3      5.7552448e-03f

#define FEAT_MAX_0      3.1083915e+00f
#define FEAT_MAX_1      6.7811191e-01f
#define FEAT_MAX_2      5.5428351e+02f
#define FEAT_MAX_3      3.6058742e-01f

#define TGT_MIN_0       8.5734266e-01f
#define TGT_MIN_1       1.1412587e-02f
#define TGT_MIN_2       3.9996448e+02f
#define TGT_MIN_3       6.0909092e-03f

#define TGT_MAX_0       3.1083915e+00f
#define TGT_MAX_1       6.7811191e-01f
#define TGT_MAX_2       5.5428351e+02f
#define TGT_MAX_3       3.6058742e-01f

/* ── Quantization params ─────────────────────────────────── */
#define INPUT_SCALE     0.00024852f
#define INPUT_ZP        (-128)
#define OUTPUT_SCALE    0.00027478f
#define OUTPUT_ZP       (-126)

/* ── Kiểu dữ liệu ────────────────────────────────────────── */
typedef struct {
    float value[N_TARGETS];
    bool  valid;
} InferenceResult_t;

typedef struct {
    float data[WINDOW_SIZE][N_FEATURES];
    int   head;
    int   count;
} SensorWindow_t;

/* ── API ─────────────────────────────────────────────────── */
bool              inference_init        (void);
void              inference_window_init (SensorWindow_t *win);
void              inference_push        (SensorWindow_t *win,
                                         const float raw[N_FEATURES]);
InferenceResult_t inference_run         (const SensorWindow_t *win);
int               inference_format_result(const InferenceResult_t *res,
                                          uint32_t step,
                                          char    *buf,
                                          int      buf_len);

#ifdef __cplusplus
}   /* ← Thiếu dòng này trong file gốc của bạn */
#endif

#endif /* DEV_EDGE_WRAPPER_INFERENCE_H_ */
