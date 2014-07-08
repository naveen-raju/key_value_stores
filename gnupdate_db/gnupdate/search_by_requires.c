/**
 * @file search_by_requires.c Search by requirements function.
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
__firstPackage(PmDatabase *db, DbMatchData *data)
{
	BTreeTraversal *trav = (BTreeTraversal *)data->data;

	return dbReadPackage(db, btreeGetFirstOffset(trav));
}

static PmPackage *
__nextPackage(PmDatabase *db, DbMatchData *data)
{
	offset_t offset;
	BTreeTraversal *trav = (BTreeTraversal *)data->data;

	offset = btreeGetNextOffset(trav);

	if (offset == -1)
		return NULL;

	return dbReadPackage(db, offset);
}

static void
__destroyData(DbMatchData *data)
{
	BTreeTraversal *trav;
	BTree *tree;
	
	trav = (BTreeTraversal *)data->data;
	tree = trav->tree;
	
	btreeDestroyTraversal(trav);
	btreeClose(tree);
}

PmStatus
dbFindByRequires(PmDatabase *db, const char *requirement, PmMatches *matches)
{
	DbMatchData *data;
	DbData *dbData;
	BTree *tree;
	offset_t offset;

	dbData = (DbData *)db->db;

	/* Search for the group. */
	offset = btreeSearch(dbData->reqDepsIndex->mainTree, requirement);

	if (offset == 0)
		return PM_FAILED; /* Not found. */

	/* The group tree was found. Open it. */
	tree = btreeOpen(dbData->reqDepsIndex, offset);

	if (tree == NULL || btreeGetSize(tree) == 0)
		return PM_FAILED; /* Tree not found. */

	if (btreeGetSize(tree) == 0)
	{
		btreeClose(tree);
		return PM_FAILED;
	}
	
	data = newMatchData(__firstPackage, __nextPackage, __destroyData);
	matches->matches = data;

	data->data = btreeInitTraversal(tree);

	return PM_SUCCESS;
}

