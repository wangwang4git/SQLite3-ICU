
#ifndef _INCLUDE_UTILS_H_
#define _INCLUDE_UTILS_H_

#include <stdlib.h>
#include <zlib.h>
#include "sqlite3.h"
#include "log-generic.h"

#ifdef ZIP
    #include "qqcompress.h"
#else
    #include "base64.h"
#endif

void ecode_init();

void ecode_release();

void qqcompress(sqlite3_context* context, int argc, sqlite3_value** argv);
void qquncompress(sqlite3_context* context, int argc, sqlite3_value** argv);

#endif  // _INCLUDE_UTILS_H_
