/** Decompress nvcc fatbin files
 *
 * Author: Niklas Eiling <niklas.eiling@rwth-aachen.de>
 * SPDX-FileCopyrightText: 2023 Niklas Eiling <niklas.eiling@rwth-aachen.de>
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#include <fatbin_decompress.h>
#include <hexdump.h>
#include <cstring>

// #define FATBIN_DECOMPRESS_DEBUG

#define FATBIN_TEXT_MAGIC     0xBA55ED50

/** Decompresses a fatbin file
 * @param input Pointer compressed input data
 * @param input_size Size of compressed data
 * @param output preallocated memory where decompressed output should be stored
 * @param output_size size of output buffer. Should be equal to the size of the decompressed data
 */
size_t decompress(const uint8_t* input, size_t input_size, uint8_t* output, size_t output_size)
{
    size_t ipos = 0, opos = 0;  
    uint64_t next_nclen;  // length of next non-compressed segment
    uint64_t next_clen;   // length of next compressed segment
    uint64_t back_offset; // negative offset where redudant data is located, relative to current opos

    while (ipos < input_size) {
        next_nclen = (input[ipos] & 0xf0) >> 4;
        next_clen = 4 + (input[ipos] & 0xf);
        if (next_nclen == 0xf) {
            do {
                next_nclen += input[++ipos];
            } while (input[ipos] == 0xff);
        }
        
        if (memcpy(output + opos, input + (++ipos), next_nclen) == NULL) {
            fprintf(stderr, "Error copying data");
            return 0;
        }
#ifdef FATBIN_DECOMPRESS_DEBUG
        printf("%#04zx/%#04zx nocompress (len:%#zx):\n", opos, ipos, next_nclen);
        hexdump(output + opos, next_nclen);
#endif
        ipos += next_nclen;
        opos += next_nclen;
        if (ipos >= input_size || opos >= output_size) {
            break;
        }
        back_offset = input[ipos] + (input[ipos + 1] << 8);
        ipos += 2;
        if (next_clen == 0xf+4) {
            do {
                next_clen += input[ipos++];
            } while (input[ipos - 1] == 0xff);
        }
#ifdef FATBIN_DECOMPRESS_DEBUG
        printf("%#04zx/%#04zx compress (decompressed len: %#zx, back_offset %#zx):\n", opos, ipos, next_clen, back_offset);
#endif
        if (next_clen <= back_offset) {
            if (memcpy(output + opos, output + opos - back_offset, next_clen) == NULL) {
                fprintf(stderr, "Error copying data");
                return 0;
            }
        } else {
            if (memcpy(output + opos, output + opos - back_offset, back_offset) == NULL) {
                fprintf(stderr, "Error copying data");
                return 0;
            }
            for (size_t i = back_offset; i < next_clen; i++) {
                output[opos + i] = output[opos + i - back_offset];
            }
        }
#ifdef FATBIN_DECOMPRESS_DEBUG
        hexdump(output + opos, next_clen);
#endif
        opos += next_clen;
    }
    return opos;
}

int decompress_fatbin(
    const fat_elf_item_t *th,
    uint8_t *output,
    size_t *output_size
)
{
    fat_elf_item_t *th_out = NULL;
    uint8_t *output_pos = 0;
    size_t padding;
    int ret = 0;

    if (output == NULL || output_size == NULL || th == NULL) {
        fprintf(stderr, "Error: invalid parameters\n");
        return -1;
    }

    output_pos = output;
    *output_size += th->uncompressed_size + th->header_size;
    const uint8_t *input = (const uint8_t *)th + th->header_size;

    if (memcpy(output_pos, th, th->header_size) == NULL) {
        fprintf(stderr, "Error copying data");
        return -1;
    }
    th_out = ((fat_elf_item_t*)output_pos);
    th_out->flags &= ~FATBIN_FLAG_COMPRESS;  // clear compressed flag
    th_out->compressed_size = 0;             // clear compressed size
    th_out->uncompressed_size = 0;           // clear decompressed size
    th_out->size = th->uncompressed_size;    // set size

    output_pos += th->header_size;

    size_t decompress_ret;

    if ((decompress_ret = decompress(input, th->compressed_size, output_pos, th->uncompressed_size)) != th->uncompressed_size) {
        fprintf(stderr, "Decompression failed: decompressed size (%#0zx) is not as indicated in header (%#0zx).\n",
                decompress_ret, th->uncompressed_size);
        return -1;
    }

    output_pos += th->uncompressed_size;

    // if (input_pos != (uint8_t*)th + eh->size) {
    //     printf("There is %#zx bytes of data remaining\n", (uint8_t*)th + eh->size - input_pos);
    // }

    padding = ((8 - (size_t)th->uncompressed_size) % 8);
    // Because we always allocated enough memory for one more elf_header and this is smaller than
    // the maximal padding of 7, we do not have to reallocate here.
    memset(output_pos, 0, padding);
    *output_size += padding;
    th_out->size += padding;

    return ret;
}