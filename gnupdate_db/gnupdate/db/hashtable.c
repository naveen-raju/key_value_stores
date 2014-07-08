/**
 * @file hashtable.c GdbHashTable implementation
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

#define HASHTABLE_HEADER_SIZE \
	(sizeof(char) + sizeof(unsigned short))

static int typeSizes[] = { -1, -1, 1, sizeof(long), sizeof(long) };

static unsigned long
__getDataSize(GdbHashTable *table)
{
	unsigned short size;
	GdbBucket *bucket;
	int i;

	size = table->bucketCount * sizeof(char) +
	       table->itemCount   * (sizeof(short) + sizeof(char));
	
	/* Get the sizes of the data. */
	for (i = 0; i < table->bucketCount; i++)
	{
		for (bucket = table->buckets[i];
			 bucket != NULL;
			 bucket = bucket->next)
		{
			if (typeSizes[bucket->type] == -1)
				size += sizeof(unsigned short);
			
			size += bucket->size;
		}
	}

	return size;
}

static unsigned short
__getTableSize(GdbHashTable *table)
{
	return HASHTABLE_HEADER_SIZE + __getDataSize(table);
}

static unsigned short
__hash(GdbHashTable *table, unsigned short key)
{
	return (key % table->bucketCount);
}

void *
htReadBlock(GdbBlock *block, const char *buffer, void *extra)
{
	GdbHashTable *table;
	int counter = 0;
	int i, j;

	MEM_CHECK(table = (GdbHashTable *)malloc(sizeof(GdbHashTable)));
	memset(table, 0, sizeof(GdbHashTable));

	table->block = block;

	table->bucketCount = gdbGet8(buffer,  &counter);
	table->itemCount   = gdbGet16(buffer, &counter);

	/* Allocate memory for the buckets. */
	MEM_CHECK(table->buckets = (GdbBucket **)malloc(table->bucketCount *
													sizeof(GdbBucket *)));
	memset(table->buckets, 0, table->bucketCount * sizeof(GdbBucket *));

	/* Allocate memory for the bucket item counts. */
	MEM_CHECK(table->counts = (unsigned char *)malloc(table->bucketCount));
	memset(table->counts, 0, table->bucketCount);

	/* Read in the buckets and their items. */
	for (i = 0; i < table->bucketCount; i++)
	{
		GdbBucket *lastBucket;

		table->counts[i] = gdbGet8(buffer, &counter);

		lastBucket = NULL;

		for (j = 0; j < table->counts[i]; j++)
		{
			GdbBucket *bucket;

			MEM_CHECK(bucket = (GdbBucket *)malloc(sizeof(GdbBucket)));	

			bucket->key  = gdbGet16(buffer, &counter);
			bucket->type = gdbGet8(buffer,  &counter);

			if (typeSizes[bucket->type] == -1)
				bucket->size = gdbGet16(buffer, &counter);
			else
				bucket->size = typeSizes[bucket->type];

			MEM_CHECK(bucket->data = malloc(bucket->size));
			memcpy(bucket->data, buffer + counter, bucket->size);

			counter += bucket->size;

			bucket->next = NULL;

			if (lastBucket == NULL)
				table->buckets[i] = bucket;
			else
				lastBucket->next = bucket;

			lastBucket = bucket;
		}
	}

	return table;
}

void
htWriteBlock(GdbBlock *block, char **buffer, unsigned long *size)
{
	GdbHashTable *table;
	int counter = 0;
	int i;

	table = (GdbHashTable *)block->detail;
	
	*size = __getTableSize(table);

	MEM_CHECK(*buffer = (char *)malloc(*size));
	
	/* Write the hashtable header */
	gdbPut8(*buffer,  &counter, table->bucketCount);
	gdbPut16(*buffer, &counter, table->itemCount);

	/* Write the buckets and their items. */
	for (i = 0; i < table->bucketCount; i++)
	{
		GdbBucket *bucket;
		
		gdbPut8(*buffer, &counter, table->counts[i]);

		for (bucket = table->buckets[i];
			 bucket != NULL;
			 bucket = bucket->next)
		{
			gdbPut16(*buffer, &counter, bucket->key);
			gdbPut8(*buffer,  &counter, bucket->type);

			if (typeSizes[bucket->type] == -1)
				gdbPut16(*buffer, &counter, bucket->size);

			memcpy(*buffer + counter, bucket->data, bucket->size);
			counter += bucket->size;
		}
	}
}

void *
htCreateBlock(GdbBlock *block, void *extra)
{
	GdbHashTable *table;

	MEM_CHECK(table = (GdbHashTable *)malloc(sizeof(GdbHashTable)));
	memset(table, 0, sizeof(GdbHashTable));

	table->block       = block;
	table->bucketCount = 11; /* Prime number. */
	table->itemCount   = 0;

	/* Allocate memory for the buckets. */
	MEM_CHECK(table->buckets = (GdbBucket **)malloc(table->bucketCount *
													sizeof(GdbBucket *)));
	memset(table->buckets, 0, table->bucketCount * sizeof(GdbBucket *));

	/* Allocate memory for the bucket item counts. */
	MEM_CHECK(table->counts = (unsigned char *)malloc(table->bucketCount));
	memset(table->counts, 0, table->bucketCount);

	return table;
}

void
htDestroyBlock(void *data)
{
	GdbHashTable *table;
	GdbBucket *bucket, *nextBucket;
	int i;
	
	table = (GdbHashTable *)data;

	for (i = 0; i < table->bucketCount; i++)
	{
		for (bucket = table->buckets[i];
			 bucket != NULL;
			 bucket = nextBucket)
		{
			nextBucket = bucket->next;

			free(bucket->data);
			free(bucket);
		}
	}

	free(table->buckets);
	free(table->counts);
	free(table);
}

GdbHashTable *
htOpen(GDatabase *db, offset_t offset)
{
	GdbBlock *block;

	if (db == NULL || offset < DB_HEADER_BLOCK_SIZE)
		return NULL;

	block = gdbReadBlock(db, offset, GDB_BLOCK_HASHTABLE, NULL);

	if (block == NULL)
		return NULL;

	return (GdbHashTable *)block->detail;
}

GdbHashTable *
htCreate(GDatabase *db)
{
	GdbBlock *block;

	if (db == NULL)
		return NULL;

	block = gdbNewBlock(db, GDB_BLOCK_HASHTABLE, NULL);

	if (block == NULL)
		return NULL;

	return (GdbHashTable *)block->detail;
}

void
htAdd(GdbHashTable *table, unsigned short key, const void *data,
	  unsigned char type, unsigned short size)
{
	unsigned short index;
	GdbBucket *bucket, *nextBucket = NULL;
	
	if (table == NULL || key == 0 || data == NULL ||
		size == 0 || type > GDB_HT_MAX_TYPE)
	{
		return;
	}

	if (typeSizes[type] != -1)
		size = typeSizes[type];

	index = __hash(table, key);

	GDB_SET_DIRTY(table->block);

	if (table->buckets[index] != NULL)
	{
		/* See if this already exists in this bucket. */
		for (bucket = table->buckets[index];
			 bucket != NULL;
			 bucket = bucket->next)
		{
			if (bucket->key == key)
			{
				/* It already exists. Update the data. */

				if (bucket->data != NULL)
					free(bucket->data);

				bucket->type = type;
				bucket->size = size;
				
				MEM_CHECK(bucket->data = malloc(size));
				memcpy(bucket->data, data, size);

				return;
			}
		}

		/* Wasn't found. Just add it to the beginning. */
		nextBucket = table->buckets[index];
	}

	/* Create the bucket for this. */
	MEM_CHECK(bucket = (GdbBucket *)malloc(sizeof(GdbBucket)));	
	bucket->key  = key;
	bucket->next = nextBucket;
	
	/* Add the data. */
	bucket->type = type;
	bucket->size = size;
	
	MEM_CHECK(bucket->data = malloc(size));
	memcpy(bucket->data, data, size);

	table->buckets[index] = bucket;
	table->counts[index]++;
	table->itemCount++;
}

char
htRemove(GdbHashTable *table, unsigned short key)
{
	unsigned short index;
	GdbBucket *bucket, *prevBucket = NULL;
	
	if (table == NULL || key == 0)
		return 0;

	index = __hash(table, key);

	for (bucket = table->buckets[index];
		 bucket != NULL;
		 bucket = bucket->next)
	{
		if (bucket->key == key)
		{
			prevBucket->next = bucket->next;
			table->itemCount--;

			table->counts[index]--;

			free(bucket->data);
			free(bucket);

			return 1;
		}
		
		prevBucket = bucket;
	}

	return 0;
}

const void *
htGetData(GdbHashTable *table, unsigned short key, unsigned short *size,
		  unsigned char *type)
{
	unsigned short index;
	GdbBucket *bucket;
	
	if (table == NULL || key == 0)
		return 0;

	index = __hash(table, key);

	for (bucket = table->buckets[index];
		 bucket != NULL;
		 bucket = bucket->next)
	{
		if (bucket->key == key)
		{
			if (size != NULL) *size = bucket->size;
			if (type != NULL) *type = bucket->type;

			return bucket->data;
		}
	}

	if (size != NULL) *size = 0;
	if (type != NULL) *type = 0;

	return 0;
}

void
htAddString(GdbHashTable *table, unsigned short key, const char *value)
{
	if (table == NULL || key == 0 || value == NULL || value == '\0')
		return;

	/* We don't really need the trailing NUL. */
	htAdd(table, key, value, GDB_HT_STRING, strlen(value));
}

void
htAddLong(GdbHashTable *table, unsigned short key, long value)
{
	unsigned long newValue;

	if (table == NULL || key == 0)
		return;

	newValue = htonl(value);

	htAdd(table, key, &newValue, GDB_HT_LONG, sizeof(long));
}

void
htAddOffset(GdbHashTable *table, unsigned short key, offset_t value)
{
	offset_t newValue;

	if (table == NULL || key == 0 || value == 0)
		return;

	newValue = htonl(value);

	htAdd(table, key, &newValue, GDB_HT_OFFSET, sizeof(offset_t));
}

char *
htGetString(GdbHashTable *table, unsigned short key)
{
	unsigned short size;
	unsigned char  type;
	const void    *data;
	char          *value;
	
	if (table == NULL || key == 0)
		return NULL;

	data = htGetData(table, key, &size, &type);

	if (size == 0 || type != GDB_HT_STRING)
		return NULL;

	MEM_CHECK(value = (char *)malloc(size + 1));

	strncpy(value, (const char *)data, size);
	value[size] = '\0';

	return value;
}

long
htGetLong(GdbHashTable *table, unsigned short key)
{
	unsigned short size;
	unsigned char  type;
	const void    *data;
	long           value;

	if (table == NULL || key == 0)
		return 0;

	data = htGetData(table, key, &size, &type);

	if (type != GDB_HT_LONG)
		return 0;

	memcpy(&value, data, sizeof(long));

	value = ntohl(value);

	return value;
}

offset_t
htGetOffset(GdbHashTable *table, unsigned short key)
{
	unsigned short size;
	unsigned char  type;
	const void    *data;
	offset_t       value;

	if (table == NULL || key == 0)
		return 0;

	data = htGetData(table, key, &size, &type);

	if (type != GDB_HT_OFFSET)
		return 0;

	memcpy(&value, data, sizeof(offset_t));

	value = ntohl(value);

	return value;
}

