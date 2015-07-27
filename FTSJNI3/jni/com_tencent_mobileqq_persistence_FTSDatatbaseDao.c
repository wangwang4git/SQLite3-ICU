#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/com_tencent_mobileqq_persistence_FTSDatatbaseDao.h"
#include "include/sqlite3.h"
#include "include/utils.h"

#ifdef ZIP
#include "include/qqcompress.h"
#else
#include "include/base64.h"
#endif


// dbName和Java层保持一致
#define DB_FILE "/data/data/com.tencent.mobileqq/databases/IndexQQMsg.db"

void qqcompress(sqlite3_context* context, int argc, sqlite3_value** argv);
void qquncompress(sqlite3_context* context, int argc, sqlite3_value** argv);

static sqlite3* db = NULL;
static sqlite3_stmt* stmt = NULL;

jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_initFTS(JNIEnv* env, jobject thiz)
{
    int errCode = sqlite3_open_v2(DB_FILE, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't open database IndexQQMsg.db, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    errCode = sqlite3_exec(db, "PRAGMA cache_size=4000;", NULL, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't set PRAGMA cache_size = 4000, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    errCode = sqlite3_create_function(db, "qqcompress", 1, SQLITE_UTF8, NULL, qqcompress, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't create function, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    errCode = sqlite3_create_function(db, "qquncompress", 1, SQLITE_UTF8, NULL, qquncompress, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't create function, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    char* sql = "CREATE VIRTUAL TABLE IF NOT EXISTS IndexMsg USING fts4(uin, istroop, time, shmsgseq, msg, msgindex, compress=qqcompress, uncompress=qquncompress);";
    // char* sql = "CREATE VIRTUAL TABLE IF NOT EXISTS IndexMsg USING fts4(uin, istroop, time, shmsgseq, msg, msgindex);";
    errCode = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't create virtual table IndexMsg, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

#ifdef ZIP
    const char* version = zlibVersion();
    logInfo("FTS init zlib version = ", version);
#endif

    logInfo("FTS init...", NULL);
    return 0;
}


jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_insertFTS(JNIEnv* env, jobject thiz, jlong juin, jint jistroop, jlong jtime, jlong jshmsgseq, jstring jmsg, jstring jmsgindex)
{
    long long uin = (long long) juin;
    int istroop = (int) jistroop;
    long long msgtime = (long long) jtime;
    long long shmsgseq = (long long) jshmsgseq;
    const char* msg = (*env)->GetStringUTFChars(env, jmsg, NULL);
    const char* msgindex = (*env)->GetStringUTFChars(env, jmsgindex, NULL);

    // 创建sqlite3_stmt
    if (NULL == stmt)
    {
        char* zSql = "INSERT INTO IndexMsg(uin, istroop, time, shmsgseq, msg, msgindex) VALUES(?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(db, zSql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            logError("Can't prepare stmt, ", sqlite3_errmsg(db));

            stmt = NULL;

            // sqlite3_close(db);
            return rc;
        }
    }

    sqlite3_bind_int64(stmt, 1, uin);

    sqlite3_bind_int(stmt, 2, istroop);

    sqlite3_bind_int64(stmt, 3, msgtime);

    sqlite3_bind_int64(stmt, 4, shmsgseq);

    sqlite3_bind_text(stmt, 5, msg, -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 6, msgindex, -1, SQLITE_STATIC);

    sqlite3_step(stmt);

    sqlite3_reset(stmt);

    (*env)->ReleaseStringUTFChars(env, jmsgindex, msgindex);
    (*env)->ReleaseStringUTFChars(env, jmsg, msg);

    logInfo("FTS insert...", NULL);

    return 0;
}

jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_insertFTSWithTrans(JNIEnv* env, jobject thiz, jobject ftsmsglist)
{
    // 获取ArrayList class类
    jclass list_clazz = (*env)->GetObjectClass(env, ftsmsglist);

    // 获取ArrayList类get函数ID
    jmethodID list_get = (*env)->GetMethodID(env, list_clazz , "get", "(I)Ljava/lang/Object;");

    // 获取ArrayList类size函数ID
    jmethodID list_size = (*env)->GetMethodID(env, list_clazz , "size", "()I");

    jint list_len = (*env)->CallIntMethod(env, ftsmsglist, list_size);

    if (list_len > 0)
    {
        int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            logError("Can't begin transcation, ", sqlite3_errmsg(db));
        }
    }

    int i = 0;
    for (i = 0; i < list_len; ++i)
    {
        jobject ftsmsg_obj = (*env)->CallObjectMethod(env, ftsmsglist, list_get, i);

        // 获取FTSMsgItem class类
        jclass ftsmsg_clazz = (*env)->GetObjectClass(env, ftsmsg_obj);

        // 获取FTSMsgItem类getUin函数ID
        jmethodID ftsmsg_getUin = (*env)->GetMethodID(env, ftsmsg_clazz, "getUin", "()J");

        // 获取FTSMsgItem类getIstroop函数ID
        jmethodID ftsmsg_getIstroop = (*env)->GetMethodID(env, ftsmsg_clazz, "getIstroop", "()I");

        // 获取FTSMsgItem类getTime函数ID
        jmethodID ftsmsg_getTime = (*env)->GetMethodID(env, ftsmsg_clazz, "getTime", "()J");

        // 获取FTSMsgItem类getShmsgseq函数ID
        jmethodID ftsmsg_getShmsgseq = (*env)->GetMethodID(env, ftsmsg_clazz, "getShmsgseq", "()J");

        // 获取FTSMsgItem类getMsg函数ID
        jmethodID ftsmsg_getMsg = (*env)->GetMethodID(env, ftsmsg_clazz, "getMsg", "()Ljava/lang/String;");

        // 获取FTSMsgItem类getMsgindex函数ID
        jmethodID ftsmsg_getMsgindex = (*env)->GetMethodID(env, ftsmsg_clazz, "getMsgindex", "()Ljava/lang/String;");


        jlong juin = (*env)->CallLongMethod(env, ftsmsg_obj, ftsmsg_getUin);
        jint jistroop = (*env)->CallIntMethod(env, ftsmsg_obj, ftsmsg_getIstroop);
        jlong jtime = (*env)->CallLongMethod(env, ftsmsg_obj, ftsmsg_getTime);
        jlong jshmsgseq = (*env)->CallLongMethod(env, ftsmsg_obj, ftsmsg_getShmsgseq);
        jstring jmsg = (jstring)(*env)->CallObjectMethod(env, ftsmsg_obj, ftsmsg_getMsg);
        jstring jmsgindex = (jstring)(*env)->CallObjectMethod(env, ftsmsg_obj, ftsmsg_getMsgindex);

        long long uin = (long long) juin;
        int istroop = (int) jistroop;
        long long msgtime = (long long) jtime;
        long long shmsgseq = (long long) jshmsgseq;
        const char* msg = (*env)->GetStringUTFChars(env, jmsg, NULL);
        const char* msgindex = (*env)->GetStringUTFChars(env, jmsgindex, NULL);

        // 创建sqlite3_stmt
        if (NULL == stmt)
        {
            char* zSql = "INSERT INTO IndexMsg(uin, istroop, time, shmsgseq, msg, msgindex) VALUES(?, ?, ?, ?, ?, ?);";
            int rc = sqlite3_prepare_v2(db, zSql, -1, &stmt, 0);
            if (rc != SQLITE_OK)
            {
                logError("Can't prepare stmt, ", sqlite3_errmsg(db));

                stmt = NULL;

                // sqlite3_close(db);
                return rc;
            }
        }

        sqlite3_bind_int64(stmt, 1, uin);

        sqlite3_bind_int(stmt, 2, istroop);

        sqlite3_bind_int64(stmt, 3, msgtime);

        sqlite3_bind_int64(stmt, 4, shmsgseq);

        sqlite3_bind_text(stmt, 5, msg, -1, SQLITE_STATIC);

        sqlite3_bind_text(stmt, 6, msgindex, -1, SQLITE_STATIC);

        sqlite3_step(stmt);

        sqlite3_reset(stmt);

        (*env)->ReleaseStringUTFChars(env, jmsgindex, msgindex);
        (*env)->ReleaseStringUTFChars(env, jmsg, msg);


        // 避免 local reference table overflow (max=512) 错误
        (*env)->DeleteLocalRef(env, jmsgindex);
        (*env)->DeleteLocalRef(env, jmsg);

        (*env)->DeleteLocalRef(env, ftsmsg_getMsgindex);
        (*env)->DeleteLocalRef(env, ftsmsg_getMsg);
        (*env)->DeleteLocalRef(env, ftsmsg_getShmsgseq);
        (*env)->DeleteLocalRef(env, ftsmsg_getTime);
        (*env)->DeleteLocalRef(env, ftsmsg_getIstroop);
        (*env)->DeleteLocalRef(env, ftsmsg_getUin);
        (*env)->DeleteLocalRef(env, ftsmsg_clazz);
        (*env)->DeleteLocalRef(env, ftsmsg_obj);
    }

    if (list_len > 0)
    {
        int rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            logError("Can't commit transcation, ", sqlite3_errmsg(db));
        }
    }

    logInfo("FTS trans...", NULL);

    return 0;

}

jobject Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_queryFTSGroups(JNIEnv* env, jobject thiz, jstring jsql, jstring jclasspath)
{
    // 获取ArrayList class类
    jclass list_clazz = (*env)->FindClass(env, "java/util/ArrayList");

    // 获取ArrayList类构造函数ID
    jmethodID list_init = (*env)->GetMethodID(env, list_clazz , "<init>", "()V");

    // 获取ArrayList类add函数ID
    jmethodID list_add  = (*env)->GetMethodID(env, list_clazz, "add", "(Ljava/lang/Object;)Z");

    // 构造ArrayList对象
    jobject list_obj = (*env)->NewObject(env, list_clazz , list_init);

    // 获取FTSMsgGroupItem class类
    const char* classpath = (*env)->GetStringUTFChars(env, jclasspath, NULL);
    jclass group_clazz = (*env)->FindClass(env, classpath);
    (*env)->ReleaseStringUTFChars(env, jclasspath, classpath);

    // 获取FTSMsgGroupItem类构造函数ID
    jmethodID group_init = (*env)->GetMethodID(env, group_clazz , "<init>", "(JII)V");

    // 搜索
    char** result;
    int nrows;
    int ncols;
    const char* sql = (*env)->GetStringUTFChars(env, jsql, NULL);
    int rc = sqlite3_get_table(db, sql, &result, &nrows, &ncols, NULL);
    (*env)->ReleaseStringUTFChars(env, jsql, sql);
    if (rc != SQLITE_OK)
    {
        logError("Can't query groups, ", sqlite3_errmsg(db));

        // sqlite3_close(db);
        return list_obj;
    }

    // 搜索结果为空
    if (nrows == 0)
    {
        logWarn("FTS queryFTSGroups: nrows = 0", NULL);

        sqlite3_free_table(result);
        return list_obj;
    }

    // SQL查询语句，SELECT三个字段：uin、istroop、counts
    if (ncols != 3)
    {
        logWarn("FTS queryFTSGroups: ncols != 3", NULL);

        sqlite3_free_table(result);
        return list_obj;
    }

    int i;
    for (i = 0; i < nrows; ++i)
    {
        // 注意：java long和c/c++ long，不一样，小心掉坑里！！
        long long uin = atoll(result[(i + 1) * ncols + 0]);

        int istroop = atoi(result[(i + 1) * ncols + 1]);

        int counts = atoi(result[(i + 1) * ncols + 2]);

        // 构造FTSMsgGroupItem对象
        jobject group_obj = (*env)->NewObject(env, group_clazz, group_init, uin, istroop, counts);

        (*env)->CallBooleanMethod(env, list_obj, list_add, group_obj);

        // 避免 local reference table overflow (max=512) 错误
        (*env)->DeleteLocalRef(env, group_obj);
    }

    sqlite3_free_table(result);
    return list_obj;
}

jobject Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_queryFTSMsgs(JNIEnv* env, jobject thiz, jstring jsql, jstring jclasspath, jlong juin, jint jistroop, jobjectArray jstringarray)
{
    // 获取ArrayList class类
    jclass list_clazz = (*env)->FindClass(env, "java/util/ArrayList");

    // 获取ArrayList类构造函数ID
    jmethodID list_init = (*env)->GetMethodID(env, list_clazz , "<init>", "()V");

    // 获取ArrayList类add函数ID
    jmethodID list_add  = (*env)->GetMethodID(env, list_clazz, "add", "(Ljava/lang/Object;)Z");

    // 构造ArrayList对象
    jobject list_obj = (*env)->NewObject(env, list_clazz , list_init);

    // 获取FTSMsgItem class类
    const char* classpath = (*env)->GetStringUTFChars(env, jclasspath, NULL);
    jclass msg_clazz = (*env)->FindClass(env, classpath);
    (*env)->ReleaseStringUTFChars(env, jclasspath, classpath);

    // 获取FTSMsgItem类构造函数ID
    jmethodID msg_init = (*env)->GetMethodID(env, msg_clazz , "<init>", "(JIJJLjava/lang/String;)V");

    // 搜索
    char** result;
    int nrows;
    int ncols;
    const char* sql = (*env)->GetStringUTFChars(env, jsql, NULL);
    int rc = sqlite3_get_table(db, sql, &result, &nrows, &ncols, NULL);
    (*env)->ReleaseStringUTFChars(env, jsql, sql);
    if (rc != SQLITE_OK)
    {
        logError("Can't query msgs, ", sqlite3_errmsg(db));

        // sqlite3_close(db);
        return list_obj;
    }

    // 搜索结果为空
    if (nrows == 0)
    {
        logWarn("FTS queryFTSMsgs: nrows = 0", NULL);

        sqlite3_free_table(result);
        return list_obj;
    }

    // SQL查询语句，SELECT五个字段：uin、istroop、time、shmsgseq，msg
    if (ncols != 5)
    {
        logWarn("FTS queryFTSMsgs: ncols != 5", NULL);

        sqlite3_free_table(result);
        return list_obj;
    }

    long long queryUin = (long long) juin;
    int queryIstroop = (int) jistroop;

    jsize len = (*env)->GetArrayLength(env, jstringarray);
    char** pstr = (char**) malloc(len * sizeof(char*));
    int i = 0;
    for (i = 0; i < len; i++)
    {
        jstring jstr = (*env)->GetObjectArrayElement(env, jstringarray, i);
        pstr[i] = (char*)(*env)->GetStringUTFChars(env, jstr, 0);

        // 避免 local reference table overflow (max=512) 错误
        (*env)->DeleteLocalRef(env, jstr);
    }

    for (i = 0; i < nrows; ++i)
    {
        // 注意：java long和c/c++ long，不一样，小心掉坑里！！
        long long uin = atoll(result[(i + 1) * ncols + 0]);

        int istroop = atoi(result[(i + 1) * ncols + 1]);

        if (queryUin != uin || queryIstroop != istroop)
        {
            continue;
        }

        long long time = atoll(result[(i + 1) * ncols + 2]);

        long long shmsgseq = atoll(result[(i + 1) * ncols + 3]);

        char* msg = result[(i + 1) * ncols + 4];

        // 后续fix，英文大小写的坑
        int ret = -1;
        int j = 0;
        for (j = 0; j < len; ++j)
        {
            if (strstr(msg, pstr[j]) != NULL)
            {
                ret = 0;
            }
        }
        if (ret == -1)
        {
            continue;
        }

        jstring jmsg = (*env)->NewStringUTF(env, msg);

        // 构造FTSMsgItem对象
        jobject msg_obj = (*env)->NewObject(env, msg_clazz, msg_init, uin, istroop, time, shmsgseq, jmsg);

        (*env)->CallBooleanMethod(env, list_obj, list_add, msg_obj);

        // 避免 local reference table overflow (max=512) 错误
        (*env)->DeleteLocalRef(env, msg_obj);
        (*env)->DeleteLocalRef(env, jmsg);
    }

    free(pstr);

    return list_obj;
}

jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_closeFTS(JNIEnv* env, jobject thiz)
{
    if (db != NULL)
    {
        sqlite3_close(db);
        db = NULL;
    }

    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    logInfo("FTS close...", NULL);
    return 0;
}

void qqcompress(sqlite3_context* context, int argc, sqlite3_value** argv)
{
#ifdef ZIP
    int len = sqlite3_value_bytes(argv[0]);
    unsigned char* msg = sqlite3_malloc(sizeof(unsigned char) * len);
    memcpy(msg, sqlite3_value_blob(argv[0]), len);

    unsigned long destlen;
    unsigned char* msg2 = qq_compress(msg, &destlen, len);
    sqlite3_free(msg);

    sqlite3_result_blob(context, msg2, destlen, sqlite3_free);
#else
    const char* msg = sqlite3_value_text(argv[0]);

    char* msg2 = base64_encode((char*) msg);

    sqlite3_result_text(context, msg2, strlen(msg2), sqlite3_free);
#endif
}

void qquncompress(sqlite3_context* context, int argc, sqlite3_value** argv)
{
#ifdef ZIP
    int len = sqlite3_value_bytes(argv[0]);
    unsigned char* msg = sqlite3_malloc(sizeof(unsigned char) * len);
    memcpy(msg, sqlite3_value_blob(argv[0]), len);

    unsigned long destlen;
    unsigned char* msg2 = qq_uncompress(msg, &destlen, len);
    sqlite3_free(msg);

    sqlite3_result_blob(context, msg2, destlen, sqlite3_free);
#else
    const char* msg = sqlite3_value_text(argv[0]);

    char* msg2 = base64_decode((char*) msg);

    sqlite3_result_text(context, msg2, strlen(msg2), sqlite3_free);
#endif
}
