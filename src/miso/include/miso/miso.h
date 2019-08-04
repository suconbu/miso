#ifndef MISO_MISO_H_
#define MISO_MISO_H_

#define _CRT_SECURE_NO_WARNINGS

#ifndef MISO_BYTE_TYPE
#define MISO_BYTE_TYPE unsigned char
#endif // MISO_BYTE_TYPE

#ifndef MISO_CORE_FILESTREAM_BUFFER_SIZE
#define MISO_CORE_FILESTREAM_BUFFER_SIZE 256
#endif // MISO_CORE_FILESTREAM_BUFFER_SIZE

#include "miso/binary_reader.h"
#include "miso/buffer.h"
#include "miso/color.h"
#include "miso/color_container.h"
#include "miso/color_formatter.h"
#include "miso/endian.h"
#include "miso/interpolator.h"
#include "miso/numeric.h"
#include "miso/scalar.h"
#include "miso/stream.h"
#include "miso/string.h"
#include "miso/xml_reader.h"

#endif // MISO_MISO_H_
