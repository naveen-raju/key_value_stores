/**
 * @file db_cache.c Block caching functions
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

void
gdbCacheAddBlock(GDatabase *db, GdbBlock *block)
{
	GdbBlock *tempBlock;
	int i;

	if (block->offset == 0)
	{
		pmError(PM_ERROR_FATAL,
				_("Trying to add a block to the list with offset 0.\n"));
		abort();
	}

	if (block->inList == 1)
		return;

	/* See if it's already in the list. */
	tempBlock = gdbCacheGetBlock(db, block->offset);

	if (tempBlock != NULL)
		return;
	
	if (db->openBlockCount >= db->openBlockSize)
	{
		GdbBlock **newBlocks;
		int        newSize;

		newSize = 2 * db->openBlockSize;

		MEM_CHECK(newBlocks = (GdbBlock **)malloc(newSize *
												  sizeof(GdbBlock *)));
		memset(newBlocks, 0, newSize * sizeof(GdbBlock *));

		for (i = 0; i < db->openBlockSize; i++)
			newBlocks[i] = db->openBlocks[i];

		free(db->openBlocks);

		db->openBlocks    = newBlocks;
		db->openBlockSize = newSize;
	}
	
	/* Find a place to put this. */
	for (i = 0; i < db->openBlockSize; i++)
	{
		if (db->openBlocks[i] == NULL)
		{
			db->openBlocks[i] = block;
			db->openBlockCount++;

			block->refCount++;
			block->inList = 1;

			return;
		}
	}

	pmError(PM_ERROR_WARNING,
			_("GNUpdate DB: "
			  "Unable to place the open block in the block list!\n"));
}

unsigned short
gdbCacheRemoveBlock(GDatabase *db, GdbBlock *block)
{
	int i;

	if (block->offset == 0)
	{
		pmError(PM_ERROR_FATAL,
				_("GNUpdate DB: "
				  "Trying to remove block from list with offset 0\n"));
		abort();
	}
	
	if (db->openBlockCount == 0)
	{
		pmError(PM_ERROR_WARNING,
				_("db->openBlockCount == 0 in %s, line %d\n"),
				__FILE__, __LINE__);
		return 0;
	}

	for (i = 0; i < db->openBlockSize; i++)
	{
		if (db->openBlocks[i] != NULL &&
			db->openBlocks[i]->offset == block->offset)
		{
			db->openBlocks[i]->refCount--;

			if (db->openBlocks[i]->refCount <= 0)
			{
				db->openBlocks[i] = NULL;
				db->openBlockCount--;
				block->inList = 0;

				return 0;
			}
			
			return db->openBlocks[i]->refCount;
		}
	}

	pmError(PM_ERROR_WARNING,
			_("GNUpdate DB: No open block found at offset %ld!\n"),
			block->offset);

	return 0;
}

GdbBlock *
gdbCacheGetBlock(GDatabase *db, offset_t offset)
{
	int i;

	for (i = 0; i < db->openBlockSize; i++)
	{
		if (db->openBlocks[i] != NULL &&
			db->openBlocks[i]->offset == offset)
		{
			db->openBlocks[i]->refCount++;

			return db->openBlocks[i];
		}
	}
	
	return NULL;
}


