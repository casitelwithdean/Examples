#ifndef __hmacsha1_h
#define __hmacsha1_h

#include <stdlib.h>
#include <stdio.h>

void hmac_sha1x(unsigned char *key,int key_length,unsigned char *data,
									int data_length,unsigned char *digest);

#endif

