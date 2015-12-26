#ifndef UTIL_FIXED_TYPES_H
#define UTIL_FIXED_TYPES_H
/*
 * File         : fixed_types.h
 * Project      : General Utilities
 * Creator(s)   : Grant Ayers (geayers@stanford.edu)
 *
 * Modification History:
 *   Rev  Date          Initials  Description of Change
 *   1.0  1-Aug-2013    GEA       Initial design.
 *
 * Standards/Formatting:
 *   C++11, 4 soft tab, wide column.
 *
 * Description:
 *   Standard fixed types borrowed from MIT Graphite.
 */
#include <stdint.h>

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t  U8;

typedef int64_t S64;
typedef int32_t S32;
typedef int16_t S16;
typedef int8_t  S8;

typedef U8 Byte;
typedef U8 Boolean;

#endif // UTIL_FIXED_TYPES_H

