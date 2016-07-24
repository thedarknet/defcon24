/**
 * \file crc.c
 * Functions and types for CRC checks.
 *
 * Generated on Mon Jul 18 07:07:41 2016,
 * by pycrc v0.9, https://pycrc.org
 * using the configuration:
 *    Width         = 8
 *    Poly          = 0x07
 *    Xor_In        = 0x00
 *    ReflectIn     = False
 *    Xor_Out       = 0x00
 *    ReflectOut    = False
 *    Algorithm     = table-driven
 *****************************************************************************/
#include "crc.h"     /* include the header file generated with pycrc */
#include <stdlib.h>
#include <stdint.h>

/**
 * Static table used for the table_driven implementation.
 *****************************************************************************/
static const crc_t crc_table[16] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d
};

/**
 * Update the crc value with new data.
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param data_len Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 *****************************************************************************/
crc_t crc_update(crc_t crc, const void *data, size_t data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    unsigned int tbl_idx;

    while (data_len--) {
        tbl_idx = (crc >> 4) ^ (*d >> 4);
        crc = crc_table[tbl_idx & 0x0f] ^ (crc << 4);
        tbl_idx = (crc >> 4) ^ (*d >> 0);
        crc = crc_table[tbl_idx & 0x0f] ^ (crc << 4);

        d++;
    }
    return crc & 0xff;
}


