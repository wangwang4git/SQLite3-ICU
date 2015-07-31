
// log函数，参考：http://mobilepearls.com/labs/native-android-api/#logging

#ifndef _INCLUDE_LOG_GENERIC_H
#define _INCLUDE_LOG_GENERIC_H

#include <android/log.h>

#define LOG_TAG "Q.fts.db.jni"

#ifdef ANDROID
	#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
	#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
	#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
	#define QUOTEME_(x) #x
	#define QUOTEME(x) QUOTEME_(x)
	#define LOGI(...) printf("I/" LOG_TAG " (" __FILE__ ":" QUOTEME(__LINE__) "): " __VA_ARGS__)
	#define LOGW(...) printf("W/" LOG_TAG " (" __FILE__ ":" QUOTEME(__LINE__) "): " __VA_ARGS__)
	#define LOGE(...) printf("E/" LOG_TAG " (" __FILE__ ":" QUOTEME(__LINE__) "): " __VA_ARGS__)
#endif

#endif // _INCLUDE_LOG_GENERIC_H
