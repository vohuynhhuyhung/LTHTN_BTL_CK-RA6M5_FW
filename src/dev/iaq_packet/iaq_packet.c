/*
 * Frame layout (14 bytes):
 *   Byte 0    : STX  = 0xAA
 *   Byte 1    : LEN  = 0x08 (payload length)
 *   Byte 2    : CMD  = 0x01
 *   Byte 3-4  : iaq_x10    (uint16_t big-endian, e.g. IAQ=1.5 → 15)
 *   Byte 5-6  : tvoc_x1000 (uint16_t big-endian, e.g. TVOC=0.031 → 31)
 *   Byte 7-8  : eco2_x10   (uint16_t big-endian, e.g. eCO2=400.0 → 4000)
 *   Byte 9-10 : etoh_x1000 (uint16_t big-endian, e.g. EtOH=0.016 → 16)
 *   Byte 11-12: CRC16 CCITT-FALSE (big-endian), computed over bytes 1..10
 *   Byte 13   : ETX  = 0x55
 */

#include "iaq_packet.h"
#include "../uart/uart.h"
#include <stdio.h>

// verify packet terminal
#define IAQ_PACKET_DEBUG 1

static uint16_t crc16_ccitt(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1) ^ 0x1021U) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

void iaq_packet_send(const iaq_data_t *data)
{
    uint8_t frame[IAQ_FRAME_TOTAL];

    uint16_t iaq_x10    = (uint16_t)(data->iaq  * 10.0f);
    uint16_t tvoc_x1000 = (uint16_t)(data->tvoc * 1000.0f);
    uint16_t eco2_x10   = (uint16_t)(data->eco2 * 10.0f);
    uint16_t etoh_x1000 = (uint16_t)(data->etoh * 1000.0f);

    frame[0]  = IAQ_FRAME_STX;
    frame[1]  = IAQ_FRAME_LEN;
    frame[2]  = IAQ_FRAME_CMD;
    frame[3]  = (uint8_t)(iaq_x10    >> 8);
    frame[4]  = (uint8_t)(iaq_x10    & 0xFF);
    frame[5]  = (uint8_t)(tvoc_x1000 >> 8);
    frame[6]  = (uint8_t)(tvoc_x1000 & 0xFF);
    frame[7]  = (uint8_t)(eco2_x10   >> 8);
    frame[8]  = (uint8_t)(eco2_x10   & 0xFF);
    frame[9]  = (uint8_t)(etoh_x1000 >> 8);
    frame[10] = (uint8_t)(etoh_x1000 & 0xFF);

    uint16_t crc = crc16_ccitt(&frame[1], 10);
    frame[11] = (uint8_t)(crc >> 8);
    frame[12] = (uint8_t)(crc & 0xFF);
    frame[13] = IAQ_FRAME_ETX;

#ifdef IAQ_PACKET_DEBUG
    char hex[IAQ_FRAME_TOTAL * 3 + 8];
    int pos = 0;
    pos += snprintf(hex + pos, sizeof(hex) - (size_t)pos, "[PKT]");
    for (uint8_t i = 0; i < IAQ_FRAME_TOTAL; i++)
    {
        pos += snprintf(hex + pos, sizeof(hex) - (size_t)pos, " %02X", frame[i]);
    }
    pos += snprintf(hex + pos, sizeof(hex) - (size_t)pos, "\r\n");
    uart_send_buf((uint8_t *)hex, (uint32_t)pos);
#else
    uart_send_buf(frame, IAQ_FRAME_TOTAL);
#endif
}
