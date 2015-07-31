#include "include/base64.h"

unsigned char* base64_encode(unsigned char* data, int input_length, int* output_length)
{
    *output_length = 4 * ((input_length + 2) / 3);

    unsigned char* encoded_data = (unsigned char*) sqlite3_malloc(*output_length * sizeof(unsigned char));
    if (encoded_data == NULL)
   	{
   		return NULL;
   	}

   	int i = 0, j = 0;
    for (; i < input_length;)
    {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; ++i)
    {
        encoded_data[*output_length - 1 - i] = '=';
    }

    return encoded_data;
}


unsigned char* base64_decode(unsigned char* data, int input_length, int* output_length)
{
    if (decoding_table == NULL)
    {
    	build_decoding_table();
    }

    if (input_length % 4 != 0)
    {
    	return NULL;
    }

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=')
    {
    	(*output_length)--;
    }
    if (data[input_length - 2] == '=')
	{
		(*output_length)--;
	}

    unsigned char* decoded_data = (unsigned char*) sqlite3_malloc(*output_length * sizeof(unsigned char));
    if (decoded_data == NULL)
    {
    	return NULL;
    }

	int i = 0, j = 0;
    for (; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                            + (sextet_b << 2 * 6)
                            + (sextet_c << 1 * 6)
                            + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}


void build_decoding_table()
{
    decoding_table = (unsigned char*) malloc(256 * sizeof(unsigned char));

	int i = 0;
    for (; i < 64; i++)
    {
    	decoding_table[(unsigned char) encoding_table[i]] = i;
    }
}


void base64_cleanup()
{
	if (decoding_table != NULL)
	{
		free(decoding_table);
	}
}
