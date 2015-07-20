#include <string.h>

#include "include/qqcompress.h"
#include "include/sqlite3.h"
#include "include/utils.h"

unsigned char* qq_compress(unsigned char* sour, unsigned long* destlen, unsigned long sourlen)
{
    unsigned long bufflen = 2048;
    unsigned char* buff = (unsigned char*) sqlite3_malloc(bufflen * sizeof(unsigned char));
    if (NULL == buff)
    {
        logError("qq_compress OOM", NULL);
        return NULL;
    }
    memset(buff, 0, bufflen * sizeof(unsigned char));

    int ret = compress(buff, &bufflen, sour, sourlen);
    if (Z_OK != ret)
    {
        switch (ret)
        {
            case Z_MEM_ERROR:
                logError("qq_uncompress failure: ", "there was not enough memory");
                break;
            case Z_BUF_ERROR:
                logError("qq_uncompress failure", "there was not enough room in the output buffer");
                break;
            default:
                break;
        }
        free(buff);
        return NULL;
    }

    if (NULL != destlen)
    {
        *destlen = bufflen;
    }
    return buff;
}

unsigned char* qq_uncompress(unsigned char* sour, unsigned long* destlen, unsigned long sourlen)
{
    unsigned long bufflen = 2048;
    unsigned char* buff = (unsigned char*) sqlite3_malloc(bufflen * sizeof(unsigned char));
    if (NULL == buff)
    {
        logError("qq_uncompress OOM", NULL);
        return NULL;
    }
    memset(buff, 0, bufflen * sizeof(unsigned char));

    int ret = uncompress(buff, &bufflen, sour, sourlen);
    if (Z_OK != ret)
    {
        switch (ret)
        {
            case Z_MEM_ERROR:
                logError("qq_uncompress failure: ", "there was not enough memory");
                break;
            case Z_BUF_ERROR:
                logError("qq_uncompress failure", "there was not enough room in the output buffer");
                break;
            case Z_DATA_ERROR:
                logError("qq_uncompress failure", "the input data was corrupted or incomplete");
                break;
            default:
                break;
        }
        free(buff);
        return NULL;
    }

    if (NULL != destlen)
    {
        *destlen = bufflen;
    }
    return buff;
}
