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

/* Chọn một trong hai mode (comment cái không dùng):
 *   IAQ_PACKET_DEBUG : in hex frame "AA 08 01 ... 55\r\n"  — để verify frame
 *   IAQ_PACKET_ASCII : in CSV      "1.0,0.031,400.0,0.016\r\n" — để Python ghi CSV
 *   Không define cả hai            : gửi binary frame thật đến ESP32 */
//#define IAQ_PACKET_DEBUG
#define IAQ_PACKET_ASCII

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

#if defined(IAQ_PACKET_DEBUG)
    /* "AA 08 01 00 0A ... 55\r\n" */
    char hex[IAQ_FRAME_TOTAL * 3 + 3];
    int pos = 0;
    for (uint8_t i = 0; i < IAQ_FRAME_TOTAL; i++)
    {
        pos += snprintf(hex + pos, sizeof(hex) - (size_t)pos,
                        i < (IAQ_FRAME_TOTAL - 1U) ? "%02X " : "%02X\r\n", frame[i]);
    }
    uart_send_buf((uint8_t *)hex, (uint32_t)pos);
#elif defined(IAQ_PACKET_ASCII)
    /* "1.0,0.031,400.0,0.016\r\n" — Python đọc và ghi CSV */
    char asc[48];
    int alen = snprintf(asc, sizeof(asc), "%d.%d,%d.%03d,%d.%d,%d.%03d\r\n",
                        iaq_x10    / 10,  iaq_x10    % 10,
                        tvoc_x1000 / 1000, tvoc_x1000 % 1000,
                        eco2_x10   / 10,  eco2_x10   % 10,
                        etoh_x1000 / 1000, etoh_x1000 % 1000);
    uart_send_buf((uint8_t *)asc, (uint32_t)alen);
#else
    uart_send_buf(frame, IAQ_FRAME_TOTAL);
#endif
}
