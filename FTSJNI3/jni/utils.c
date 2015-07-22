#include "include/utils.h"

#define TAG "FTSDATATBASEDAO"

// 变长参数实现，留待以后吧～
void logInfo(const char* msg1, const char* msg2)
{
    if (msg2 == NULL)
    {
        __android_log_print(ANDROID_LOG_INFO, TAG, "%s", msg1);
    }
    else
    {
        __android_log_print(ANDROID_LOG_INFO, TAG, "%s%s", msg1, msg2);
    }
}

void logWarn(const char* msg1, const char* msg2)
{
    if (msg2 == NULL)
    {
        __android_log_print(ANDROID_LOG_WARN, TAG, "%s", msg1);
    }
    else
    {
        __android_log_print(ANDROID_LOG_WARN, TAG, "%s%s", msg1, msg2);
    }
}

void logError(const char* msg1, const char* msg2)
{
    if (msg2 == NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", msg1);
    }
    else
    {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "%s%s", msg1, msg2);
    }
}
