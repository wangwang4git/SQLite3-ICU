#include <stdio.h>
#include <android/log.h>

#ifndef _Included_utils
#define _Included_utils

// log函数，参考：http://mobilepearls.com/labs/native-android-api/#logging
void logInfo(char* msg1, char* msg2);
void logWarn(char* msg1, char* msg2);
void logError(char* msg1, char* msg2);

#endif
