/**
 * @file get_all_packages.c Returns all packages in the database.
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

#include <dirent.h>

static PmPackage *
__firstPackage(PmDatabase *db, DbMatchData *data)
{
	BTreeTraversal *trav;
	offset_t offset;

	trav = (BTreeTraversal *)data->data;

	offset = btreeGetFirstOffset(trav);

	if (offset == -1)
		return NULL;

	return dbReadPackage(db, offset);
}

static PmPackage *
__nextPackage(PmDatabase *db, DbMatchData *data)
{
	BTreeTraversal *trav;
	offset_t offset;

	trav = (BTreeTraversal *)data->data;

	offset = btreeGetNextOffset(trav);

	if (offset == -1)
		return NULL;

	return dbReadPackage(db, offset);
}

static void
__destroyData(DbMatchData *data)
{
	btreeDestroyTraversal((BTreeTraversal *)data->data);
}

PmStatus
dbGetAllPackages(PmDatabase *db, PmMatches *matches)
{
	DbMatchData *data;

	data = newMatchData(__firstPackage, __nextPackage, __destroyData);
	matches->matches = data;

	data->data = btreeInitTraversal(((DbData *)db->db)->namesIndex->mainTree);

	return PM_SUCCESS;
}
