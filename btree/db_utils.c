/**
 * @file db_utils.c Utility functions
 *
 * @Copyright (C) 1999-2004 The GNUpdate Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#include "db_internal.h"

unsigned char
gdbGet8(const unsigned char *buffer, int *counter)
{
	/* char i = (buffer[*counter] & 0xFF); */
	char i = buffer[*counter];

	(*counter)++;

	return i;
}

unsigned short
gdbGet16(const unsigned char *buffer, int *counter)
{
	unsigned short s;

	memcpy(&s, buffer + *counter, sizeof(unsigned short));
	s = ntohs(s);

	*counter += sizeof(unsigned short);

	return s;

#if 0
	return ntohs(((unsigned short)gdbGet8(buffer, counter) |
				  ((unsigned short)gdbGet8(buffer, counter) << 8)));
#endif
}

unsigned long
gdbGet32(const unsigned char *buffer, int *counter)
{
	unsigned long l;

	memcpy(&l, buffer + *counter, sizeof(unsigned long));
	l = ntohl(l);

	*counter += sizeof(unsigned long);

	return l;

#if 0
	return (((unsigned long)gdbGet16(buffer, counter) |
			 ((unsigned long)gdbGet16(buffer, counter) << 16)));
#endif
}

void
gdbPut8(unsigned char *buffer, int *counter, unsigned char c)
{
	buffer[*counter] = c;

	(*counter)++;
}

void
gdbPut16(unsigned char *buffer, int *counter, unsigned short s)
{
	s = htons(s);

	memcpy(buffer + *counter, &s, sizeof(unsigned short));
	*counter += sizeof(unsigned short);

#if 0
	gdbPut8(buffer, counter, (unsigned char)s);
	gdbPut8(buffer, counter, (unsigned char)(s >> 8));
#endif
}

void
gdbPut32(unsigned char *buffer, int *counter, unsigned long l)
{
	l = htonl(l);

	memcpy(buffer + *counter, &l, sizeof(unsigned long));
	*counter += sizeof(unsigned long);

#if 0
	gdbPut16(buffer, counter, (unsigned short)l);
	gdbPut16(buffer, counter, (unsigned short)(l >> 16));
#endif
}

void
gdbPad(FILE *fp, long count)
{
	char *c;

	if (fp == NULL || count == 0)
		return;
	
	MEM_CHECK(c = (char *)malloc(count));
	memset(c, 0, count);

	fwrite(c, 1, count, fp);

	free(c);
}

void
gdbCompressString(const char *base, unsigned short baseLen,
				  const char *key, unsigned short keyLen,
				  char **outKey, unsigned short *outLen)
{
	unsigned short preLen, sufLen;
	unsigned short maxLen, minLen;
	unsigned short newLen;
	const char *c1, *c2;
	char *newKey;

	if (base == NULL || baseLen == 0 || key == NULL || keyLen == 0 ||
		outKey == NULL || outLen == NULL)
	{
		return;
	}

	sufLen = 0;

	maxLen = (baseLen > keyLen ? baseLen : keyLen);
	minLen = (baseLen < keyLen ? baseLen : keyLen);

	maxLen = (maxLen < 255 ? maxLen : 255);
	minLen = (minLen < 255 ? minLen : 255);

	/*
	 * Get the prefix length.
	 *
	 * Okay, so it's a little hacky :) It should be more efficient though.
	 */
	for (c1 = base, c2 = key, preLen = 0;
		 (preLen < minLen) && (*c1 == *c2);
		 c1++, c2++, preLen++)
		;

	newLen = keyLen - preLen + 1;

	MEM_CHECK(newKey = (char *)malloc(newLen));
	
	newKey[0] = preLen;
	strncpy(newKey + 1, key + preLen, keyLen - preLen);

	*outKey = newKey;
	*outLen = newLen;
}

void
gdbUncompressString(const char *base, unsigned short baseLen,
					const char *key, unsigned short keyLen,
					char **outKey, unsigned short *outLen)
{
	unsigned short preLen, newLen;
	char *newKey;

	if (base == NULL || baseLen == 0 || key == NULL || keyLen == 0 ||
		outKey == NULL || outLen == NULL)
	{
		return;
	}

	preLen = key[0];

	newLen = preLen + keyLen - 1;

	MEM_CHECK(newKey = (char *)malloc(newLen));

	strncpy(newKey, base, preLen);
	strncpy(newKey + preLen, key + 1, keyLen - 1);

	*outKey = newKey;
	*outLen = newLen;
}

