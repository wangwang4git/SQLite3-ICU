#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/com_tencent_mobileqq_persistence_fts_FTSDatatbase.h"
#include "include/sqlite3.h"
#include "include/log-generic.h"
#include "include/utils.h"


// dbName和Java层保持一致
#define DB_FILE "/data/data/com.tencent.mobileqq/databases/%s-IndexQQMsg.db"

static sqlite3* db = NULL;
static sqlite3_stmt* stmt_insert = NULL;

jint Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_initFTS(JNIEnv* env, jobject thiz, jstring juin)
{
    char dbname[128];
    const char* uin = (*env)->GetStringUTFChars(env, juin, NULL);
    sprintf(dbname, DB_FILE, uin);
    (*env)->ReleaseStringUTFChars(env, juin, uin);

    int errCode = sqlite3_open_v2(dbname, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (SQLITE_OK != errCode)
    {
        LOGE("Can't open database %s, %s", dbname, sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    errCode = sqlite3_exec(db, "PRAGMA cache_size=4000;", NULL, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        LOGE("Can't set PRAGMA cache_size = 4000, %s", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    errCode = sqlite3_create_function(db, "qqcompress", 1, SQLITE_UTF8, NULL, qqcompress, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        LOGE("Can't create function, %s", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    errCode = sqlite3_create_function(db, "qquncompress", 1, SQLITE_UTF8, NULL, qquncompress, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        LOGE("Can't create function, %s", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    char* sql = "CREATE VIRTUAL TABLE IF NOT EXISTS IndexContent USING fts4(type, content, contentindex, oid, ext1, ext2, ext3, exts, compress=qqcompress, uncompress=qquncompress);";
    // char* sql = "CREATE VIRTUAL TABLE IF NOT EXISTS IndexContent USING fts4(type, content, contentindex, oid, ext1, ext2, ext3, exts);";
    errCode = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (SQLITE_OK != errCode)
    {
        LOGE("Can't create virtual table IndexContent, %s", sqlite3_errmsg(db));

        sqlite3_close(db);
        return errCode;
    }

    ecode_init();

    LOGI("FTS init, %s", dbname);

    return 0;
}

jint Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_closeFTS(JNIEnv* env, jobject thiz)
{
    if (db != NULL)
    {
        sqlite3_close(db);
        db = NULL;
    }

    if (stmt_insert != NULL)
    {
        sqlite3_finalize(stmt_insert);
        stmt_insert = NULL;
    }

    ecode_release();

    LOGI("FTS close");
    return 0;
}

jint Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_insertFTS(JNIEnv* env, jobject thiz, jobject entity)
{
    // 获取FTSEntity class类
    jclass entity_clazz = (*env)->GetObjectClass(env, entity);

    // 获取FTSEntity对象实例变量mType
    jfieldID entity_type = (*env)->GetFieldID(env, entity_clazz, "mType", "I");
    jint jtype = (*env)->GetIntField(env, entity, entity_type);

    // 获取FTSEntity对象实例变量mContent
    jfieldID entity_content = (*env)->GetFieldID(env, entity_clazz, "mContent", "Ljava/lang/String;");
    jstring jcontent = (jstring)(*env)->GetObjectField(env, entity, entity_content);

    // 获取FTSEntity对象实例变量mContentIndex
    jfieldID entity_contentIndex = (*env)->GetFieldID(env, entity_clazz, "mContentIndex", "Ljava/lang/String;");
    jstring jcontentIndex = (jstring)(*env)->GetObjectField(env, entity, entity_contentIndex);

    // 获取FTSEntity对象实例变量mOId
    jfieldID entity_oId = (*env)->GetFieldID(env, entity_clazz, "mOId", "J");
    jlong joId = (*env)->GetLongField(env, entity, entity_oId);

    // 获取FTSEntity对象实例变量mExt1
    jfieldID entity_ext1 = (*env)->GetFieldID(env, entity_clazz, "mExt1", "Ljava/lang/String;");
    jstring jext1 = (jstring)(*env)->GetObjectField(env, entity, entity_ext1);

    // 获取FTSEntity对象实例变量mExt2
    jfieldID entity_ext2 = (*env)->GetFieldID(env, entity_clazz, "mExt2", "Ljava/lang/String;");
    jstring jext2 = (jstring)(*env)->GetObjectField(env, entity, entity_ext2);

    // 获取FTSEntity对象实例变量mExt3
    jfieldID entity_ext3 = (*env)->GetFieldID(env, entity_clazz, "mExt3", "Ljava/lang/String;");
    jstring jext3 = (jstring)(*env)->GetObjectField(env, entity, entity_ext3);

    // 获取FTSEntity对象实例变量mExts
    jfieldID entity_exts = (*env)->GetFieldID(env, entity_clazz, "mExts", "[B");
    jbyteArray jexts = (jbyteArray)(*env)->GetObjectField(env, entity, entity_exts);

    int type = (int) jtype;
    const char* content = (*env)->GetStringUTFChars(env, jcontent, NULL);
    const char* contentIndex = (*env)->GetStringUTFChars(env, jcontentIndex, NULL);
    long long oId = (long long)joId;
    const char* ext1 = (*env)->GetStringUTFChars(env, jext1, NULL);
    const char* ext2 = (*env)->GetStringUTFChars(env, jext2, NULL);
    const char* ext3 = (*env)->GetStringUTFChars(env, jext3, NULL);
    unsigned char* exts = (*env)->GetByteArrayElements(env, jexts, NULL);
    int extslen = (int)(*env)->GetArrayLength(env, jexts);

    if (NULL == stmt_insert)
    {
        // DDL CREATE VIRTUAL TABLE IF NOT EXISTS IndexContent USING fts4(type, content, contentindex, oid, ext1, ext2, ext3, exts);
        char* zSql = "INSERT INTO IndexContent(type, content, contentindex, oid, ext1, ext2, ext3, exts) VALUES(?, ?, ?, ?, ?, ?, ?, ?);";
        int rc = sqlite3_prepare_v2(db, zSql, strlen(zSql), &stmt_insert, 0);
        if (rc != SQLITE_OK)
        {
            LOGE("Can't prepare stmt_insert, %s", sqlite3_errmsg(db));

            stmt_insert = NULL;

            return rc;
        }
    }

    sqlite3_bind_int(stmt_insert, 1, type);
    sqlite3_bind_text(stmt_insert, 2, content, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 3, contentIndex, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt_insert, 4, oId);
    sqlite3_bind_text(stmt_insert, 5, ext1, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 6, ext2, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 7, ext3, -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt_insert, 8, exts, extslen, SQLITE_STATIC);

    sqlite3_step(stmt_insert);
    sqlite3_reset(stmt_insert);

    (*env)->ReleaseStringUTFChars(env, jcontent, content);
    (*env)->ReleaseStringUTFChars(env, jcontentIndex, contentIndex);
    (*env)->ReleaseStringUTFChars(env, jext1, ext1);
    (*env)->ReleaseStringUTFChars(env, jext2, ext2);
    (*env)->ReleaseStringUTFChars(env, jext3, ext3);
    (*env)->ReleaseByteArrayElements(env, jexts, exts, 0);

    // 避免 local reference table overflow (max=512) 错误
    (*env)->DeleteLocalRef(env, entity_clazz);
    (*env)->DeleteLocalRef(env, entity_type);
    (*env)->DeleteLocalRef(env, entity_content);
    (*env)->DeleteLocalRef(env, jcontent);
    (*env)->DeleteLocalRef(env, entity_contentIndex);
    (*env)->DeleteLocalRef(env, jcontentIndex);
    (*env)->DeleteLocalRef(env, entity_oId);
    (*env)->DeleteLocalRef(env, entity_ext1);
    (*env)->DeleteLocalRef(env, jext1);
    (*env)->DeleteLocalRef(env, entity_ext2);
    (*env)->DeleteLocalRef(env, jext2);
    (*env)->DeleteLocalRef(env, entity_ext3);
    (*env)->DeleteLocalRef(env, jext3);
    (*env)->DeleteLocalRef(env, entity_exts);
    (*env)->DeleteLocalRef(env, jexts);

    LOGI("FTS insert");

    return 0;
}

jint Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_insertFTSWithTrans(JNIEnv* env, jobject thiz, jobject entitylist)
{
    LOGI("FTS insert trans start");

    // 获取ArrayList class类
    jclass list_clazz = (*env)->GetObjectClass(env, entitylist);

    // 获取ArrayList类get函数ID
    jmethodID list_get = (*env)->GetMethodID(env, list_clazz , "get", "(I)Ljava/lang/Object;");

    // 获取ArrayList类size函数ID
    jmethodID list_size = (*env)->GetMethodID(env, list_clazz , "size", "()I");

    jint list_len = (*env)->CallIntMethod(env, entitylist, list_size);

    if (list_len > 0)
    {
        int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            LOGE("Can't begin transcation, %s", sqlite3_errmsg(db));
        }
    }

    int i = 0;
    for (; i < list_len; ++i)
    {
        jobject fts_obj = (*env)->CallObjectMethod(env, entitylist, list_get, i);

        Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_insertFTS(env, thiz, fts_obj);

        (*env)->DeleteLocalRef(env, fts_obj);
    }

    if (list_len > 0)
    {
        int rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            LOGE("Can't commit transcation, %s", sqlite3_errmsg(db));
        }
    }

    // 避免 local reference table overflow (max=512) 错误
    (*env)->DeleteLocalRef(env, list_clazz);
    (*env)->DeleteLocalRef(env, list_get);
    (*env)->DeleteLocalRef(env, list_size);

    LOGI("FTS insert trans end");

    return 0;
}

jobject Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_queryFTSEntities(JNIEnv* env, jobject thiz, jstring jsql, jobjectArray jstringarray, jstring jclasspath)
{
    // 获取ArrayList class类
    jclass list_clazz = (*env)->FindClass(env, "java/util/ArrayList");

    // 获取ArrayList类构造函数ID
    jmethodID list_init = (*env)->GetMethodID(env, list_clazz , "<init>", "()V");

    // 获取ArrayList类add函数ID
    jmethodID list_add  = (*env)->GetMethodID(env, list_clazz, "add", "(Ljava/lang/Object;)Z");

    // 获取FTSEntity class类
    const char* classpath = (*env)->GetStringUTFChars(env, jclasspath, NULL);
    jclass fts_clazz = (*env)->FindClass(env, classpath);
    (*env)->ReleaseStringUTFChars(env, jclasspath, classpath);

    // 获取FTSEntity构造函数ID
    jmethodID fts_init = (*env)->GetMethodID(env, fts_clazz , "<init>", "()V");

    // 搜索逻辑
    sqlite3_stmt* stmt_query = NULL;
    const char* sql = (*env)->GetStringUTFChars(env, jsql, NULL);
    int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt_query, NULL);
    (*env)->ReleaseStringUTFChars(env, jsql, sql);
    if (rc != SQLITE_OK)
    {
        LOGE("Can't prepare stmt_query, %s", sqlite3_errmsg(db));
        stmt_query = NULL;

        (*env)->DeleteLocalRef(env, list_clazz);
        (*env)->DeleteLocalRef(env, list_init);
        (*env)->DeleteLocalRef(env, list_add);
        (*env)->DeleteLocalRef(env, fts_clazz);
        (*env)->DeleteLocalRef(env, fts_init);

        return NULL;
    }

    int ncols = sqlite3_column_count(stmt_query);
    // SQL查询语句，SELECT字段：type, content, oid, ext1, ext2, ext3, exts
    if (ncols != 7)
    {
        LOGW("FTS queryFTSEntities: ncols != 7");

        (*env)->DeleteLocalRef(env, list_clazz);
        (*env)->DeleteLocalRef(env, list_init);
        (*env)->DeleteLocalRef(env, list_add);
        (*env)->DeleteLocalRef(env, fts_clazz);
        (*env)->DeleteLocalRef(env, fts_init);
        return NULL;
    }


    rc = sqlite3_step(stmt_query);
    if (rc == SQLITE_DONE)
    {
        LOGI("queryFTSEntities none");

        (*env)->DeleteLocalRef(env, list_clazz);
        (*env)->DeleteLocalRef(env, list_init);
        (*env)->DeleteLocalRef(env, list_add);
        (*env)->DeleteLocalRef(env, fts_clazz);
        (*env)->DeleteLocalRef(env, fts_init);
        return NULL;
    }
    else if (rc != SQLITE_ROW)
    {
        LOGE("Can't queryFTSEntities, %s", sqlite3_errmsg(db));

        (*env)->DeleteLocalRef(env, list_clazz);
        (*env)->DeleteLocalRef(env, list_init);
        (*env)->DeleteLocalRef(env, list_add);
        (*env)->DeleteLocalRef(env, fts_clazz);
        (*env)->DeleteLocalRef(env, fts_init);
        return NULL;
    }

    // 构造ArrayList对象
    jobject list_obj = (*env)->NewObject(env, list_clazz , list_init);

    jsize len = (*env)->GetArrayLength(env, jstringarray);
    char** pstr = (char**) malloc(len * sizeof(char*));
    int i = 0;
    for (; i < len; i++)
    {
        jstring jstr = (*env)->GetObjectArrayElement(env, jstringarray, i);
        pstr[i] = (char*)(*env)->GetStringUTFChars(env, jstr, 0);

        // 避免 local reference table overflow (max=512) 错误
        (*env)->DeleteLocalRef(env, jstr);
    }

    while (rc == SQLITE_ROW)
    {
        int type = sqlite3_column_int(stmt_query, 0);
        const char* content = sqlite3_column_text(stmt_query, 1);

        // OR逻辑，后续优化
        // 后续fix，英文大小写的坑
        // --------
        int ret = -1;
        int j = 0;
        for (j = 0; j < len; ++j)
        {
            if (strstr(content, pstr[j]) != NULL)
            {
                ret = 0;
                break;
            }
        }
        if (ret == -1)
        {
            rc = sqlite3_step(stmt_query);
            continue;
        }
        // --------

        // 注意：java long和c/c++ long，不一样，小心掉坑里！！
        long long oid = sqlite3_column_int64(stmt_query, 2);
        const char* ext1 = sqlite3_column_text(stmt_query, 3);
        const char* ext2 = sqlite3_column_text(stmt_query, 4);
        const char* ext3 = sqlite3_column_text(stmt_query, 5);
        int exts_len = sqlite3_column_bytes(stmt_query, 6);
        unsigned char* exts = sqlite3_malloc(exts_len * sizeof(unsigned char));
        memcpy(exts, sqlite3_column_blob(stmt_query, 6), exts_len);

        // 构造搜索结果集
        jstring jcontent = (*env)->NewStringUTF(env, content);
        jstring jext1 = (*env)->NewStringUTF(env, ext1);
        jstring jext2 = (*env)->NewStringUTF(env, ext2);
        jstring jext3 = (*env)->NewStringUTF(env, ext3);
        // unsigned char* 转 jbyteArray
        jbyteArray exts_array = (*env)->NewByteArray(env, (jsize)exts_len);
        unsigned char* byteArrayElements = (*env)->GetByteArrayElements(env, exts_array, NULL);
        memcpy(byteArrayElements, exts, exts_len);

        // 构造FTSEntity对象
        jobject fts_obj = (*env)->NewObject(env, fts_clazz, fts_init);

        // 获取FTSEntity对象实例变量mType
        jfieldID entity_type = (*env)->GetFieldID(env, fts_clazz, "mType", "I");
        (*env)->SetIntField(env, fts_obj, entity_type, type);

        // 获取FTSEntity对象实例变量mContent
        jfieldID entity_content = (*env)->GetFieldID(env, fts_clazz, "mContent", "Ljava/lang/String;");
        (*env)->SetObjectField(env, fts_obj, entity_content, jcontent);

        // 获取FTSEntity对象实例变量mOId
        jfieldID entity_oId = (*env)->GetFieldID(env, fts_clazz, "mOId", "J");
        (*env)->SetLongField(env, fts_obj, entity_oId, oid);

        // 获取FTSEntity对象实例变量mExt1
        jfieldID entity_ext1 = (*env)->GetFieldID(env, fts_clazz, "mExt1", "Ljava/lang/String;");
        (*env)->SetObjectField(env, fts_obj, entity_ext1, jext1);

        // 获取FTSEntity对象实例变量mExt2
        jfieldID entity_ext2 = (*env)->GetFieldID(env, fts_clazz, "mExt2", "Ljava/lang/String;");
        (*env)->SetObjectField(env, fts_obj, entity_ext2, jext2);

        // 获取FTSEntity对象实例变量mExt3
        jfieldID entity_ext3 = (*env)->GetFieldID(env, fts_clazz, "mExt3", "Ljava/lang/String;");
        (*env)->SetObjectField(env, fts_obj, entity_ext3, jext3);

        // 获取FTSEntity对象实例变量mExts
        jfieldID entity_exts = (*env)->GetFieldID(env, fts_clazz, "mExts", "[B");
        (*env)->SetObjectField(env, fts_obj, entity_exts, exts_array);

        (*env)->CallBooleanMethod(env, list_obj, list_add, fts_obj);

        // 避免 local reference table overflow (max=512) 错误
        (*env)->DeleteLocalRef(env, entity_type);
        (*env)->DeleteLocalRef(env, entity_content);
        (*env)->DeleteLocalRef(env, entity_oId);
        (*env)->DeleteLocalRef(env, entity_ext1);
        (*env)->DeleteLocalRef(env, entity_ext2);
        (*env)->DeleteLocalRef(env, entity_ext3);
        (*env)->DeleteLocalRef(env, entity_exts);

        (*env)->DeleteLocalRef(env, fts_obj);

        (*env)->DeleteLocalRef(env, jcontent);
        (*env)->DeleteLocalRef(env, jext1);
        (*env)->DeleteLocalRef(env, jext2);
        (*env)->DeleteLocalRef(env, jext3);
        (*env)->DeleteLocalRef(env, exts_array);

        sqlite3_free(exts);
        (*env)->ReleaseByteArrayElements(env, exts_array, byteArrayElements, 0);

        rc = sqlite3_step(stmt_query);
    }

    if (stmt_query != NULL)
    {
        sqlite3_reset(stmt_query);
        sqlite3_finalize(stmt_query);
        stmt_query = NULL;
    }

    (*env)->DeleteLocalRef(env, list_clazz);
    (*env)->DeleteLocalRef(env, list_init);
    (*env)->DeleteLocalRef(env, list_add);
    (*env)->DeleteLocalRef(env, fts_clazz);
    (*env)->DeleteLocalRef(env, fts_init);
    (*env)->DeleteLocalRef(env, list_obj);

    LOGI("FTS query");

    return list_obj;
}

jint Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_deleteFTS(JNIEnv* env, jobject thiz, jstring jsql)
{
    const char* sql = (*env)->GetStringUTFChars(env, jsql, NULL);
    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    (*env)->ReleaseStringUTFChars(env, jsql, sql);
    if (rc != SQLITE_OK)
    {
        LOGE("Can't delete, %s", sqlite3_errmsg(db));
    }

    LOGI("FTS delete");

    return rc;
}

jint Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_deleteFTSWithTrans(JNIEnv* env, jobject thiz, jobject jsqllist)
{
    LOGI("FTS delete trans start");

    // 获取ArrayList class类
    jclass list_clazz = (*env)->GetObjectClass(env, jsqllist);

    // 获取ArrayList类get函数ID
    jmethodID list_get = (*env)->GetMethodID(env, list_clazz , "get", "(I)Ljava/lang/Object;");

    // 获取ArrayList类size函数ID
    jmethodID list_size = (*env)->GetMethodID(env, list_clazz , "size", "()I");

    jint list_len = (*env)->CallIntMethod(env, jsqllist, list_size);

    if (list_len > 0)
    {
        int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            LOGE("Can't begin transcation, %s", sqlite3_errmsg(db));
        }
    }

    int i = 0;
    for (; i < list_len; ++i)
    {
        jstring jsql = (jstring)(*env)->CallObjectMethod(env, jsqllist, list_get, i);

        Java_com_tencent_mobileqq_persistence_fts_FTSDatatbase_deleteFTS(env, thiz, jsql);

        (*env)->DeleteLocalRef(env, jsql);
    }

    if (list_len > 0)
    {
        int rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        if (rc != SQLITE_OK)
        {
            LOGE("Can't commit transcation, %s", sqlite3_errmsg(db));
        }
    }

    // 避免 local reference table overflow (max=512) 错误
    (*env)->DeleteLocalRef(env, list_clazz);
    (*env)->DeleteLocalRef(env, list_get);
    (*env)->DeleteLocalRef(env, list_size);

    LOGI("FTS delete trans end");

    return 0;
}

