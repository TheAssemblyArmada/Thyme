/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Some extra string manipulation functions not present in all standard CRTs
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_STRINGEX_H
#define BASE_STRINGEX_H

#include "bittype.h"
#include "unichar.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t ex_strlcat(char *dst, const char *src, size_t dsize);
size_t ex_strlcpy(char *dst, const char *src, size_t dsize);
size_t u_strlcpy(unichar_t *dst, const unichar_t *src, size_t dsize);
char *ex_strtrim(char *str);
char *ex_strlwr(char *str);
char *ex_strupr(char *str);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BASE_STRINGEX_H
