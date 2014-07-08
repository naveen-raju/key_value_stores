/**
 * @file search_by_file.c Search by file support.
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
#include "gnupdate.h"

static PmPackage *
__firstListPackage(PmDatabase *db, DbMatchData *data)
{
	offset_t offset;
	GdbOffsetList *list;

	list = (GdbOffsetList *)data->data;

	offset = olGetOffset(list, data->index++);

	/* Extra precaution. */
	if (offset == 0)
		return NULL;

	return dbReadPackage(db, offset);
}

static PmPackage *
__nextListPackage(PmDatabase *db, DbMatchData *data)
{
	offset_t offset;
	GdbOffsetList *list;

	list = (GdbOffsetList *)data->data;

	offset = olGetOffset(list, data->index++);

	if (offset == 0)
		return NULL;

	return dbReadPackage(db, offset);
}

static void
__destroyListData(DbMatchData *data)
{
	GdbOffsetList *list;
	
	list = (GdbOffsetList *)data->data;
	
	olClose(list);
}

static PmPackage *
__firstNodePackage(PmDatabase *db, DbMatchData *data)
{
	offset_t offset;

	offset = (offset_t)data->data;

	return dbReadPackage(db, offset);
}

static PmPackage *
__nextNodePackage(PmDatabase *db, DbMatchData *data)
{
	return NULL;
}

static void
__destroyNodeData(DbMatchData *data)
{
}

PmStatus
dbFindByFile(PmDatabase *db, const char *file, PmMatches *matches)
{
	GdbOffsetList *list;
	DbMatchData   *data;
	DbData        *dbData;
	offset_t       offset;
	blocktype_t    type;

	dbData = (DbData *)db->db;

	/* Search for the file. */
	offset = btreeSearch(dbData->filesIndex->mainTree, file);

	if (offset == 0)
		return PM_FAILED; /* Not found. */

	/* The file tree was found. Open it. */
	type = gdbBlockTypeAt(dbData->filesIndex, offset);

	if (type == GDB_BLOCK_OFFSET_LIST)
	{
		list = olOpen(dbData->filesIndex, offset);

		if (list == NULL)
			return PM_FAILED; /* List not found. */

		if (olGetCount(list) == 0)
		{
			olClose(list);
			return PM_FAILED;
		}

		data = newMatchData(__firstListPackage, __nextListPackage,
							__destroyListData);

		data->data = list;
	}
	else
	{
		data = newMatchData(__firstNodePackage, __nextNodePackage,
							__destroyNodeData);

		data->data = (void *)offset;
	}

	data->index = 0;
	matches->matches = data;

	return PM_SUCCESS;
}

