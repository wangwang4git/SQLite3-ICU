
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# 启用zip压缩，默认base64
# LOCAL_CFLAGS += -DZIP

LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -lz
LOCAL_LDLIBS += -L./jni/lib -lsqlite

LOCAL_MODULE := FTSDatabase
LOCAL_SRC_FILES := com_tencent_mobileqq_persistence_fts_FTSDatatbase.c utils.c qqcompress.c base64.c

include $(BUILD_SHARED_LIBRARY)
