/**
 * @file hashtable.h HashTable implementation
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
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

typedef struct _GdbBucket    GdbBucket;    /**< A bucket.    */
typedef struct _GdbHashTable GdbHashTable; /**< A HashTable. */

#include "db.h"

#define HASHTABLE_BLOCK_MULTIPLE 16

/** @name Data Types */
/*@{*/
#define GDB_HT_RAW       0x00  /**< Raw data.        */
#define GDB_HT_STRING    0x01  /**< String.          */
#define GDB_HT_BOOLEAN   0x02  /**< Boolean (0 or 1) */
#define GDB_HT_LONG      0x03  /**< Long integer.    */
#define GDB_HT_OFFSET    0x04  /**< File offset      */

#define GDB_HT_MIN_TYPE  GDB_HT_RAW
#define GDB_HT_MAX_TYPE  GDB_HT_OFFSET
/*@}*/

/**
 * A bucket.
 */
struct _GdbBucket
{
	unsigned short  key;   /**< The key.              */

	unsigned char   type;  /**< Data type.            */
	unsigned short  size;  /**< The size of the data. */
	void           *data;  /**< The data.             */

	GdbBucket      *next;  /**< The next bucket.      */
};

/**
 * A hashtable.
 */
struct _GdbHashTable
{
	GdbBlock *block;            /**< The associated block.           */

	unsigned char  bucketCount; /**< The number of buckets.          */
	unsigned short itemCount;   /**< The number of items.            */

	GdbBucket    **buckets;     /**< The buckets.                    */
	unsigned char *counts;      /**< Number of items in each bucket. */
};

/**
 * Reads a HashTable from a buffer.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block  The block.
 * @param buffer The buffer to read from.
 * @param extra  NULL.
 * 
 * @return A GdbHashTable, or NULL on error.
 */
void *htReadBlock(GdbBlock *block, const char *buffer, void *extra);

/**
 * Writes a HashTable to a buffer.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block  The block.
 * @param buffer The returned buffer.
 * @param size   The returned buffer size.
 */
void htWriteBlock(GdbBlock *block, char **buffer, unsigned long *size);

/**
 * Creates a HashTable block.
 *
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param block The block.
 * @param extra NULL
 *
 * @return A GdbHashTable structure.
 */
void *htCreateBlock(GdbBlock *block, void *extra);

/**
 * Destroys a GdbHashTable structure in memory.
 * 
 * This is meant to be called by the block functions. Don't call this
 * directly.
 *
 * @param data The GdbHashTable to destroy.
 */
void htDestroyBlock(void *data);

/**
 * Opens a hashtable from inside a database.
 *
 * @param db     The active database.
 * @param offset The offset of the hashtable
 *
 * @return An GdbHashTable structure.
 */
GdbHashTable *htOpen(GDatabase *db, offset_t offset);

/**
 * Creates a hashtable inside a database.
 *
 * @param db     The active database.
 *
 * @return An GdbHashTable structure.
 */
GdbHashTable *htCreate(GDatabase *db);

/**
 * Adds a key and value to the hashtable.
 *
 * @param table  The hashtable.
 * @param key    The key.
 * @param data   The data to store.
 * @param type   The type of data.
 * @param size   The size of the data. If type is not a string or raw data,
 *               this value will be ignored.
 */
void htAdd(GdbHashTable *table, unsigned short key, const void *data,
		   unsigned char type, unsigned short size);

/**
 * Removes a key and its associated data from a hashtable.
 *
 * @param table The hashtable.
 * @param key   The key.
 *
 * @return 1 if removed, 0 if not found or invalid parameter.
 */
char htRemove(GdbHashTable *table, unsigned short key);

/**
 * Gets the data associated with the specified key.
 *
 * @param table The hashtable.
 * @param key   The key associated with the data.
 * @param size  The returned size of the data.
 * @param type  The returned type of the data.
 *
 * @return The data associated with @a key, or NULL if not found.
 */
const void *htGetData(GdbHashTable *table, unsigned short key,
					  unsigned short *size, unsigned char *type);

/**
 * Adds a key and a string to the hashtable.
 * 
 * This is a convenience wrapper for htAdd().
 *
 * @param table The hashtable.
 * @param key   The key.
 * @param value The string to add.
 */
void htAddString(GdbHashTable *table, unsigned short key, const char *value);

/**
 * Adds a key and a long value to the hashtable.
 *
 * This is a convenience wrapper for htAdd().
 *
 * @param table The hashtable.
 * @param key   The key.
 * @param value The long value to add.
 */
void htAddLong(GdbHashTable *table, unsigned short key, long value);

/**
 * Adds a key and an offset to the hashtable.
 *
 * This is a convenience wrapper for htAdd().
 *
 * @param table The hashtable.
 * @param key   The key.
 * @param value The offset to add.
 */
void htAddOffset(GdbHashTable *table, unsigned short key, offset_t value);

/**
 * Gets the string associated with the specified key.
 *
 * This is a convenience wrapper for htGetData().
 * 
 * The returned value must be freed when no longer needed.
 *
 * @param table The hashtable.
 * @param key   The key associated with the string.
 * 
 * @return The string associated with @a key, or NULL if not found
 *         (or invalid data type).
 */
char *htGetString(GdbHashTable *table, unsigned short key);

/**
 * Gets the long value associated with the specified key.
 *
 * This is a convenience wrapper for htGetData().
 *
 * @param table The hashtable.
 * @param key   The key associated with the long value.
 *
 * @return The long value associated with @a key, or 0 if not found
 *         (or invalid data type).
 */
long htGetLong(GdbHashTable *table, unsigned short key);

/**
 * Gets the offset associated with the specified key.
 *
 * This is a convenience wrapper for htGetData().
 *
 * @param table The hashtable.
 * @param key   The key associated with the offset.
 *
 * @return The offset associated with @a key, or 0 if not found
 *         (or invalid data type).
 */
offset_t htGetOffset(GdbHashTable *table, unsigned short key);

#endif /* _HASHTABLE_H_ */
