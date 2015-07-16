#include "include/base64.h"

char *base64_encode(char *s)
{
	char *p = s, *e, *r, *_ret;
	int len = strlen(s);
	unsigned char unit[4], temp[3];
	
	e = s + len;
	len = len / 3 * 4 + 4 + 1;
	r = _ret = (char *)malloc(len);
	
	while (p < e) {
		memset(temp,0,3);
		if (e-p >= 3) {
			memcpy(temp,p,3);
			p += 3;
		} else {
			memcpy(temp,p,e-p);
			p = e;
		}
		
		unit[0] = temp[0] >> 2;
		unit[1] = temp[0] << 6;
		unit[1] = (unit[1]>>2) | (temp[1]>>4);
		unit[2] = temp[1] << 4;
		unit[2] = (unit[2]>>2) | (temp[2]>>6);
		unit[3] = temp[2] << 2;
		unit[3] = unit[3] >> 2;
		*r++ = B64[unit[0]];
		*r++ = B64[unit[1]];
		*r++ = (unit[2] ? B64[unit[2]] : '=');
		*r++ = (unit[3] ? B64[unit[3]] : '=');
	}
	*r = 0;
	
	#if RETURNMALLOC == 0
	strcpy(s,_ret);
	free(_ret);
	_ret = s;
	#endif //RETURNMALLOC
	
	return _ret;
}

char *base64_decode(char *s)
{
	char *p = s, *e, *r, *_ret;
	int len = strlen(s);
	unsigned char i, unit[4];
	
	e = s + len;
	len = len / 4 * 3 + 1;
	r = _ret = (char *)malloc(len);
	
	while (p < e) {
		memcpy(unit,p,4);
		if (unit[3] == '=')
			unit[3] = 0;
		if (unit[2] == '=')
			unit[2] = 0;
		p += 4;
		
		for (i=0; unit[0]!=B64[i] && i<64; i++);
		unit[0] = i==64 ? 0 : i;
		for (i=0; unit[1]!=B64[i] && i<64; i++);
		unit[1] = i==64 ? 0 : i;
		for (i=0; unit[2]!=B64[i] && i<64; i++);
		unit[2] = i==64 ? 0 : i;
		for (i=0; unit[3]!=B64[i] && i<64; i++);
		unit[3] = i==64 ? 0 : i;
		*r++ = (unit[0]<<2) | (unit[1]>>4);
		*r++ = (unit[1]<<4) | (unit[2]>>2);
		*r++ = (unit[2]<<6) | unit[3];
	}
	*r = 0;
	
	#if RETURNMALLOC == 0
	strcpy(s,_ret);
	free(_ret);
	_ret = s;
	#endif //RETURNMALLOC
	
	return _ret;
}
