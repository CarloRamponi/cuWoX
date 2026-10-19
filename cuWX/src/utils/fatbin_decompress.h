/** Decompress nvcc fatbin files
 *
 * Author: Niklas Eiling <niklas.eiling@rwth-aachen.de>
 * SPDX-FileCopyrightText: 2023 Niklas Eiling <niklas.eiling@rwth-aachen.de>
 * SPDX-License-Identifier: Apache-2.0
 *********************************************************************************/

#ifndef __FATBIN_DECOMPRESS_H__
#define __FATBIN_DECOMPRESS_H__

// #ifdef __cplusplus
// extern "C" {
// #endif

#include <stdint.h>
#include <runtime/fatbin.h>

int decompress_fatbin(const struct fat_elf_item *th, uint8_t *output, size_t *output_size);

// #ifdef __cplusplus
// }
// #endif

#endif // __FATBIN_DECOMPRESS_H__