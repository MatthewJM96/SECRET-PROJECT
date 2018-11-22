/**
 * @file types.h
 * @brief Provides a set of consistent typenames.
 */

#pragma once

#if !defined(SP_Types_h__)
#define SP_Types_h__

#include <cstdint>

// Integral types.
using   i8 = int8_t;
using  i16 = int16_t;
using  i32 = int32_t;
using  i64 = int64_t;
using  ui8 = uint8_t;
using ui16 = uint16_t;
using ui32 = uint32_t;
using ui64 = uint64_t;

// Floating-point types.
using f32 = float;
using f64 = double;

// Tau > Pi.
#undef M_PI // Get rid of M_PI redefinition warnings.
#define M_TAU  6.28318530717958647692
#define M_TAUF 6.2831853f
#define M_PI   M_TAU / 2.0
#define M_PIF  M_TAUF / 2.0f

#endif // !defined(SP_Types_h__)