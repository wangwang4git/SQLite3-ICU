#include <android/log.h>

#ifndef _Included_utils
#define _Included_utils

// log函数，参考：http://mobilepearls.com/labs/native-android-api/#logging
void logInfo(char* msg);
void logWarn(char* msg);
void logError(char* msg);

#endif
