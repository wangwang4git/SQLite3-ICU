#include <stdio.h>
#include <string.h>

#include "include/com_tencent_mobileqq_persistence_FTSDatatbaseDao.h"
#include "include/utils.h"
#include "include/sqlite3.h"
#include "include/fts3_tokenizer.h"

// dbName和Java层保持一致
#define DB_FILE "/data/data/com.tencent.mobileqq/databases/IndexQQMsg.db"

static sqlite3* db = NULL;
static sqlite3_stmt* stmt = NULL;

extern char* getSegmentedMsg(char* msg);

jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_initFTS(JNIEnv* env, jobject thiz)
{
    int errCode = sqlite3_open_v2(DB_FILE, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't open database IndexQQMsg.db, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    char* sql = "CREATE VIRTUAL TABLE IF NOT EXISTS IndexMsg USING fts3(uin, istroop, time, shmsgseq, msg, msgindex);";
    errCode = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        logError("Can't create virtual table IndexMsg, ", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    logInfo("FTS init...", NULL);
    return 0;
}


// 事务写，参数绑定留待后续
jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_insertFTS(JNIEnv* env, jobject thiz, jlong juin, jint jistroop, jlong jtime, jlong jshmsgseq, jstring jmsg)
{
    long long uin = (long long) juin;
    int istroop = (int) jistroop;
    long long msgtime = (long long) jtime;
    long long shmsgseq = (long long) jshmsgseq;
    const char* msg = (*env)->GetStringUTFChars(env, jmsg, NULL);

    // 分词，再组装
    char* segments = getSegmentedMsg(msg);
    if (NULL == segments || strlen(segments) == 0)
    {
        logWarn("FTS insert: msg is null...", NULL);
        return -1;
    }
    logInfo("FTS insert: segments = ", segments);

    // 创建sqlite3_stmt
    if (NULL == stmt)
    {
        char* zSql = "INSERT INTO IndexMsg(uin, istroop, time, shmsgseq, msg, msgindex) VALUES(?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(db, zSql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            logError(sqlite3_errmsg(db), NULL);
            return rc;
        }
    }

    sqlite3_bind_int64(stmt, 1, uin);
    sqlite3_bind_int(stmt, 2, istroop);
    sqlite3_bind_int64(stmt, 3, msgtime);
    sqlite3_bind_int64(stmt, 4, shmsgseq);
    sqlite3_bind_text(stmt, 5, msg, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, segments, -1, SQLITE_STATIC);

    sqlite3_step(stmt);

    sqlite3_reset(stmt);

    free(segments);
    (*env)->ReleaseStringUTFChars(env, jmsg, msg);

    logInfo("FTS insert...", NULL);

    return 0;
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

jstring Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_wordSegment(JNIEnv* env, jclass clazz, jstring jsearch)
{
    const char* msg = (*env)->GetStringUTFChars(env, jsearch, NULL);

    // 分词，再组装
    char* segments = getSegmentedMsg(msg);
    if (NULL == segments || strlen(segments) == 0)
    {   
        logWarn("FTS word segment: msg is null...", NULL);
        return NULL;
    }
    // logInfo("FTS word segment: segments = ", segments);

    jstring jsegments = (*env)->NewStringUTF(env, segments);

    free(segments);
    (*env)->ReleaseStringUTFChars(env, jsearch, msg);

    logInfo("FTS word segment...", NULL);

    return jsegments;
}
