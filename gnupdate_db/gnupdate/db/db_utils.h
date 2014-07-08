/**
 * @file db_utils.h Utility functions
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
#ifndef _GNUPDATEDB_UTILS_H_
#define _GNUPDATEDB_UTILS_H_

#include <stdio.h>

/**
 * Returns 1 byte (8 bits) from a buffer.
 *
 * @param buffer  The buffer.
 * @param counter A pointer to the current offset.
 *
 * @return The byte.
 */
unsigned char gdbGet8(const unsigned char *buffer, int *counter);

/**
 * Returns 2 bytes (16 bits) from a buffer.
 *
 * @param buffer  The buffer.
 * @param counter A pointer to the current offset.
 *
 * @return A short value (2 bytes).
 */
unsigned short gdbGet16(const unsigned char *buffer, int *counter);

/**
 * Returns 4 bytes (32 bits) from a buffer.
 *
 * @param buffer  The buffer.
 * @param counter A pointer to the current offset.
 *
 * @return A long value (4 bytes).
 */
unsigned long gdbGet32(const unsigned char *buffer, int *counter);

/**
 * Writes 1 byte (8 bits) to a buffer.
 *
 * @param buffer  The buffer.
 * @param counter A pointer to the current offset.
 * @param c       The character to write.
 */
void gdbPut8(unsigned char *buffer, int *counter, unsigned char c);

/**
 * Writes 2 bytes (16 bits) to a buffer.
 *
 * @param buffer  The buffer.
 * @param counter A pointer to the current offset.
 * @param s       The short to write.
 */
void gdbPut16(unsigned char *buffer, int *counter, unsigned short s);

/**
 * Writes 4 bytes (32 bits) to a buffer.
 *
 * @param buffer  The buffer.
 * @param counter A pointer to the current offset.
 * @param l       The long to write.
 */
void gdbPut32(unsigned char *buffer, int *counter, unsigned long l);

/**
 * Pads data in a file.
 *
 * @param fp    The file pointer.
 * @param count The number of bytes to pad.
 */
void gdbPad(FILE *fp, long count);

/**
 * Compresses a string using prefix and suffix compression.
 *
 * @param base    The base string.
 * @param baseLen The base length.
 * @param key     The key to compress.
 * @param keyLen  The key length.
 * @param outKey  The destination key.
 * @param outLen  The destination key length.
 */
void gdbCompressString(const char *base, unsigned short baseLen,
					   const char *key, unsigned short keyLen,
					   char **outKey, unsigned short *outLen);

/**
 * Uncompresses a string.
 *
 * @param base    The base string.
 * @param baseLen The base length.
 * @param key     The key.
 * @param keyLen  The key length.
 * @param outKey  The destination key.
 * @param outLen  The destination key length.
 */
void gdbUncompressString(const char *base, unsigned short baseLen,
						 const char *key, unsigned short keyLen,
						 char **outKey, unsigned short *outLen);

#endif /* _GNUPDATEDB_UTILS_H_ */

