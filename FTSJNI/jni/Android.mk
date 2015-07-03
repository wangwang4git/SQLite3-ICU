
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -L./jni/lib -licuuc
LOCAL_LDLIBS += -L./jni/lib -licui18n
LOCAL_LDLIBS += -L./jni/lib -lsqlite

LOCAL_MODULE := FTSDatabase
LOCAL_SRC_FILES := com_tencent_mobileqq_persistence_FTSDatatbaseDao.c fts3_icuqq.c utils.c

include $(BUILD_SHARED_LIBRARY)
