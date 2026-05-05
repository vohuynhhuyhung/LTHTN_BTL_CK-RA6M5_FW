/*
 * iaq_packet.h
 *
 * Binary frame protocol for sending IAQ data over UART to ESP32.
 *
 * Frame layout (13 bytes total):
 *   [STX=0xAA][LEN=0x08][CMD=0x01][iaq_x10:2][tvoc_x1000:2][eco2_x10:2][etoh_x1000:2][CRC16:2][ETX=0x55]
 *
 * CRC16 (CCITT-FALSE): computed over bytes from LEN through end of payload (not STX, not CRC, not ETX).
 */

#ifndef IAQ_PACKET_H_
#define IAQ_PACKET_H_

#include <stdint.h>

#define IAQ_FRAME_STX   (0xAAU)
#define IAQ_FRAME_ETX   (0x55U)
#define IAQ_FRAME_CMD   (0x01U)
#define IAQ_FRAME_DATA  (0x02U)
#define IAQ_PAYLOAD_FRAME_LEN   (5U)        /* payload length in bytes */
#define IAQ_FRAME_TOTAL (11U)       /* total frame bytes: STX+LEN+CMD+8payload+CRC16+ETX */

typedef enum {
    SENSOR_ID_IAQ = 0,
    SENSOR_ID_TVOC,
    SENSOR_ID_ECO2,
    SENSOR_ID_ETOH,
    SENSOR_MAX
} sensor_id_t;


typedef struct {
    uint8_t id;
    float   data_current;
    float   data_predict;
} sensor_manager_t;


extern sensor_manager_t g_sensors[SENSOR_MAX];
/* Build and send one binary IAQ frame over UART */
void iaq_packet_send(const sensor_manager_t *sensor);

#endif /* IAQ_PACKET_H_ */
