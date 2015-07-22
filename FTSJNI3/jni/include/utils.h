#include <stdio.h>
#include <android/log.h>

#ifndef _Included_utils
#define _Included_utils

// log函数，参考：http://mobilepearls.com/labs/native-android-api/#logging
void logInfo(const char* msg1, const char* msg2);
void logWarn(const char* msg1, const char* msg2);
void logError(const char* msg1, const char* msg2);

#endif
