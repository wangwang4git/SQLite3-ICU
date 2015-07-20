#ifndef _Included_qqcompress
#define _Included_qqcompress

#include <zlib.h>

unsigned char* qq_compress(unsigned char* sour, unsigned long *destlen, unsigned long sourlen);

unsigned char* qq_uncompress(unsigned char* sour, unsigned long *destlen, unsigned long sourlen);

#endif