/**
 * @file db.c GNUpdate database functions
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

static void
__setupDatabase(GDatabase *db)
{
	db->openBlockCount = 0;
	db->openBlockSize  = 10;

	MEM_CHECK(db->openBlocks = (GdbBlock **)malloc(db->openBlockSize *
												   sizeof(GdbBlock *)));
	memset(db->openBlocks, 0, db->openBlockSize * sizeof(GdbBlock *));
}

GDatabase *
gdbOpen(const char *filename, GdbType type, PmAccessMode mode)
{
	GDatabase *db;
	FILE      *fp;

	cxReturnValueUnless(filename != NULL,      NULL);
	cxReturnValueUnless(mode != PM_MODE_ERROR, NULL);

	if (mode == PM_MODE_READ_WRITE)
		fp = fopen(filename, "r+");
	else
		fp = fopen(filename, "r");

	if (fp == NULL)
	{
		if (mode == PM_MODE_READ_WRITE)
			return gdbCreate(filename, type);
		else
		{
			pmError(PM_ERROR_WARNING,
					_("GNUpdate DB: "
					  "Unable to open database %s for reading.\n"),
					filename);

			return NULL;
		}
	}

	MEM_CHECK(db = (GDatabase *)malloc(sizeof(GDatabase)));
	memset(db, 0, sizeof(GDatabase));

	db->fp = fp;

	if ((gdbReadHeader(db) == 0) || (db->type != type))
	{
		fclose(fp);

		free(db);

		return NULL;
	}

	__setupDatabase(db);

	db->mode     = mode;
	db->filename = strdup(filename);
	db->mainTree = btreeOpen(db, DB_MAIN_TREE_OFFSET);

	return db;
}

void
gdbClose(GDatabase *db)
{
	cxReturnUnless(db != NULL);

	if (db->fp != NULL)
		fclose(db->fp);

	btreeClose(db->mainTree);

	gdbDestroy(db);
}

GDatabase *
gdbCreate(const char *filename, GdbType type)
{
	GDatabase *db;
	FILE      *fp;

	cxReturnValueUnless(filename != NULL, NULL);

	fp = fopen(filename, "w+");

	if (fp == NULL)
	{
		pmError(PM_ERROR_WARNING,
				_("GNUpdate DB: "
				  "Unable to open database %s for reading/writing.\n"),
				filename);

		return NULL;
	}

	MEM_CHECK(db = (GDatabase *)malloc(sizeof(GDatabase)));
	memset(db, 0, sizeof(GDatabase));

	__setupDatabase(db);

	db->filename = strdup(filename);
	db->type     = type;
	db->fp       = fp;

	gdbWriteHeader(db);

	/* Leave enough room for the free block list. */
	fseek(db->fp, DB_FREE_BLOCK_LIST_OFFSET, SEEK_SET);
	gdbPad(db->fp, DB_FREE_BLOCK_LIST_SIZE);

	db->mainTree = btreeCreate(db, 5);

	return db;
}

GDatabase *
gdbDestroy(GDatabase *db)
{
	cxReturnValueUnless(db != NULL, NULL);

	free(db->openBlocks);
	free(db->filename);
	free(db);

	return NULL;
}

GdbStatus
gdbAddDataEntry(GDatabase *db, GdbHashTable *table, unsigned short key,
				const void *data, long size)
{
	if (db == NULL || db->fp == NULL || key == 0 || table == NULL ||
		data == NULL || size == 0)
	{
		return GDB_ERROR;
	}

	htAdd(table, key, data, size, GDB_HT_RAW);

	return GDB_SUCCESS;
}

GdbStatus
gdbAddIndexEntry(GDatabase *db, BTree *tree, const char *key, offset_t offset)
{
	if (db == NULL || tree == NULL || db->fp == NULL || key == NULL ||
		offset == 0)
	{
		return GDB_ERROR;
	}
	
	return btreeInsert(tree, key, offset, 0);
}

GdbStatus
gdbAddTree(GDatabase *db, BTree *tree, const char *key, BTree **newTree)
{
	GdbStatus   status;
	offset_t    offset;
	short       blockSize;
	blocktype_t type;
	
	if (db == NULL || tree == NULL || db->fp == NULL || key == NULL ||
		newTree == NULL)
	{
		return GDB_ERROR;
	}

	*newTree = btreeCreate(db, 5);

	offset = (*newTree)->block->offset;
	blockSize = (*newTree)->block->multiple;
	type = (*newTree)->block->type;

	if (*newTree == NULL)
	{
		*newTree = NULL;

		return GDB_ERROR;
	}

	status = btreeInsert(tree, key, offset, 0);

	if (status == GDB_DUPLICATE)
	{
		/* Return the existing newTree. */
		btreeClose(*newTree);
		gdbFreeBlock(db, offset, type);

		offset = btreeSearch(tree, key);

		if (offset == 0)
		{
			/* I doubt this will ever happen. */
			pmError(PM_ERROR_FATAL,
					_("GNUpdate DB: Possible database corruption! Back up "
					  "your database and contact a developer.\n"));
			exit(1);
		}

		*newTree = btreeOpen(db, offset);

		status = GDB_SUCCESS;
	}
	else if (status == GDB_ERROR)
	{
		btreeClose(*newTree);

		fseek(db->fp, offset, SEEK_SET);

#if 0
		gdbPad(db->fp, blockSize);
#endif

		gdbFreeBlock(db, offset, type);

		*newTree = NULL;
	}

	return status;
}

GdbStatus
gdbAddHashTable(GDatabase *db, BTree *tree, const char *key,
				GdbHashTable **newTable)
{
	return GDB_ERROR;
}

