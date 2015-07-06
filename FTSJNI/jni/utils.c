#include "include/utils.h"

#define TAG "FTSDATATBASEDAO"

void logInfo(char* msg)
{
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", msg);
}

void logWarn(char* msg)
{
    __android_log_print(ANDROID_LOG_WARN, TAG, "%s", msg);
}

void logError(char* msg)
{
    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", msg);
}
