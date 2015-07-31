
// BASE64压缩接口

#ifndef _INCLUDE_BASE64_H
#define _INCLUDE_BASE64_H

#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

// 标准Base64编码表
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char* decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

unsigned char* base64_encode(unsigned char* data, int input_length, int* output_length);

unsigned char* base64_decode(unsigned char* data, int input_length, int* output_length);

void build_decoding_table();


void base64_cleanup();

#endif  // _INCLUDE_BASE64_H
