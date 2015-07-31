#include "include/utils.h"

void ecode_init()
{
#ifdef ZIP
    const char* version = zlibVersion();
    LOGI("zlib version = %s", version);
#else
    LOGI("base64 version = add salt");
    build_decoding_table();
#endif
}

void ecode_release()
{
#ifdef ZIP
    LOGI("zlib release");
#else
    LOGI("base64 release");
    base64_cleanup();
#endif
}

void qqcompress(sqlite3_context* context, int argc, sqlite3_value** argv)
{
    int len = sqlite3_value_bytes(argv[0]);
    unsigned char* msg = sqlite3_malloc(sizeof(unsigned char) * len);
    memcpy(msg, sqlite3_value_blob(argv[0]), len);

    // 字段为空，提前退出
    if (len == 0)
    {
        sqlite3_result_blob(context, msg, len, sqlite3_free);
        return;
    }

    int len2;
    unsigned char* msg2;

#ifdef ZIP
    msg2 = qq_compress(msg, &len2, len);
#else
    msg2 = base64_encode(msg, len, &len2);
#endif

    sqlite3_free(msg);

    sqlite3_result_blob(context, msg2, len2, sqlite3_free);
}

void qquncompress(sqlite3_context* context, int argc, sqlite3_value** argv)
{
    int len = sqlite3_value_bytes(argv[0]);
    unsigned char* msg = sqlite3_malloc(sizeof(unsigned char) * len);
    memcpy(msg, sqlite3_value_blob(argv[0]), len);

    // 字段为空，提前退出
    if (len == 0)
    {
        sqlite3_result_blob(context, msg, len, sqlite3_free);
        return;
    }

    int len2;
    unsigned char* msg2;

#ifdef ZIP
    msg2 = qq_uncompress(msg, &len2, len);
#else
    msg2 = base64_decode(msg, len, &len2);
#endif

    sqlite3_free(msg);

    sqlite3_result_blob(context, msg2, len2, sqlite3_free);
}