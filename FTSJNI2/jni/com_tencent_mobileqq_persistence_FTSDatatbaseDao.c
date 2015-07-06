#include <stdio.h>
#include <string.h>

#include "include/com_tencent_mobileqq_persistence_FTSDatatbaseDao.h"
#include "include/utils.h"
#include "include/sqlite3.h"
#include "include/fts3_tokenizer.h"

// dbName和Java层保持一致
#define DB_FILE "/data/data/com.tencent.mobileqq/databases/IndexQQMsg.db"

static sqlite3* db = NULL;

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

    char* sql = "CREATE VIRTUAL TABLE IF NOT EXISTS IndexMsg USING fts3(uin, istroop, uniseq, msg);";
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


jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_insertFTS(JNIEnv* env, jobject thiz, jlong juin, jint jistroop, jlong juniseq, jstring jmsg)
{
    long long uin = (long long) juin;
    int istroop = (int) jistroop;
    long long uniseq = (long long) juniseq;
    const char* msg = (*env)->GetStringUTFChars(env, jmsg, NULL);

    // 分词，再组装
    char* segments = getSegmentedMsg(msg);
    if (NULL == segments || strlen(segments) == 0)
    {
        logWarn("FTS insert: msg is null...", NULL);
        return -1;
    }
    logInfo("FTS insert: segments = ", segments);

    sqlite3_stmt* pStmt = NULL;
    char* zSql = "INSERT INTO IndexMsg(uin, istroop, uniseq, msg) VALUES(?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        logError(sqlite3_errmsg(db), NULL);
        return rc;
    }

    sqlite3_bind_int64(pStmt, 1, uin);
    sqlite3_bind_int(pStmt, 2, istroop);
    sqlite3_bind_int64(pStmt, 3, uniseq);
    sqlite3_bind_text(pStmt, 4, segments, -1, SQLITE_STATIC);
    sqlite3_step(pStmt);
    int ret = sqlite3_finalize(pStmt);

    free(segments);
    (*env)->ReleaseStringUTFChars(env, jmsg, msg);

    logInfo("FTS insert...", NULL);

    return ret;
}

jint Java_com_tencent_mobileqq_persistence_FTSDatatbaseDao_closeFTS(JNIEnv* env, jobject thiz)
{
    if (db != NULL)
    {
        sqlite3_close(db);
    }

    logInfo("FTS close...", NULL);
    return 0;
}
