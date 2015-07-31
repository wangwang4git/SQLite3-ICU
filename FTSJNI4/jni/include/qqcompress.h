
// zip压缩接口

#ifndef _INCLUDED_QQCOMPRESS_H
#define _INCLUDED_QQCOMPRESS_H

#include <zlib.h>
#include "sqlite3.h"
#include "log-generic.h"

unsigned char* qq_compress(unsigned char* sour, int *destlen, int sourlen);

unsigned char* qq_uncompress(unsigned char* sour, int *destlen, int sourlen);

#endif  // _INCLUDED_QQCOMPRESS_H