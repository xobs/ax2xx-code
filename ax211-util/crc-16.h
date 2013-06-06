#ifndef __CRC16_H__
#define __CRC16_H__
#include <stdint.h>
uint16_t crc16(void *data_p, uint16_t length);
uint8_t crc7(const uint8_t *buffer, size_t len);

#endif // __CRC16_H__
