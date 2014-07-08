/**
 * @file offsetlist.c A list of offsets.
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

void *
olReadBlock(GdbBlock *block, const char *buffer, void *extra)
{
	GdbOffsetList *list;
	int counter = 0;
	int i;

	MEM_CHECK(list = (GdbOffsetList *)malloc(sizeof(GdbOffsetList)));
	memset(list, 0, sizeof(GdbOffsetList));

	list->block = block;

	list->count = gdbGet16(buffer, &counter);
	
	list->arraySize = (list->count < 10 ? 10 : list->count);

	MEM_CHECK(list->offsets = (offset_t *)malloc(list->arraySize *
												 sizeof(offset_t)));

	for (i = 0; i < list->count; i++)
		list->offsets[i] = gdbGet32(buffer, &counter);

	return list;
}

void
olWriteBlock(GdbBlock *block, char **buffer, unsigned long *size)
{
	GdbOffsetList *list = (GdbOffsetList *)block->detail;
	int i, counter = 0;

	*size = sizeof(short) + list->count * sizeof(offset_t);

	MEM_CHECK(*buffer = (char *)malloc(*size));
		
	gdbPut16(*buffer, &counter, list->count);
	
	for (i = 0; i < list->count; i++)
		gdbPut32(*buffer, &counter, list->offsets[i]);
}

void *
olCreateBlock(GdbBlock *block, void *extra)
{
	GdbOffsetList *list;

	MEM_CHECK(list = (GdbOffsetList *)malloc(sizeof(GdbOffsetList)));
	memset(list, 0, sizeof(GdbOffsetList));

	list->block     = block;
	list->arraySize = 10;
	list->count     = 0;

	MEM_CHECK(list->offsets = (offset_t *)malloc(list->arraySize *
												 sizeof(offset_t)));

	return list;
}

void
olDestroyBlock(void *data)
{
	GdbOffsetList *list = (GdbOffsetList *)data;

	free(list->offsets);
	free(list);
}

GdbOffsetList *
olOpen(GDatabase *db, offset_t offset)
{
	GdbBlock *block;

	if (db == NULL || db->fp == NULL || offset < DB_HEADER_BLOCK_SIZE)
		return NULL;

	block = gdbReadBlock(db, offset, GDB_BLOCK_OFFSET_LIST, NULL);

	if (block == NULL)
		return NULL;

	return (GdbOffsetList *)block->detail;
}

void
olClose(GdbOffsetList *list)
{
	if (list == NULL)
		return;

	gdbDestroyBlock(list->block);
}

GdbOffsetList *
olCreate(GDatabase *db)
{
	GdbBlock *block;

	if (db == NULL || db->fp == NULL)
		return NULL;

	block = gdbNewBlock(db, GDB_BLOCK_OFFSET_LIST, NULL);
	
	if (block == NULL)
		return NULL;

	return (GdbOffsetList *)block->detail;
}

void
olAddOffset(GdbOffsetList *list, offset_t offset)
{
	if (list == NULL || offset == 0)
		return;

	if (list->count == list->arraySize)
	{
		/* Expand the array. */
		offset_t *newArray;
		unsigned long newSize;
		
		newSize = list->arraySize * 2;
		
		MEM_CHECK(newArray = (offset_t *)malloc(newSize * sizeof(offset_t)));
		memset(newArray, 0, newSize * sizeof(offset_t));

		/* Copy the array over. */
		memcpy(newArray, list->offsets, list->count * sizeof(offset_t));

		/* Free the old array. */
		free(list->offsets);

		/* Move the new one over. */
		list->offsets = newArray;
		list->arraySize = newSize;
	}

	list->offsets[list->count] = offset;

	list->count++;
}

offset_t
olGetOffset(GdbOffsetList *list, unsigned short index)
{
	if (list == NULL || index >= list->count)
		return 0;

	return list->offsets[index];
}

unsigned short
olGetCount(GdbOffsetList *list)
{
	if (list == NULL)
		return 0;

	return list->count;
}
