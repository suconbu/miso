#ifndef MISO_COMMON_H_
#define MISO_COMMON_H_

#define _CRT_SECURE_NO_WARNINGS

#ifdef MISO_HEADER_ONLY
#define MISO_INLINE inline
#else // MISO_HEADER_ONLY
#define MISO_INLINE
#endif // MISO_HEADER_ONLY

#include <cstdint>

#endif // MISO_COMMON_H_
